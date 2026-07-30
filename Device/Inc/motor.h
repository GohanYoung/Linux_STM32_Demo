#ifndef __MOTOR_H
#define __MOTOR_H

#include <stdint.h>
#include "pid.h"

#define MOTOR_ENCODER_PPR  11
#define MOTOR_PID_PERIOD_S 0.01f

void Motor_Init(void);
void Motor_SetTargetRPM(int16_t rpm);
int16_t Motor_GetCurrentRPM(void);
void Motor_Start(void);
void Motor_Stop(void);
void Motor_PID_Update(void);

extern PID_Controller_t g_motor_pid;

#endif