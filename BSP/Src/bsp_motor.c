#include "bsp_motor.h"
#include "tim.h"  // 必须在 CubeMX 开启 TIM2 和 TIM3
#include "gpio.h" // 包含方向引脚的宏定义


void BSP_Motor_HW_Init(void) {
    // 启动 TIM2 的通道 1 作为 PWM 输出
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
    // 启动 TIM3 及其两个通道作为正交编码器模式
    HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL);
    // 初始状态电机刹车/停止
    BSP_Motor_SetPWM(0);
}

int16_t BSP_Motor_GetEncoderDelta(void) {
    // 获取 TIM3 的当前计数值
    int16_t count = (int16_t)__HAL_TIM_GET_COUNTER(&htim3);
    // 获取完立刻清零，这样每次读到的就是单位时间内的脉冲增量
    __HAL_TIM_SET_COUNTER(&htim3, 0);
    return count;
}

void BSP_Motor_SetPWM(int16_t pwm_val) {
    // 1. 判断方向并设置 IN1, IN2 引脚电平
    if (pwm_val > 0) {
        // 正转
        HAL_GPIO_WritePin(MOTOR_IN1_GPIO_Port, MOTOR_IN1_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(MOTOR_IN2_GPIO_Port, MOTOR_IN2_Pin, GPIO_PIN_RESET);
    } else if (pwm_val < 0) {
        // 反转
        HAL_GPIO_WritePin(MOTOR_IN1_GPIO_Port, MOTOR_IN1_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(MOTOR_IN2_GPIO_Port, MOTOR_IN2_Pin, GPIO_PIN_SET);
        pwm_val = -pwm_val; // 取绝对值用于设置 PWM 占空比
    } else {
        // 停止/刹车
        HAL_GPIO_WritePin(MOTOR_IN1_GPIO_Port, MOTOR_IN1_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(MOTOR_IN2_GPIO_Port, MOTOR_IN2_Pin, GPIO_PIN_RESET);
    }

    // 2. 限幅保护 (防止传入异常值导致寄存器溢出)
    // 自动从 TIM 句柄获取 ARR 值，与 CubeMX 配置保持同步
    if (pwm_val > (int16_t)(htim2.Init.Period + 1)) {
        pwm_val = (int16_t)(htim2.Init.Period + 1);
    }

    // 3. 设置 TIM2 通道 1 的 CCR 寄存器，改变占空比
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, (uint32_t)pwm_val);
}

