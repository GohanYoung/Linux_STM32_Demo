#ifndef __OLED_H
#define __OLED_H

#include <stdint.h>

// OLED 的默认 I2C 地址 (多数为 0x78 或 0x7A)
#define OLED_I2C_ADDR 0x78 

// 暴露给 App 层的 API
void Device_OLED_Init(void);
void Device_OLED_Clear(void);
void Device_OLED_ShowChar(uint8_t x, uint8_t y, char chr);
void Device_OLED_ShowString(uint8_t x, uint8_t y, char *chr);

#endif