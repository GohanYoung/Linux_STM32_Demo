#include "bsp_dht11.h"
#include "tim.h"  // 包含 htim4
#include "gpio.h" // 包含引脚宏

void BSP_DHT11_HW_Init(void) {
    // 启动 TIM4 定时器，用作微秒延时时钟
    HAL_TIM_Base_Start(&htim4);
}

void BSP_Delay_us(uint16_t us) {
    // 将定时器计数值清零
    __HAL_TIM_SET_COUNTER(&htim4, 0);
    // 死循环等待，直到计数值达到要求的微秒数
    while (__HAL_TIM_GET_COUNTER(&htim4) < us);
}

void BSP_DHT11_SetPin(uint8_t val) {
    // 开漏模式下：写 0 拉低总线，写 1 释放总线
    HAL_GPIO_WritePin(DHT11_DATA_GPIO_Port, DHT11_DATA_Pin, val ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

uint8_t BSP_DHT11_ReadPin(void) {
    // 注意：读取前，必须确保之前已经 SetPin(1) 释放了总线
    return HAL_GPIO_ReadPin(DHT11_DATA_GPIO_Port, DHT11_DATA_Pin) == GPIO_PIN_SET ? 1 : 0;
}