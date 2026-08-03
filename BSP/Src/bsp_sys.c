#include "bsp_sys.h"
#include "stm32f1xx_hal.h"
#include "gpio.h"
#include "tim.h" 

void BSP_Delay_ms(uint32_t ms)
{
    HAL_Delay(ms);
}

void BSP_Light_led(void) {
    for (int i = 0; i < 10; i++) {
        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
        for (volatile uint32_t d = 0; d < 720000; d++);  // 约 200ms @72MHz
        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
        for (volatile uint32_t d = 0; d < 720000; d++);
    }
}
// 微秒级延时函数，使用 TIM4 定时器
void BSP_Delay_us(uint32_t us) {
    uint32_t start = __HAL_TIM_GET_COUNTER(&htim4);
    uint32_t arr = __HAL_TIM_GET_AUTORELOAD(&htim4); // 如果是10ms中断，通常是 9999
    
    uint32_t current;
    uint32_t last = start;
    uint32_t total_elapsed = 0;

    // 不断累加逝去的时间，直到满足要求的微秒数
    while (total_elapsed < us) {
        current = __HAL_TIM_GET_COUNTER(&htim4);
        
        if (current >= last) {
            // 正常计数阶段：当前值 >= 上次值
            total_elapsed += (current - last);
        } else {
            // 溢出反转阶段：定时器达到了 ARR 归零了 (例如 9999 变成了 0)
            // 逝去时间 = (最大值 + 1) - 上次值 + 当前值
            total_elapsed += (arr + 1 - last + current);
        }
        
        last = current; // 更新上次读取的值
    }
}