#ifndef TFT_ILI9341_H_
#define TFT_ILI9341_H_


#include "stm32f1xx_hal.h"

//---Khai báo SPI handle---
#define TFT_SPI_HANDLE hspi1
extern SPI_HandleTypeDef TFT_SPI_HANDLE;

//---Khai báo các chân kết nối---
#define TFT_CS_PIN GPIO_PIN_2
#define TFT_CS_PORT GPIOB

#define TFT_DC_PIN GPIO_PIN_0
#define TFT_DC_PORT GPIOB       

#define TFT_RST_PIN GPIO_PIN_1
#define TFT_RST_PORT GPIOB


//---Bảng màu chuẩn 565 RGB---
#define TFT_BLACK   0x0000
#define TFT_WHITE   0xFFFF
#define TFT_RED     0xF800
#define TFT_GREEN   0x07E0
#define TFT_BLUE    0x001F
#define TFT_CYAN    0x07FF
#define TFT_MAGENTA 0xF81F
#define TFT_YELLOW  0xFFE0
#define TFT_GRAY    0x8410
#define TFT_NAVY    0x0010

//---Kích thước màn hình---
#define TFT_WIDTH  320
#define TFT_HEIGHT 240

//---Các hàm chưc năng---
void TFT_Reset(void);
void TFT_Init(void);


void TFT_FillScreen(uint16_t color);
void TFT_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void TFT_DrawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void TFT_DrawPixel(uint16_t x, uint16_t y, uint16_t color);

void TFT_DrawHLine(uint16_t x, uint16_t y, uint16_t w, uint16_t color);
void TFT_DrawVLine(uint16_t x, uint16_t y, uint16_t h, uint16_t color);
void TFT_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);

void TFT_DrawChar(int x, int y, char c, uint16_t color, uint16_t bg, uint8_t size);
void TFT_DrawString(int x, int y, const char *str, uint16_t color, uint16_t bg, uint8_t size);


#endif /* TFT_ILI9341_H_ */
