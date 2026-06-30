#include "pid.h"

void PID_Init(PID_Controller_t *pid, float p, float i, float d, float dt,
              int32_t out_max, int32_t integral_max) {
    // 初始化时做一次浮点→定点转换，dt 吸收进 Ki 和 Kd
    pid->Kp_scaled = (int32_t)(p * PID_SCALE);
    pid->Ki_scaled = (int32_t)(i * dt * PID_SCALE);
    pid->Kd_scaled = (int32_t)((d / dt) * PID_SCALE);
    pid->target = 0;
    pid->last_target = 0;
    pid->current = 0;
    pid->error = 0;
    pid->last_error = 0;
    pid->integral = 0;
    pid->out = 0;
    pid->out_max = out_max;
    pid->out_min = -out_max;
    pid->integral_max = (int64_t)integral_max * PID_SCALE;
}

int32_t PID_Calc(PID_Controller_t *pid, int32_t target, int32_t current) {
    pid->target = target;
    pid->current = current;
    pid->error = pid->target - pid->current;
    
    // 0. 目标值切换时清零积分，防止上一阶段的积分残留干扰
    if (target != pid->last_target) {
        pid->integral = 0;
        pid->last_error = 0;
        pid->last_target = target;
    }
    
    // 1. 累加误差 (dt 已在 Ki_scaled 中吸收，无需重复乘 dt)
    pid->integral += (int64_t)pid->error;
    
    // 2. 积分限幅抗饱和 (Anti-windup)
    if (pid->integral > pid->integral_max) {
        pid->integral = pid->integral_max;
    } else if (pid->integral < -pid->integral_max) {
        pid->integral = -pid->integral_max;
    }
    
    // 3. 全整数 PID 差分方程 (位置式)
    int64_t p_term = (int64_t)pid->Kp_scaled * pid->error;
    int64_t i_term = (int64_t)pid->Ki_scaled * pid->integral;
    int64_t d_term = (int64_t)pid->Kd_scaled * (pid->error - pid->last_error);
    
    pid->out = (int32_t)((p_term + i_term + d_term) >> PID_SCALE_BITS);
    pid->last_error = pid->error;
    
    // 4. 输出限幅
    if (pid->out > pid->out_max) {
        pid->out = pid->out_max;
    } else if (pid->out < pid->out_min) {
        pid->out = pid->out_min;
    }
    
    return pid->out;
}