#ifndef __DHT11_H
#define __DHT11_H

#include "device_cfg.h" // 引入我们之前定义的 SensorDHT11_t

// 初始化传感器模块
void Device_DHT11_Init(void);
// 读取一次温湿度，返回 DEV_OK 表示成功，DEV_ERROR 表示失败或校验错
int8_t Device_DHT11_Read(SensorDHT11_t *sensor_data);

#endif