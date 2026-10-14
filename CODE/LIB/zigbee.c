#include "zigbee.h"
#include "TFT_ILI9341.h"
#include <string.h>

extern UART_HandleTypeDef ZB_HUART;

#define RB_MASK  (ZB_RX_BUF_SIZE - 1)

static uint8_t           _rb[ZB_RX_BUF_SIZE];
static volatile uint16_t _rb_head = 0;
static uint16_t          _rb_tail = 0;

#define RB_PUSH(b)  do { _rb[_rb_head & RB_MASK] = (b); _rb_head++; } while(0)
#define RB_AVAIL()  ((uint16_t)(_rb_head - _rb_tail))
#define RB_POP()    (_rb[_rb_tail++ & RB_MASK])

/* =========================================================
   PARSER
   ========================================================= */

typedef enum
{
    PS_IDLE,
    PS_LEN,
    PS_C0,
    PS_C1,
    PS_DATA,
    PS_FCS

} PS_t;

static PS_t     _ps   = PS_IDLE;
static uint8_t  _zlen = 0;
static uint8_t  _zc0  = 0;
static uint8_t  _zc1  = 0;

static uint8_t  _zpay[ZB_PAYLOAD_MAX];

static uint8_t  _zidx = 0;
static uint32_t _zts  = 0;

static void _reset(void)
{
    _ps = PS_IDLE;
    _zidx = 0;
}

/* =========================================================
   DATABASE
   ========================================================= */

static ZbNode_t _nodes[ZB_MAX_NODES];

static uint8_t _count = 0;

static ZbNode_t      _latest;
static volatile bool _fresh = false;

static void _node_init(ZbNode_t *n)
{
    memset(n, 0, sizeof(ZbNode_t));

    n->temperature = 0.0f;

    n->voltage_ep2 = 0.0f;
    n->voltage_ep3 = 0.0f;
    n->voltage_ep4 = 0.0f;
}

static ZbNode_t* _find(uint16_t addr)
{
    for (int i = 0; i < ZB_MAX_NODES; i++)
    {
        if (_nodes[i].active &&
            _nodes[i].addr == addr)
        {
            return &_nodes[i];
        }
    }

    return NULL;
}

static ZbNode_t* _get_or_create(uint16_t addr)
{
    ZbNode_t *n = _find(addr);

    if (n) return n;

    for (int i = 0; i < ZB_MAX_NODES; i++)
    {
        if (!_nodes[i].active)
        {
            n = &_nodes[i];
            goto init;
        }
    }

    {
        uint32_t oldest = UINT32_MAX;

        for (int i = 0; i < ZB_MAX_NODES; i++)
        {
            if (_nodes[i].last_seen_ms < oldest)
            {
                oldest = _nodes[i].last_seen_ms;
                n = &_nodes[i];
            }
        }

        _count--;
    }

init:

    _node_init(n);

    n->active    = true;
    n->addr      = addr;
    n->joined_ms = HAL_GetTick();

    _count++;

    return n;
}

/* =========================================================
   PROCESS FRAME
   ========================================================= */

static void _process(void)
{
    if (_zc0 != 0x44 ||
        _zc1 != 0x81 ||
        _zlen < 10)
    {
        return;
    }

    uint16_t clus =
        (uint16_t)((_zpay[3] << 8) | _zpay[2]);

    uint16_t addr =
        (uint16_t)((_zpay[5] << 8) | _zpay[4]);

    uint8_t ep  = _zpay[6];
    uint8_t lqi = _zpay[9];

    int z = -1;

    for (int i = 0; i <= (int)_zlen - 5; i++)
    {
        if (_zpay[i] == 0x18 &&
            _zpay[i + 2] == 0x0A)
        {
            z = i;
            break;
        }
    }

    if (z < 0 || (z + 7) > _zlen)
    {
        return;
    }

    uint16_t raw =
        (uint16_t)((_zpay[z + 7] << 8) |
                    _zpay[z + 6]);

    __disable_irq();

    ZbNode_t *n = _get_or_create(addr);

    if (!n)
    {
        __enable_irq();
        return;
    }

    n->lqi          = lqi;
    n->last_seen_ms = HAL_GetTick();
    n->online       = true;

    switch (clus)
    {
        /* =========================
           TEMPERATURE
           ========================= */

        case 0x0402:

            n->temperature =
                (float)((int16_t)raw) / 100.0f;

            n->valid.temperature = 1;

            break;

        /* =========================
           ANALOG DATA
           ========================= */

        case 0x040B:
        case 0x0B04:
        case 0x000C:

            /* EP2 = CURRENT */

            if (ep == 0x02)
            {
                float adc_voltage;

                adc_voltage = raw / 100.0f;

                /* OFFSET 1.65V */

                if (adc_voltage < 1.65f)
                {
                    adc_voltage = 1.65f;
                }

                n->voltage_ep2 =
                    (adc_voltage - 1.65f) * 50.0f;

                n->valid.voltage_ep2 = 1;
            }

            /* EP3 = VOLTAGE */

            else if (ep == 0x03)
            {
                n->voltage_ep3 =
                    (raw / 100.0f) * 2.0f;

                n->valid.voltage_ep3 = 1;
            }

            /* EP4 = VIBRATION */

            else if (ep == 0x04)
            {
                n->voltage_ep4 =
                    raw / 100.0f;

                n->valid.voltage_ep4 = 1;
            }

            break;

        default:
            break;
    }

    /* =====================================================
       CH? UPDATE FIELD V?A NH?N
       KHÔNG XÓA DATA EP KHÁC
       ===================================================== */

   if (n->valid.temperature == 1 && n->valid.voltage_ep2 == 1) 
    {
        _latest = *n;
        _fresh = true;
        
        // G?i di xong thì reset c? valid v? 0 d? ch? chu k? 5 giây ti?p theo tích luy l?i t? d?u
        n->valid.temperature = 0;
        n->valid.voltage_ep2 = 0;
    }

    __enable_irq();
}

