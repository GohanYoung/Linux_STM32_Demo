#include "encoder_motor.h"
#include "bsp_motor.h"
#include "bsp_sys.h"
#include "pid.h"

// 电机物理参数 (根据 MG310 实际参数调整)
// 假设减速比 1:20，编码器线数 11，TIM3配置为 4倍频
// 轴转一圈脉冲数 = 11 * 20 * 4 = 880
#define MOTOR_PULSE_PER_ROUND 880.0f
// 控制周期 10ms (需要在调用 Device_Motor_ControlLoop 的定时器中保证这个频率)
#define CONTROL_PERIOD_MS 10.0f 

static PID_Controller_t speed_pid;
static MotorState_t g_motor;

void Device_Motor_GetState(MotorState_t *out_state) {
    BSP_Sys_EnterCritical();       // 关中断，防止 ISR 写到一半
    *out_state = g_motor;          // 原子拷贝整个结构体
    BSP_Sys_ExitCritical();        // 开中断
}

void Device_Motor_Init(void) {
    // 1. 初始化底层定时器
    BSP_Motor_HW_Init();
    
    // 关键一步：把当前的控制回路函数“注册”到底层中断的钩子上去！
    BSP_RegisterCallback(Device_Motor_ControlLoop);

    // 2. 清理状态数据
    g_motor.target_rpm = 0;
    g_motor.real_rpm = 0;
    g_motor.current_pwm = 0;
    g_motor.total_pulse = 0;
    
    // 3. 初始化 PID 算法
    // Kp=5.0, Ki=50.0, Kd=0.001, dt=0.01, 输出限幅=1000, 积分限幅=800
    PID_Init(&speed_pid, 5.0f, 50.0f, 0.001f, 0.01f, 1000, 800);
}

void Device_Motor_SetTargetRPM(int16_t target_rpm) {
    g_motor.target_rpm = target_rpm;
}

void Device_Motor_ControlLoop(void) {
    // 1. 从 BSP 层获取增量脉冲
    int16_t delta_pulse = BSP_Motor_GetEncoderDelta();
    g_motor.total_pulse += delta_pulse;

    // 2. 脉冲增量 → 转速 RPM (纯整数，无浮点)
    //    公式: RPM = delta_pulse * (60000 / (880 * 10))
    //    化简: RPM = delta_pulse * 75 / 11
    g_motor.real_rpm = (int16_t)(((int32_t)delta_pulse * 75) / 11);

    // 3. 整数 PID 计算 (运算在 ISR 中全硬件整数执行，无 FPU 开销)
    int32_t pid_out = PID_Calc(&speed_pid, (int32_t)g_motor.target_rpm, (int32_t)g_motor.real_rpm);
    
    g_motor.current_pwm = (int16_t)pid_out;
    
    // 4. 下发 PWM (BSP 内部自动根据 htim2.Init.Period 限幅)
    BSP_Motor_SetPWM(g_motor.current_pwm);
}