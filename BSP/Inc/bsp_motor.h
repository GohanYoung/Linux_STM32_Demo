#ifndef __BSP_MOTOR_H
#define __BSP_MOTOR_H

#include <stdint.h>

// 启动底层硬件 (定时器与通道)
void BSP_Motor_HW_Init(void);
// 获取单位时间内的编码器脉冲数变化量 (带防溢出处理)
int16_t BSP_Motor_GetEncoderDelta(void);
// 设置电机的物理 PWM 和转向 (输入范围: -1000 到 1000)
void BSP_Motor_SetPWM(int16_t pwm_val);



#endif /* __BSP_MOTOR_H */