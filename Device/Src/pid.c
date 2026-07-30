#include "pid.h"

void PID_Init(PID_Controller_t *pid, q15_t kp, q15_t ki, q15_t kd,
              q15_t out_max, q15_t out_min)
{
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;

    pid->target   = 0;
    pid->feedback = 0;

    pid->error      = 0;
    pid->error_prev = 0;
    pid->error_prev2 = 0;

    pid->output     = 0;
    pid->output_max = out_max;
    pid->output_min = out_min;
}

q15_t PID_Compute(PID_Controller_t *pid, q15_t target, q15_t feedback)
{
    q15_t p_term, i_term, d_term, d_input;

    pid->target   = target;
    pid->feedback = feedback;

    pid->error_prev2 = pid->error_prev;
    pid->error_prev  = pid->error;
    pid->error       = Q15_SUB(target, feedback);

    /* 比例项: kp * (error - error_prev) */
    p_term = Q15_MUL(pid->kp, Q15_SUB(pid->error, pid->error_prev));

    /* 积分项: ki * error */
    i_term = Q15_MUL(pid->ki, pid->error);

    /* 微分项: kd * (error - 2*error_prev + error_prev2) */
    d_input = Q15_SUB(pid->error, Q15_MUL(Q15_FROM_INT(2), pid->error_prev));
    d_input = Q15_ADD(d_input, pid->error_prev2);
    d_term  = Q15_MUL(pid->kd, d_input);

    /* 增量式输出累加并限幅 */
    pid->output = Q15_ADD(pid->output, Q15_ADD(p_term, Q15_ADD(i_term, d_term)));
    pid->output = Q15_CLAMP(pid->output, pid->output_min, pid->output_max);

    return pid->output;
}

void PID_Reset(PID_Controller_t *pid)
{
    pid->error      = 0;
    pid->error_prev = 0;
    pid->error_prev2 = 0;
    pid->output     = 0;
}

void PID_SetTunings(PID_Controller_t *pid, q15_t kp, q15_t ki, q15_t kd)
{
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
}