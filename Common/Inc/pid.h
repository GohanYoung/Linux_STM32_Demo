#ifndef __PID_H
#define __PID_H

#include <stdint.h>

// PID 运算缩放因子：所有浮点系数在初始化时预乘此值转为定点数
// 运行时全程整数运算，避免 STM32F103（无 FPU）的软件浮点开销
#define PID_SCALE_BITS  10
#define PID_SCALE       (1 << PID_SCALE_BITS)  // 1024

typedef struct {
    int32_t Kp_scaled;   // Kp * PID_SCALE
    int32_t Ki_scaled;   // Ki * dt * PID_SCALE (dt 已吸收进系数)
    int32_t Kd_scaled;   // Kd / dt * PID_SCALE (dt 已吸收进系数)
    
    int32_t target;      // 目标设定值
    int32_t last_target; // 上一次目标值，用于检测目标切换
    int32_t current;     // 当前反馈值
    
    int32_t error;       // 当前误差 e(k)
    int32_t last_error;  // 上次误差 e(k-1)
    int64_t integral;    // 误差累加 (int64 防溢出)
    
    int32_t out;         // PID 输出值
    int32_t out_max;     // 输出上限
    int32_t out_min;     // 输出下限
    int64_t integral_max;// 积分上限 (int64 防溢出)
} PID_Controller_t;

// 初始化：传入浮点系数和 dt，内部自动转为定点数
void PID_Init(PID_Controller_t *pid, float p, float i, float d, float dt,
              int32_t out_max, int32_t integral_max);

// 运行时计算：全整数运算，适合 ISR / 无 FPU 场景
int32_t PID_Calc(PID_Controller_t *pid, int32_t target, int32_t current);

#endif