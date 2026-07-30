#ifndef __PID_H
#define __PID_H

#include <stdint.h>

typedef struct {
    float kp;
    float ki;
    float kd;

    float target;
    float feedback;

    float error;
    float error_prev;
    float error_prev2;

    float integral;
    float integral_max;
    float integral_min;

    float output;
    float output_max;
    float output_min;
} PID_Controller_t;

void PID_Init(PID_Controller_t *pid, float kp, float ki, float kd,
              float out_max, float out_min);
float PID_Compute(PID_Controller_t *pid, float target, float feedback);
void PID_Reset(PID_Controller_t *pid);
void PID_SetTunings(PID_Controller_t *pid, float kp, float ki, float kd);

#endif