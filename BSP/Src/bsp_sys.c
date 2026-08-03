#include "bsp_sys.h"
#include "stm32f1xx_hal.h"
#include "gpio.h"

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