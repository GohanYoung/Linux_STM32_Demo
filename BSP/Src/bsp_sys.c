#include "bsp_sys.h"
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"  // 所有的 OS 依赖全部被拦截在这里！

uint32_t BSP_Sys_GetTick(void) {
    return HAL_GetTick(); // RTOS 环境下，HAL_GetTick 通常也会映射到 OS tick
}

void BSP_Sys_Delay(uint32_t ms) {
    // 使用 RTOS 的延时函数，交出 CPU 控制权
    osDelay(ms); 
}

void BSP_Sys_EnterCritical(void) {
    // 隐藏具体的 OS 临界区调用
    taskENTER_CRITICAL(); 
}

void BSP_Sys_ExitCritical(void) {
    // 隐藏具体的 OS 临界区调用
    taskEXIT_CRITICAL();
}