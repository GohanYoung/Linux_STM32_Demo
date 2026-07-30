#ifndef __PID_H
#define __PID_H

#include <stdint.h>
#include "qmath.h"

typedef struct {
    q15_t kp;
    q15_t ki;
    q15_t kd;

    q15_t target;
    q15_t feedback;

    q15_t error;
    q15_t error_prev;
    q15_t error_prev2;

    q15_t output;
    q15_t output_max;
    q15_t output_min;
} PID_Controller_t;

void  PID_Init(PID_Controller_t *pid, q15_t kp, q15_t ki, q15_t kd,
               q15_t out_max, q15_t out_min);
q15_t PID_Compute(PID_Controller_t *pid, q15_t target, q15_t feedback);
void  PID_Reset(PID_Controller_t *pid);
void  PID_SetTunings(PID_Controller_t *pid, q15_t kp, q15_t ki, q15_t kd);

#endif