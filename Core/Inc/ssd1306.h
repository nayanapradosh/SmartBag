#ifndef SSD1306_H
#define SSD1306_H

#include "stm32f1xx_hal.h"

/* SSD1306 I2C address */
#define SSD1306_I2C_ADDR    (0x3C << 1)

/* OLED dimensions */
#define SSD1306_WIDTH       128
#define SSD1306_HEIGHT      64

/* OLED functions */
void SSD1306_Init(void);
void SSD1306_Clear(void);
void SSD1306_UpdateScreen(void);

void SSD1306_SetCursor(uint8_t x, uint8_t y);
void SSD1306_WriteChar(char ch);
void SSD1306_WriteString(char *str);

#endif /* SSD1306_H */
