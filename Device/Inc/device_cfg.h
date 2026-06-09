#ifndef __DEVICE_CFG_H
#define __DEVICE_CFG_H

#include <stdint.h>

/* =======================================================
 * 1. 系统通用状态码宏定义
 * ======================================================= */
#define DEV_OK       0
#define DEV_ERROR   -1
#define DEV_BUSY    -2
#define DEV_TIMEOUT -3

/* =======================================================
 * 2. 核心设备状态结构体定义 (面向对象思想)
 * ======================================================= */

 /**
 * @brief 温湿度传感器数据对象 (适用于 DHT11)
 */
typedef struct {
    uint8_t  is_online;      // 0:掉线, 1:在线
    int16_t  temperature;    // 温度值 ×10 (如 253 表示 25.3℃)
    int16_t  humidity;       // 湿度值 ×10 (如 450 表示 45.0%)
    uint32_t last_update_ms; // 上次更新的时间戳 (用于应用层判断数据是否过期)
} SensorDHT11_t;

/**
 * @brief 电机控制与状态反馈对象
 */
typedef struct {
    int16_t target_rpm;  // 目标转速 (RPM)
    int16_t real_rpm;    // 当前实际转速 (RPM)
    int16_t current_pwm; // 当前下发的 PWM 值
    int32_t total_pulse; // 累计总脉冲数 (用于位置控制/里程计，当前仅做监控)
} MotorState_t;

/* =======================================================
 * 3. 基础设备驱动函数指针模板 (适用于标准读写设备，如 I2C/SPI)
 * ======================================================= */
typedef struct {
    uint8_t dev_id;         //从设备地址
    int8_t (*init)(void);   //初始化
    int8_t (*write_bytes)(uint8_t reg_addr, uint8_t* data, uint16_t len);   //指定寄存器，批量写入 N 字节
    int8_t (*read_bytes)(uint8_t reg_addr, uint8_t* data, uint16_t len);    //指定寄存器，批量读取 N 字节
} DevOps_t;


# endif