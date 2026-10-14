#include <stdlib.h>
#include "TFT_ILI9341.h"

//---Bảng Font 5x7---   
const uint8_t Font5x7[][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, // Space (32)
    {0x00, 0x00, 0x5F, 0x00, 0x00}, // !
    {0x00, 0x07, 0x00, 0x07, 0x00}, // "
    {0x14, 0x7F, 0x14, 0x7F, 0x14}, // #
    {0x24, 0x2A, 0x7F, 0x2A, 0x12}, // $
    {0x23, 0x13, 0x08, 0x64, 0x62}, // %
    {0x36, 0x49, 0x55, 0x22, 0x50}, // &
    {0x00, 0x06, 0x09, 0x06, 0x00}, // '
    {0x00, 0x1C, 0x22, 0x41, 0x00}, // (
    {0x00, 0x41, 0x22, 0x1C, 0x00}, // )
    {0x14, 0x08, 0x3E, 0x08, 0x14}, // *
    {0x08, 0x08, 0x3E, 0x08, 0x08}, // +
    {0x00, 0x50, 0x30, 0x00, 0x00}, // ,
    {0x08, 0x08, 0x08, 0x08, 0x08}, // -
    {0x00, 0x60, 0x60, 0x00, 0x00}, // .
    {0x20, 0x10, 0x08, 0x04, 0x02}, // /
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, // 0 (48)
    {0x00, 0x42, 0x7F, 0x40, 0x00}, // 1
    {0x42, 0x61, 0x51, 0x49, 0x46}, // 2
    {0x21, 0x41, 0x45, 0x4B, 0x31}, // 3
    {0x18, 0x14, 0x12, 0x7F, 0x10}, // 4
    {0x27, 0x45, 0x45, 0x45, 0x39}, // 5
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, // 6
    {0x01, 0x71, 0x09, 0x05, 0x03}, // 7
    {0x36, 0x49, 0x49, 0x49, 0x36}, // 8
    {0x06, 0x49, 0x49, 0x29, 0x1E}, // 9
    {0x00, 0x36, 0x36, 0x00, 0x00}, // : (58)
    {0x00, 0x56, 0x36, 0x00, 0x00}, // ;
    {0x08, 0x14, 0x22, 0x41, 0x00}, // <
    {0x14, 0x14, 0x14, 0x14, 0x14}, // =
    {0x00, 0x41, 0x22, 0x14, 0x08}, // >
    {0x02, 0x01, 0x51, 0x09, 0x06}, // ?
    {0x32, 0x49, 0x79, 0x41, 0x3E}, // @
    {0x7E, 0x11, 0x11, 0x11, 0x7E}, // A (65)
    {0x7F, 0x49, 0x49, 0x49, 0x36}, // B
    {0x3E, 0x41, 0x41, 0x41, 0x22}, // C
    {0x7F, 0x41, 0x41, 0x22, 0x1C}, // D
    {0x7F, 0x49, 0x49, 0x49, 0x41}, // E
    {0x7F, 0x09, 0x09, 0x09, 0x01}, // F
    {0x3E, 0x41, 0x49, 0x49, 0x7A}, // G
    {0x7F, 0x08, 0x08, 0x08, 0x7F}, // H
    {0x00, 0x41, 0x7F, 0x41, 0x00}, // I
    {0x20, 0x40, 0x41, 0x3F, 0x01}, // J
    {0x7F, 0x08, 0x14, 0x22, 0x41}, // K
    {0x7F, 0x40, 0x40, 0x40, 0x40}, // L
    {0x7F, 0x02, 0x04, 0x02, 0x7F}, // M
    {0x7F, 0x04, 0x08, 0x10, 0x7F}, // N
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, // O
    {0x7F, 0x09, 0x09, 0x09, 0x06}, // P
    {0x3E, 0x41, 0x51, 0x21, 0x5E}, // Q
    {0x7F, 0x09, 0x19, 0x29, 0x46}, // R
    {0x46, 0x49, 0x49, 0x49, 0x31}, // S
    {0x01, 0x01, 0x7F, 0x01, 0x01}, // T
    {0x3F, 0x40, 0x40, 0x40, 0x3F}, // U
    {0x1F, 0x20, 0x40, 0x20, 0x1F}, // V
    {0x3F, 0x40, 0x38, 0x40, 0x3F}, // W
    {0x63, 0x14, 0x08, 0x14, 0x63}, // X
    {0x07, 0x08, 0x70, 0x08, 0x07}, // Y
    {0x61, 0x51, 0x49, 0x45, 0x43}, // Z
    {0x00, 0x7F, 0x41, 0x41, 0x00}, // [
    {0x02, 0x04, 0x08, 0x10, 0x20}, // Backslash
    {0x00, 0x41, 0x41, 0x7F, 0x00}, // ]
    {0x08, 0x04, 0x02, 0x04, 0x08}, // ^
    {0x40, 0x40, 0x40, 0x40, 0x40}, // _
    {0x00, 0x01, 0x02, 0x04, 0x00}, // `
    {0x20, 0x54, 0x54, 0x54, 0x78}, // a (97)
    {0x7F, 0x48, 0x44, 0x44, 0x38}, // b
    {0x38, 0x44, 0x44, 0x44, 0x20}, // c
    {0x38, 0x44, 0x44, 0x44, 0x7F}, // d
    {0x38, 0x54, 0x54, 0x54, 0x18}, // e
    {0x10, 0x7E, 0x11, 0x11, 0x02}, // f
    {0x18, 0x54, 0x54, 0x54, 0x3C}, // g
    {0x7F, 0x08, 0x04, 0x04, 0x78}, // h
    {0x00, 0x44, 0x7D, 0x40, 0x00}, // i
    {0x20, 0x40, 0x44, 0x3D, 0x00}, // j
    {0x7F, 0x08, 0x14, 0x22, 0x41}, // k
    {0x00, 0x00, 0x7F, 0x00, 0x00}, // l
    {0x7C, 0x08, 0x54, 0x54, 0x78}, // m
    {0x7C, 0x08, 0x04, 0x04, 0x78}, // n
    {0x38, 0x44, 0x44, 0x44, 0x38}, // o
    {0x7C, 0x14, 0x14, 0x14, 0x08}, // p
    {0x38, 0x44, 0x44, 0x88, 0x7C}, // q
    {0x7C, 0x08, 0x04, 0x04, 0x08}, // r
    {0x48, 0x54, 0x54, 0x54, 0x20}, // s
    {0x04, 0x7F, 0x44, 0x44, 0x20}, // t
    {0x3C, 0x40, 0x40, 0x20, 0x7C}, // u
    {0x1C, 0x20, 0x40, 0x20, 0x1C}, // v
    {0x3C, 0xC8, 0xA8, 0xC8, 0x3C}, // w
    {0x44, 0x28, 0x10, 0x28, 0x44}, // x
    {0x78, 0x84, 0x84, 0x84, 0x78}, // y
    {0x64, 0x54, 0x54, 0x54, 0x4C}  // z
};

