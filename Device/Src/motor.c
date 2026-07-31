#include "motor.h"
#include "tim.h"

#define MOTOR_PWM_MAX  1000
#define MOTOR_PWM_MIN  0

PID_Controller_t g_motor_pid;

static volatile int32_t  g_encoder_count;
static volatile int32_t  g_last_encoder_count;
static volatile int16_t  g_current_rpm;
static volatile uint8_t  g_motor_running;

void Motor_Init(void)
{
    g_motor_running = 0;
    g_encoder_count = 0;
    g_last_encoder_count = 0;
    g_current_rpm = 0;

    PID_Init(&g_motor_pid,
             Q15_FROM_FLOAT(1.5f),                     /* kp = 1.5 */
             Q15_FROM_FLOAT(0.05f),                    /* ki = 0.05 */
             Q15_FROM_FLOAT(0.01f),                    /* kd = 0.01 */
             Q15_FROM_INT(MOTOR_PWM_MAX),              /* out_max = 1000 */
             Q15_FROM_INT(MOTOR_PWM_MIN));             /* out_min = 0 */

    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 0);

    HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL);
}

void Motor_SetTargetRPM(int16_t rpm)
{
    g_motor_pid.target = Q15_FROM_INT((int32_t)rpm);

    if (!g_motor_running && rpm > 0) {
        Motor_Start();
    }
    if (rpm == 0) {
        Motor_Stop();
    }
}

int16_t Motor_GetCurrentRPM(void)
{
    return g_current_rpm;
}

uint8_t Motor_IsRunning(void)
{
    return g_motor_running;
}

void Motor_Start(void)
{
    PID_Reset(&g_motor_pid);
    g_motor_running = 1;
}

void Motor_Stop(void)
{
    g_motor_running = 0;
    PID_Reset(&g_motor_pid);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 0);
}

void Motor_PID_Update(void)
{
    if (!g_motor_running) {
        return;
    }

    g_encoder_count = (int32_t)__HAL_TIM_GET_COUNTER(&htim3);
    int32_t delta = g_encoder_count - g_last_encoder_count;
    g_last_encoder_count = g_encoder_count;

    /*
     * RPM 整数计算 (无浮点):
     *   rpm = delta / 0.01s / (PPR * 4) * 60
     *       = delta * 100 / (11 * 4) * 60
     *       = delta * 1500 / 11
     */
    g_current_rpm = (int16_t)(delta * 1500 / 11);

    q15_t pwm_q15 = PID_Compute(&g_motor_pid,
                                g_motor_pid.target,
                                Q15_FROM_INT((int32_t)g_current_rpm));

    int16_t pwm_value = Q15_TO_INT(pwm_q15);

    if (pwm_value > MOTOR_PWM_MAX) pwm_value = MOTOR_PWM_MAX;
    if (pwm_value < MOTOR_PWM_MIN) pwm_value = MOTOR_PWM_MIN;

    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, (uint32_t)pwm_value);
}