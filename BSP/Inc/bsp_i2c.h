#ifndef __BSP_I2C_H
#define __BSP_I2C_H

#include <stdint.h>
#include "device_cfg.h" // 引入我们刚刚定义的错误码宏 (DEV_OK 等)

// 声明对外开放的 I2C 内存写函数 (OLED 必备)
// dev_addr: 设备I2C地址 (如OLED的0x78)
// mem_addr: 寄存器地址 (命令为0x00，数据为0x40)
// data: 数据指针
// len: 数据长度
int8_t BSP_I2C1_WriteMem(uint16_t dev_addr, uint16_t mem_addr, uint8_t *data, uint16_t len);

#endif