//---Hàm gửi Command---
void TFT_SendCommand(uint8_t cmd) 
{
    HAL_GPIO_WritePin(TFT_DC_PORT, TFT_DC_PIN, GPIO_PIN_RESET); // DC = 0 for command
    HAL_GPIO_WritePin(TFT_CS_PORT, TFT_CS_PIN, GPIO_PIN_RESET); // CS = 0 to select
    HAL_SPI_Transmit(&TFT_SPI_HANDLE, &cmd, 1, 10);
    HAL_GPIO_WritePin(TFT_CS_PORT, TFT_CS_PIN, GPIO_PIN_SET); // CS = 1 to deselect
}

//---Hàm gửi Data---
void TFT_SendData(uint8_t data) 
{
    HAL_GPIO_WritePin(TFT_DC_PORT, TFT_DC_PIN, GPIO_PIN_SET); // DC = 1 for data
    HAL_GPIO_WritePin(TFT_CS_PORT, TFT_CS_PIN, GPIO_PIN_RESET); // CS = 0 to select
    HAL_SPI_Transmit(&TFT_SPI_HANDLE, &data, 1, 10);
    HAL_GPIO_WritePin(TFT_CS_PORT, TFT_CS_PIN, GPIO_PIN_SET); // CS = 1 to deselect
}

//---Hàm reset TFT---
void TFT_Reset(void) 
{
    HAL_GPIO_WritePin(TFT_RST_PORT, TFT_RST_PIN, GPIO_PIN_RESET); // RST = 0
    HAL_Delay(100);
    HAL_GPIO_WritePin(TFT_RST_PORT, TFT_RST_PIN, GPIO_PIN_SET); // RST = 1
    HAL_Delay(100);
}

//---Hàm cấu hình TFT---
void TFT_Init(void) 
{
    TFT_Reset();
    TFT_SendCommand(0x01); // Software reset    
    HAL_Delay(120);

    TFT_SendCommand(0x28); // Display OFF

    TFT_SendCommand(0x3A); // Colour mode
    TFT_SendData(0x55); // 16-bit color (RGB565)

    TFT_SendCommand(0x36);
    TFT_SendData(0xE8); // Màn hình ngang

    TFT_SendCommand(0x11); // Sleep out
    HAL_Delay(120);
    TFT_SendCommand(0x29); // Display ON    
}

