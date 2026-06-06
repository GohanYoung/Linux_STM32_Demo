#include "pid.h"

void PID_Init(PID_Controller_t *pid, float p, float i, float d, float out_max, float integral_max) {
    pid->Kp = p;
    pid->Ki = i;
    pid->Kd = d;
    pid->target = 0.0f;
    pid->current = 0.0f;
    pid->error = 0.0f;
    pid->last_error = 0.0f;
    pid->integral = 0.0f;
    pid->out = 0.0f;
    pid->out_max = out_max;
    pid->out_min = -out_max;
    pid->integral_max = integral_max;
}

float PID_Calc(PID_Controller_t *pid, float target, float current) {
    pid->target = target;
    pid->current = current;
    pid->error = pid->target - pid->current;
    
    // 1. 误差积分累加
    pid->integral += pid->error;
    
    // 2. 积分限幅抗饱和 (Anti-windup)
    if (pid->integral > pid->integral_max) {
        pid->integral = pid->integral_max;
    } else if (pid->integral < -pid->integral_max) {
        pid->integral = -pid->integral_max;
    }
    
    // 3. 位置式 PID 核心差分方程计算
    pid->out = (pid->Kp * pid->error) + 
               (pid->Ki * pid->integral) + 
               (pid->Kd * (pid->error - pid->last_error));
               
    // 4. 更新历史误差
    pid->last_error = pid->error;
    
    // 5. 最终输出控制量限幅
    if (pid->out > pid->out_max) {
        pid->out = pid->out_max;
    } else if (pid->out < pid->out_min) {
        pid->out = pid->out_min;
    }
    
    return pid->out;
}