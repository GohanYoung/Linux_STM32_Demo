#include "pid.h"

void PID_Init(PID_Controller_t *pid, float kp, float ki, float kd,
              float out_max, float out_min) {
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;

    pid->target = 0.0f;
    pid->feedback = 0.0f;

    pid->error = 0.0f;
    pid->error_prev = 0.0f;
    pid->error_prev2 = 0.0f;

    pid->integral = 0.0f;
    pid->integral_max = out_max;
    pid->integral_min = out_min;

    pid->output = 0.0f;
    pid->output_max = out_max;
    pid->output_min = out_min;
}

float PID_Compute(PID_Controller_t *pid, float target, float feedback) {
    pid->target = target;
    pid->feedback = feedback;

    pid->error_prev2 = pid->error_prev;
    pid->error_prev = pid->error;
    pid->error = target - feedback;

    pid->integral += pid->error;
    if (pid->integral > pid->integral_max) {
        pid->integral = pid->integral_max;
    }
    if (pid->integral < pid->integral_min) {
        pid->integral = pid->integral_min;
    }

    float p_term = pid->kp * (pid->error - pid->error_prev);
    float i_term = pid->ki * pid->error;
    float d_term = pid->kd * (pid->error - 2.0f * pid->error_prev + pid->error_prev2);

    pid->output += (p_term + i_term + d_term);
    if (pid->output > pid->output_max) {
        pid->output = pid->output_max;
    }
    if (pid->output < pid->output_min) {
        pid->output = pid->output_min;
    }

    return pid->output;
}

void PID_Reset(PID_Controller_t *pid) {
    pid->error = 0.0f;
    pid->error_prev = 0.0f;
    pid->error_prev2 = 0.0f;
    pid->integral = 0.0f;
    pid->output = 0.0f;
}

void PID_SetTunings(PID_Controller_t *pid, float kp, float ki, float kd) {
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
}