/* =========================================================
   ZNP SEND
   ========================================================= */

static void _znp_send(uint8_t cmd0,
                      uint8_t cmd1,
                      uint8_t len,
                      const uint8_t *data)
{
    uint8_t fcs = len ^ cmd0 ^ cmd1;

    for (int i = 0; i < len; i++)
    {
        fcs ^= data[i];
    }

    uint8_t hdr[4] =
    {
        0xFE,
        len,
        cmd0,
        cmd1
    };

    HAL_UART_Transmit(&ZB_HUART,
                      hdr,
                      4,
                      100);

    if (len)
    {
        HAL_UART_Transmit(&ZB_HUART,
                          (uint8_t*)data,
                          len,
                          100);
    }

    HAL_UART_Transmit(&ZB_HUART,
                      &fcs,
                      1,
                      100);
}

/* =========================================================
   WAIT RESPONSE
   ========================================================= */

static bool _znp_wait(uint8_t cmd0,
                      uint8_t cmd1,
                      uint32_t timeout_ms)
{
    uint8_t  buf[72] = {0};

    uint8_t  idx = 0;

    uint32_t t0 = HAL_GetTick();

    while ((HAL_GetTick() - t0) < timeout_ms)
    {
        if (ZB_HUART.Instance->SR & USART_SR_RXNE)
        {
            buf[idx++] = ZB_HUART.Instance->DR;

            if (idx >= 4 &&
                buf[idx - 4] == 0xFE &&
                buf[idx - 2] == cmd0 &&
                buf[idx - 1] == cmd1)
            {
                return true;
            }

            if (idx >= sizeof(buf))
            {
                idx = 0;
            }
        }

        if (ZB_HUART.Instance->SR &
           (USART_SR_ORE |
            USART_SR_NE  |
            USART_SR_FE))
        {
            (void)ZB_HUART.Instance->DR;
        }
    }

    return false;
}

/* =========================================================
   TFT
   ========================================================= */

static void _draw(uint16_t y,
                  const char *msg,
                  bool ok)
{
    TFT_DrawString(10,
                   y,
                   (char*)msg,
                   0xFFE0,
                   0x0000,
                   1);

    TFT_DrawString(220,
                   y,
                   ok ? "[OK]" : "[LOI]",
                   ok ? 0x07E0 : 0xF800,
                   0x0000,
                   1);
}

/* =========================================================
   INIT E18
   ========================================================= */

