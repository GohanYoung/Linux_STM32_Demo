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

EncoderMotor_t g_motor;
static PID_Controller_t speed_pid;

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
    // 参数说明：Kp=5.0, Ki=0.5, Kd=0.1, 输出限幅=1000(PWM最大值), 积分限幅=800
    // 注意：这里的 P, I, D 是预设初始值，实际需要根据电机特性曲线用上位机调参
    PID_Init(&speed_pid, 5.0f, 0.5f, 0.1f, 1000.0f, 800.0f);
}

void Device_Motor_SetTargetRPM(int16_t target_rpm) {
    g_motor.target_rpm = target_rpm;
}

void Device_Motor_ControlLoop(void) {
    // 1. 从 BSP 层获取增量脉冲
    int16_t delta_pulse = BSP_Motor_GetEncoderDelta();
    g_motor.total_pulse += delta_pulse; // 累计用于里程计算

    // 2. 将脉冲增量转换为真实的物理转速 RPM
    // 公式: RPM = (脉冲增量 / 编码器一圈总脉冲) * (60秒 / 控制周期秒)
    // RPM = (delta_pulse / 1320) * (60 / 0.01) = (delta_pulse * 6000) / 1320
    g_motor.real_rpm = (int16_t)(((float)delta_pulse * 6000.0f) / MOTOR_PULSE_PER_ROUND);

    // 3. 将目标速度和实际速度喂给 PID，得出需要的控制量 (PWM)
    float pid_out = PID_Calc(&speed_pid, (float)g_motor.target_rpm, (float)g_motor.real_rpm);
    
    // 4. 将浮点控制量转为整数并缓存记录
    g_motor.current_pwm = (int16_t)pid_out;
    
    // 5. 调用 BSP 层将 PWM 下发到硬件
    BSP_Motor_SetPWM(g_motor.current_pwm);
}