//---Hàm thiết lập vùng vẽ---
void TFT_SetAddress(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    uint8_t data[4];

    HAL_GPIO_WritePin(TFT_CS_PORT, TFT_CS_PIN, GPIO_PIN_RESET);

    // Column
    HAL_GPIO_WritePin(TFT_DC_PORT, TFT_DC_PIN, GPIO_PIN_RESET);
    uint8_t cmd = 0x2A;
    HAL_SPI_Transmit(&TFT_SPI_HANDLE, &cmd, 1, 50);

    HAL_GPIO_WritePin(TFT_DC_PORT, TFT_DC_PIN, GPIO_PIN_SET);

    data[0] = x >> 8;
    data[1] = x & 0xFF;
    data[2] = (x + w - 1) >> 8;
    data[3] = (x + w - 1) & 0xFF;

    HAL_SPI_Transmit(&TFT_SPI_HANDLE, data, 4, 50);

    // Row
    HAL_GPIO_WritePin(TFT_DC_PORT, TFT_DC_PIN, GPIO_PIN_RESET);
    cmd = 0x2B;
    HAL_SPI_Transmit(&TFT_SPI_HANDLE, &cmd, 1, 50);

    HAL_GPIO_WritePin(TFT_DC_PORT, TFT_DC_PIN, GPIO_PIN_SET);

    data[0] = y >> 8;
    data[1] = y & 0xFF;
    data[2] = (y + h - 1) >> 8;
    data[3] = (y + h - 1) & 0xFF;

    HAL_SPI_Transmit(&TFT_SPI_HANDLE, data, 4, 50);

    // RAM write
    HAL_GPIO_WritePin(TFT_DC_PORT, TFT_DC_PIN, GPIO_PIN_RESET);
    cmd = 0x2C;
    HAL_SPI_Transmit(&TFT_SPI_HANDLE, &cmd, 1, 50);

    HAL_GPIO_WritePin(TFT_DC_PORT, TFT_DC_PIN, GPIO_PIN_SET);

    HAL_GPIO_WritePin(TFT_CS_PORT, TFT_CS_PIN, GPIO_PIN_SET);
}

//---Hàm gửi nhiều byte dữ liệu liên tục---
void TFT_WriteBuffer(uint8_t *buff, uint16_t size)
{
    HAL_GPIO_WritePin(TFT_DC_PORT, TFT_DC_PIN, GPIO_PIN_SET);   // DC = 1 for data
    HAL_GPIO_WritePin(TFT_CS_PORT, TFT_CS_PIN, GPIO_PIN_RESET); // CS = 0 to select
    HAL_SPI_Transmit(&TFT_SPI_HANDLE, buff, size, 50);        // Gửi buffer dữ liệu
    HAL_GPIO_WritePin(TFT_CS_PORT, TFT_CS_PIN, GPIO_PIN_SET);   // CS = 1 to deselect
}

