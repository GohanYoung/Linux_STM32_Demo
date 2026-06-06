#ifndef __PID_H
#define __PID_H

#include <stdint.h>

typedef struct {
    float Kp;           // 比例系数
    float Ki;           // 积分系数
    float Kd;           // 微分系数
    
    float target;       // 目标设定值
    float current;      // 当前反馈值
    
    float error;        // 当前误差 e(k)
    float last_error;   // 上次误差 e(k-1)
    float integral;     // 误差积分项
    
    float out;          // PID 计算输出值
    float out_max;      // 输出限幅最大值
    float out_min;      // 输出限幅最小值
    float integral_max; // 积分限幅 (Anti-windup)
} PID_Controller_t;

void PID_Init(PID_Controller_t *pid, float p, float i, float d, float out_max, float integral_max);
float PID_Calc(PID_Controller_t *pid, float target, float current);

#endif