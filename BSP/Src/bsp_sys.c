#include "bsp_sys.h"
#include "stm32f1xx_hal.h" // 整个工程中，只有 BSP 层的 .c 文件才有资格包含这个头文件！

uint32_t BSP_Sys_GetTick(void) {
    return HAL_GetTick();
}

void BSP_Sys_Delay(uint32_t ms) {
    HAL_Delay(ms);
}