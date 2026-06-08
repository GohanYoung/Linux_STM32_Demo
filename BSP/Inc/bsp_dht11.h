#ifndef __BSP_DHT11_H
#define __BSP_DHT11_H

#include <stdint.h>

// 启动底层硬件（定时器）
void BSP_DHT11_HW_Init(void);
// 控制引脚电平 (0 或 1)
void BSP_DHT11_SetPin(uint8_t val);
// 读取引脚当前电平 (返回 0 或 1)
uint8_t BSP_DHT11_ReadPin(void);

#endif