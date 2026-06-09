#ifndef __ENCODER_MOTOR_H
#define __ENCODER_MOTOR_H

#include <stdint.h>
#include "device_cfg.h"

// 模块初始化
void Device_Motor_Init(void);
// 设置目标转速
void Device_Motor_SetTargetRPM(int16_t target_rpm);
// 闭环控制核心回调函数 (推荐放置在 10ms 的定时器中断内)
void Device_Motor_ControlLoop(void);

void Device_Motor_GetState(MotorState_t *out_state);

#endif /* __ENCODER_MOTOR_H */