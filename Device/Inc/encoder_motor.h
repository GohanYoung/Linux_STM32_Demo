#ifndef __ENCODER_MOTOR_H
#define __ENCODER_MOTOR_H

#include <stdint.h>

// 电机状态机对象
typedef struct {
    int16_t target_rpm;  // 目标转速 (RPM)
    int16_t real_rpm;    // 当前实际转速 (RPM)
    int16_t current_pwm; // 当前下发的 PWM 值
    int32_t total_pulse; // 累计总脉冲数 (用于位置控制/里程计，当前仅做监控)
} EncoderMotor_t;

// 暴露给外部的电机状态对象
extern EncoderMotor_t g_motor;

// 模块初始化
void Device_Motor_Init(void);
// 设置目标转速
void Device_Motor_SetTargetRPM(int16_t target_rpm);
// 闭环控制核心回调函数 (推荐放置在 10ms 的定时器中断内)
void Device_Motor_ControlLoop(void);

#endif /* __ENCODER_MOTOR_H */