static void _e18_init(void)
{
    TFT_FillScreen(0x0000);

    uint8_t rst[] = {0x01};

    _znp_send(0x41,
              0x00,
              1,
              rst);

    _draw(40,
          "1. Reset E18...",
          _znp_wait(0x41, 0x80, 3000));

    uint8_t ep1[] =
    {
        0x01,0x04,0x01,0x00,
        0x00,0x00,0x00,0x00,0x00
    };

    uint8_t ep2[] =
    {
        0x02,0x04,0x01,0x00,
        0x00,0x00,0x00,0x00,0x00
    };

    uint8_t ep3[] =
    {
        0x03,0x04,0x01,0x00,
        0x00,0x00,0x00,0x00,0x00
    };

    uint8_t ep4[] =
    {
        0x04,0x04,0x01,0x00,
        0x00,0x00,0x00,0x00,0x00
    };

    _znp_send(0x24, 0x00, 9, ep1);

    bool epOk =
        _znp_wait(0x64, 0x00, 1000);

    _znp_send(0x24, 0x00, 9, ep2);
    _znp_wait(0x64, 0x00, 1000);

    _znp_send(0x24, 0x00, 9, ep3);
    _znp_wait(0x64, 0x00, 1000);

    _znp_send(0x24, 0x00, 9, ep4);
    _znp_wait(0x64, 0x00, 1000);

    _draw(60,
          "2. Dang ky EP 1,2,3,4...",
          epOk);

    uint8_t start[] =
    {
        0x00,
        0x00
    };

    _znp_send(0x25,
              0x40,
              2,
              start);

    bool netOk =
        _znp_wait(0x45, 0xC0, 5000);

    _draw(80,
          "3. Start Network...",
          netOk);

    if (netOk)
    {
        TFT_DrawString(10,
                       110,
                       "COORDINATOR ONLINE!",
                       0x07E0,
                       0x0000,
                       2);
    }
    else
    {
        TFT_DrawString(10,
                       110,
                       "THAT BAI (CHECK TX)",
                       0xF800,
                       0x0000,
                       2);
    }

    HAL_Delay(2000);

    TFT_FillScreen(0x0000);
}

/* =========================================================
   PUBLIC API
   ========================================================= */

void ZB_Begin(void)
{
    for (int i = 0; i < ZB_MAX_NODES; i++)
    {
        _node_init(&_nodes[i]);
    }

    _count = 0;

    _fresh = false;

    _e18_init();

    __HAL_UART_CLEAR_OREFLAG(&ZB_HUART);

    (void)ZB_HUART.Instance->DR;

    __HAL_UART_ENABLE_IT(&ZB_HUART,
                         UART_IT_RXNE);

    __HAL_UART_ENABLE_IT(&ZB_HUART,
                         UART_IT_ERR);

    HAL_NVIC_SetPriority(ZB_UART_IRQn,
                         1,
                         0);

    HAL_NVIC_EnableIRQ(ZB_UART_IRQn);
}

void ZB_IRQHandler(void)
{
    uint32_t sr = ZB_HUART.Instance->SR;

    if (sr & USART_SR_RXNE)
    {
        uint8_t b =
            (uint8_t)ZB_HUART.Instance->DR;

        if (RB_AVAIL() < ZB_RX_BUF_SIZE)
        {
            RB_PUSH(b);
        }
    }

    if (sr &
       (USART_SR_ORE |
        USART_SR_NE  |
        USART_SR_FE))
    {
        (void)ZB_HUART.Instance->DR;
    }
}

void ZB_Poll(void)
{
    if (_ps != PS_IDLE &&
       (HAL_GetTick() - _zts) >
        ZB_FRAME_PARSE_TO)
    {
        _reset();
    }

    while (RB_AVAIL())
    {
        uint8_t b = RB_POP();

        switch (_ps)
        {
            case PS_IDLE:

                if (b == 0xFE)
                {
                    _zts = HAL_GetTick();
                    _ps = PS_LEN;
                }

                break;

            case PS_LEN:

                if (b > ZB_PAYLOAD_MAX)
                {
                    _reset();
                    break;
                }

                _zlen = b;

                _ps = PS_C0;

                break;

            case PS_C0:

                _zc0 = b;

                _ps = PS_C1;

                break;

            case PS_C1:

                _zc1 = b;

                _zidx = 0;

                _ps = (_zlen > 0)
                    ? PS_DATA
                    : PS_FCS;

                break;

            case PS_DATA:

                _zpay[_zidx++] = b;

                if (_zidx >= _zlen)
                {
                    _ps = PS_FCS;
                }

                break;

            case PS_FCS:

            {
                uint8_t calc_fcs =
                    _zlen ^ _zc0 ^ _zc1;

                for (int i = 0; i < _zlen; i++)
                {
                    calc_fcs ^= _zpay[i];
                }

                if (calc_fcs == b)
                {
                    _process();
                }
            }

            _reset();

            break;
        }
    }
}

void ZB_Tick(void)
{
    uint32_t now = HAL_GetTick();

    for (int i = 0; i < ZB_MAX_NODES; i++)
    {
        ZbNode_t *n = &_nodes[i];

        if (!n->active)
        {
            continue;
        }

        uint32_t dt =
            now - n->last_seen_ms;

        n->online =
            (dt < ZB_TIMEOUT_OFFLINE);

        if (dt > ZB_TIMEOUT_EXPIRE)
        {
            _node_init(n);

            _count--;
        }
    }
}

bool ZB_Read(ZbNode_t *out)
{
    if (!_fresh)
    {
        return false;
    }

    __disable_irq();

    *out = _latest;

    _fresh = false;

    __enable_irq();

    return true;
}

uint8_t ZB_Count(void)
{
    return _count;
}