//---Hàm tô màu hình chữ nhật---
#define TFT_BUF_SIZE 256   // 256 byte = 128 pixel (tối ưu cho F103)
static uint8_t tft_buf[TFT_BUF_SIZE];
void TFT_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) // Tô màu hình chữ nhật tại (x, y) với chiều rộng w, chiều cao h và màu color
{
    // 1. Kiểm tra giới hạn
    if (x >= TFT_WIDTH || y >= TFT_HEIGHT) return;
    if (x + w > TFT_WIDTH)  w = TFT_WIDTH - x;
    if (y + h > TFT_HEIGHT) h = TFT_HEIGHT - y;

    // 2. Set vùng vẽ
    TFT_SetAddress(x, y, w, h);

    // 3. Chuẩn bị buffer (chỉ làm 1 lần)
    uint8_t hi = color >> 8;   
    uint8_t lo = color & 0xFF;
    for (uint16_t i = 0; i < TFT_BUF_SIZE; i += 2)
    {
        tft_buf[i]     = hi;
        tft_buf[i + 1] = lo;
    }

    // 4. Bắt đầu truyền (giữ CS LOW)
    HAL_GPIO_WritePin(TFT_DC_PORT, TFT_DC_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(TFT_CS_PORT, TFT_CS_PIN, GPIO_PIN_RESET);

    uint32_t total_bytes = (uint32_t)w * h * 2; // Tổng số byte cần gửi (mỗi pixel 2 byte)

    while (total_bytes)
    {
        uint16_t send = (total_bytes > TFT_BUF_SIZE) ? TFT_BUF_SIZE : total_bytes;
        HAL_SPI_Transmit(&TFT_SPI_HANDLE, tft_buf, send, 50);
        total_bytes -= send;
    }

    // 5. Kết thúc
    HAL_GPIO_WritePin(TFT_CS_PORT, TFT_CS_PIN, GPIO_PIN_SET);
}

// ---Hàm đổi màu nền---
void TFT_FillScreen (uint16_t color) 
{
    TFT_FillRect(0, 0, TFT_WIDTH, TFT_HEIGHT, color);
}

//---Hàm vẽ Pixel---
void TFT_DrawPixel(uint16_t x, uint16_t y, uint16_t color) // Vẽ pixel tại (x, y) với màu color
{
    if (x >= TFT_WIDTH || y >= TFT_HEIGHT) return; // Kiểm tra giới hạn

    TFT_SetAddress(x, y, 1, 1); // Thiết lập vùng vẽ 1 pixel
    uint8_t data[2] = {color >> 8, color & 0xFF};
    TFT_WriteBuffer(data, 2);
}

//---Hàm vẽ Line ngang---
void TFT_DrawHLine(uint16_t x, uint16_t y, uint16_t w, uint16_t color) // Vẽ đường ngang từ (x, y) với chiều rộng w và màu color
{
    TFT_FillRect(x,y,w,1,color); // Vẽ đường ngang
}

//---Hàm vẽ Line dọc---
void TFT_DrawVLine(uint16_t x, uint16_t y, uint16_t h, uint16_t color) // Vẽ đường dọc từ (x, y) với chiều cao h và màu color
{
    TFT_FillRect(x,y,1,h,color); // Vẽ đường dọc
}

//---Hàm vẽ Rectangle---
void TFT_DrawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) // Vẽ hình chữ nhật tại (x, y) với chiều rộng w, chiều cao h và màu color
{
    TFT_DrawHLine(x, y, w, color);           // Đường trên
    TFT_DrawHLine(x, y + h - 1, w, color);   // Đường dưới
    TFT_DrawVLine(x, y, h, color);          // Đường trái
    TFT_DrawVLine(x + w - 1, y, h, color);  // Đường phải
}

//---Hàm vẽ Line bất kỳ---
void TFT_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color) // Vẽ đường thẳng từ (x0, y0) đến (x1, y1) với màu color
{
    int16_t dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int16_t dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1; 
    int16_t err = dx + dy, e2;

    while (1) {
        TFT_DrawPixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

//---Hàm vẽ Char---
void TFT_DrawChar(int x, int y, char c, uint16_t color, uint16_t bg, uint8_t size) // Vẽ ký tự c tại (x, y) với màu chữ color, màu nền bg và kích thước size
{
    if (c < 32 || c > 122) return;

    uint16_t w = 6 * size;
    uint16_t h = 8 * size;
    TFT_SetAddress(x, y, w, h);

    HAL_GPIO_WritePin(TFT_DC_PORT, TFT_DC_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(TFT_CS_PORT, TFT_CS_PIN, GPIO_PIN_RESET);

    uint8_t hi_c = color >> 8, lo_c = color;
    uint8_t hi_b = bg >> 8,    lo_b = bg;

    const uint8_t *ch = Font5x7[c - 32]; // cache font

    static uint8_t buf[256];
    uint16_t idx = 0;

    for (int j = 0; j < 8; j++)
    {
        uint8_t mask = (1 << j); // tính 1 lần

        for (int ys = 0; ys < size; ys++)
        {
            for (int i = 0; i < 6; i++)
            {
                uint8_t line = (i < 5) ? ch[i] : 0x00;

                // preload màu → giảm branch trong xs loop
                uint8_t hi = (line & mask) ? hi_c : hi_b;
                uint8_t lo = (line & mask) ? lo_c : lo_b;

                for (int xs = 0; xs < size; xs++)
                {
                    buf[idx++] = hi;
                    buf[idx++] = lo;

                    if (idx == 256)
                    {
                        HAL_SPI_Transmit(&TFT_SPI_HANDLE, buf, 256, 50);
                        idx = 0;
                    }
                }
            }
        }
    }

    if (idx) HAL_SPI_Transmit(&TFT_SPI_HANDLE, buf, idx, 50);

    HAL_GPIO_WritePin(TFT_CS_PORT, TFT_CS_PIN, GPIO_PIN_SET);
}

//---Hàm vẽ String---
void TFT_DrawString(int x, int y, const char *str, uint16_t color, uint16_t bg, uint8_t size)   // Vẽ chuỗi tại (x, y) với màu chữ color, màu nền bg và kích thước size
{
    while (*str)
    {
        TFT_DrawChar(x, y, *str, color, bg, size);
        x += 6 * size; // 5 pixel + 1 pixel space
        str++;
    }
}


