#include "bsp_sys.h" 
#include "stm32f1xx_hal.h" 
#include "cmsis_os.h"  // 所有的 OS 依赖全部被拦截在这里！
#include "tim.h"

void BSP_SET_PC13_test(){
    // 点亮PC13 LED（拉低电平）
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
    HAL_Delay(100);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
}

uint32_t BSP_Sys_GetTick(void) {
    return HAL_GetTick();
}

void BSP_HAL_Delay(uint32_t ms){
    HAL_Delay(ms);
}

void BSP_Sys_Delay(uint32_t ms) {
    osDelay(pdMS_TO_TICKS(ms));
}

void BSP_Sys_EnterCritical(void) {
    taskENTER_CRITICAL();
}

void BSP_Sys_ExitCritical(void) {
    taskEXIT_CRITICAL();
}

BSP_MutexHandle BSP_Mutex_Create(void) {
    return (BSP_MutexHandle)xSemaphoreCreateMutex();
}

bool BSP_Mutex_Lock(BSP_MutexHandle mutex, uint32_t timeout_ms) {
    if (mutex == NULL) return false;
    uint32_t ticks = (timeout_ms == 0xFFFFFFFF) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
    return (xSemaphoreTake((SemaphoreHandle_t)mutex, ticks) == pdTRUE);
}

void BSP_Mutex_Unlock(BSP_MutexHandle mutex) {
    if (mutex != NULL) {
        xSemaphoreGive((SemaphoreHandle_t)mutex);
    }
}

BSP_QueueHandle BSP_Queue_Create(uint32_t length, uint32_t item_size) {
    return (BSP_QueueHandle)xQueueCreate(length, item_size);
}

bool BSP_Queue_Send(BSP_QueueHandle queue, const void* item, uint32_t timeout_ms) {
    if (queue == NULL) return false;
    uint32_t ticks = (timeout_ms == 0xFFFFFFFF) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
    return (xQueueSend((QueueHandle_t)queue, item, ticks) == pdTRUE);
}

bool BSP_Queue_Receive(BSP_QueueHandle queue, void* item, uint32_t timeout_ms) {
    if (queue == NULL) return false;
    uint32_t ticks = (timeout_ms == 0xFFFFFFFF) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
    return (xQueueReceive((QueueHandle_t)queue, item, ticks) == pdTRUE);
}

void BSP_Thread_Create(BSP_TaskFunction_t task_func, const char* name, uint32_t stack_size, uint8_t priority) {
    xTaskCreate((TaskFunction_t)task_func, name, stack_size, NULL, priority, NULL);
}

//实现微妙级的延迟
// --- ARM Cortex-M DWT 寄存器地址定义 ---
#define DEM_CR          *(volatile uint32_t *)0xE000EDFC
#define DEM_CR_TRCENA   (1 << 24)
#define DWT_CR          *(volatile uint32_t *)0xE0001000
#define DWT_CR_CYCCNTENA (1 << 0)
#define DWT_CYCCNT      *(volatile uint32_t *)0xE0001004
// 系统时钟频率 (STM32F1 通常为 72,000,000)
extern uint32_t SystemCoreClock; 

// 1. DWT 初始化函数 (需要在 App_Main_Run 最开始调用一次)(生成微秒级别的计数)
void BSP_Delay_us_Init(void) {
    // 开启 Core Debug 模块的 Trace 功能
    DEM_CR |= DEM_CR_TRCENA; 
    // 清零周期计数器
    DWT_CYCCNT = 0; 
    // 启动计数器
    DWT_CR |= DWT_CR_CYCCNTENA; 
}

// 2. 完美的微秒延时实现
void BSP_Delay_us(uint32_t us) {
    // 计算需要等待的 CPU 时钟周期数 (如 72MHz 下，1us = 72 个周期)
    uint32_t ticks = us * (SystemCoreClock / 1000000); 
    // 记录起始时刻的节拍数
    uint32_t start_tick = DWT_CYCCNT; 
    
    // 利用无符号整型的溢出特性，完美处理 32 位计数器翻转的问题
    while ((DWT_CYCCNT - start_tick) < ticks); 
}

//开启中断定时器4
void BSP_Sys_TIM4_Start(void) {
    // 此时硬件定时器作为系统公用资源被启动
    HAL_TIM_Base_Start_IT(&htim4); 
}

/*
中断调度(后续有其他硬件中断可以添加钩子来处理其他业务函数)
*/
// 定义一个静态的函数指针变量，初始为空
static BSP_Timer_Callback_t sys_hook = NULL;
// 将函数挂载钩子上的函数
void BSP_RegisterCallback(BSP_Timer_Callback_t callback) {
    sys_hook = callback;
}
// 硬件定时器溢出中断回调
void BSP_Sys_TIM_ISR_Dispatcher(TIM_HandleTypeDef *htim) {
    
    //  处理 TIM4 的 10ms 周期中断
    if (htim->Instance == TIM4) {
        // 判断钩子上有没有挂函数，如果有，就执行它！
        // 此时 BSP 完全不知道执行的是电机控制还是别的东西，实现了 100% 解耦
        if (sys_hook != NULL) {
            sys_hook(); 
        }
    }
}