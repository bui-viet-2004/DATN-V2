#pragma once

#include "main.h"
#include "stm32f1xx_hal.h"
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

/* ─── Cấu hình (sửa tại đây nếu cần) ──────────── */
#define ZB_HUART            huart1          /* UART nối với E18        */
#define ZB_UART_IRQn        USART1_IRQn
#define ZB_MAX_NODES        20              /* số node tối đa          */
#define ZB_RX_BUF_SIZE      256             /* ring buffer (lũy thừa 2)*/
#define ZB_PAYLOAD_MAX      64              /* payload tối đa 1 frame  */
#define ZB_FRAME_PARSE_TO   200             /* ms: timeout parse frame */
#define ZB_TIMEOUT_OFFLINE  20000           /* ms: không gửi → offline */
#define ZB_TIMEOUT_EXPIRE   30000          /* ms: không gửi → xóa    */

/* ─── Dữ liệu 1 node ───────────────────────────── */
typedef struct {
    uint16_t addr;          /* Short address Zigbee                  */
    bool     active;        /* true = slot đang dùng                 */
    bool     online;        /* true = last_seen < ZB_TIMEOUT_OFFLINE */
    uint32_t last_seen_ms;
    uint32_t joined_ms;     // Thời điểm Node gia nhập mạng
 
    float    temperature;   /* °C  — NAN nếu chưa nhận              */
    float    voltage_ep2;   /* V   — NAN nếu chưa nhận              */
    float    voltage_ep3;   /* V   — NAN nếu chưa nhận              */
    float    voltage_ep4;   /* V   — NAN nếu chưa nhận              */
   
    uint8_t  lqi;

    struct {
        uint8_t temperature : 1;
        uint8_t voltage_ep2 : 1;
        uint8_t voltage_ep3 : 1;
        uint8_t voltage_ep4 : 1;
     
    } valid;
} ZbNode_t;

/* ─── API ───────────────────────────────────────── */

/** Init E18 (reset, đăng ký EP, start network) + bật UART interrupt */
void    ZB_Begin(void);

/** Đặt vào USART1_IRQHandler() trong stm32f1xx_it.c */
void    ZB_IRQHandler(void);

/** Parse ring buffer — gọi liên tục trong while(1) */
void    ZB_Poll(void);

/** Dọn node hết hạn — gọi mỗi ~5 giây */
void    ZB_Tick(void);

/**
 * @brief Lấy node có data mới nhất
 * @return true = có data mới, out đã được ghi
 *         false = chưa có gì mới
 */
bool    ZB_Read(ZbNode_t *out);

/** Số node đang active */
uint8_t ZB_Count(void);