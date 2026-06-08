/**
 * @file    bsp_sys.c
 * @brief   系统级 BSP 封装层
 * @details 本文件提供对底层 HAL 库和 RTOS 函数的统一封装，实现硬件和操作系统的解耦。
 *          所有上层代码应通过本层接口访问系统服务，便于后续移植到其他平台。
 */

#include "bsp_sys.h" 
#include "stm32f1xx_hal.h" 
#include "cmsis_os.h"  // 所有的 OS 依赖全部被拦截在这里！

/**
 * @brief   获取系统时间戳
 * @details 返回从系统启动以来的毫秒数，在 RTOS 环境下通常映射到 OS tick。
 * @note    在 FreeRTOS 环境中，HAL_GetTick 会被重定向到 xTaskGetTickCount()
 * @return  系统运行时间（单位：毫秒）
 */
uint32_t BSP_Sys_GetTick(void) {
    return HAL_GetTick();
}

/**
 * @brief   毫秒级延时（非阻塞）
 * @details 使用 RTOS 的延时函数，当前任务进入阻塞态，释放 CPU 控制权给其他任务。
 *          适用于不需要高精度的任务级延时场景。
 * @param   ms 延时时间（单位：毫秒）
 * @note    在中断服务程序（ISR）中不可调用此函数
 */
void BSP_Sys_Delay(uint32_t ms) {
    osDelay(ms);
}

/**
 * @brief   进入临界区
 * @details 禁止任务调度和中断（部分中断除外），保护临界段代码不被其他任务或中断打断。
 *          用于保护多任务环境下的共享资源访问。
 * @note    临界区应尽可能短小，避免影响系统实时性
 * @note    不可嵌套调用，必须与 BSP_Sys_ExitCritical() 配对使用
 */
void BSP_Sys_EnterCritical(void) {
    taskENTER_CRITICAL();
}

/**
 * @brief   退出临界区
 * @details 恢复任务调度和中断，允许其他任务或中断正常执行。
 *          必须与 BSP_Sys_EnterCritical() 配对使用。
 */
void BSP_Sys_ExitCritical(void) {
    taskEXIT_CRITICAL();
}

/**
 * @brief   创建互斥锁
 * @details 动态创建一个 FreeRTOS 互斥锁，用于保护共享资源的互斥访问。
 *          互斥锁支持优先级继承机制，可防止优先级反转问题。
 * @return  互斥锁句柄，创建失败返回 NULL
 */
BSP_MutexHandle BSP_Mutex_Create(void) {
    return (BSP_MutexHandle)xSemaphoreCreateMutex();
}

/**
 * @brief   获取互斥锁
 * @details 尝试获取指定的互斥锁，如果锁已被占用则阻塞等待。
 * @param   mutex       互斥锁句柄
 * @param   timeout_ms  等待超时时间（毫秒），传入 0xFFFFFFFF 表示无限等待
 * @return  获取成功返回 true，超时或失败返回 false
 */
bool BSP_Mutex_Lock(BSP_MutexHandle mutex, uint32_t timeout_ms) {
    if (mutex == NULL) return false;
    uint32_t ticks = (timeout_ms == 0xFFFFFFFF) ? portMAX_DELAY : timeout_ms;
    return (xSemaphoreTake((SemaphoreHandle_t)mutex, ticks) == pdTRUE);
}

/**
 * @brief   释放互斥锁
 * @details 释放之前获取的互斥锁，允许其他任务获取该锁。
 * @param   mutex 互斥锁句柄
 * @note    必须由持有该锁的任务调用，否则行为未定义
 */
void BSP_Mutex_Unlock(BSP_MutexHandle mutex) {
    if (mutex != NULL) {
        xSemaphoreGive((SemaphoreHandle_t)mutex);
    }
}

/**
 * @brief   创建消息队列
 * @details 动态创建一个 FreeRTOS 消息队列，用于任务间的异步通信。
 * @param   length      队列可容纳的最大消息数量
 * @param   item_size   每个消息的大小（字节）
 * @return  队列句柄，创建失败返回 NULL
 */
BSP_QueueHandle BSP_Queue_Create(uint32_t length, uint32_t item_size) {
    return (BSP_QueueHandle)xQueueCreate(length, item_size);
}

/**
 * @brief   发送消息到队列
 * @details 将消息复制到队列尾部，如果队列已满则阻塞等待。
 * @param   queue       队列句柄
 * @param   item        指向要发送消息的指针
 * @param   timeout_ms  等待超时时间（毫秒），传入 0 表示非阻塞
 * @return  发送成功返回 true，队列满且超时返回 false
 */
bool BSP_Queue_Send(BSP_QueueHandle queue, const void* item, uint32_t timeout_ms) {
    if (queue == NULL) return false;
    return (xQueueSend((QueueHandle_t)queue, item, timeout_ms) == pdTRUE);
}

/**
 * @brief   从队列接收消息
 * @details 从队列头部接收消息并复制到指定缓冲区，如果队列为空则阻塞等待。
 * @param   queue       队列句柄
 * @param   item        指向接收缓冲区的指针
 * @param   timeout_ms  等待超时时间（毫秒），传入 0xFFFFFFFF 表示无限等待
 * @return  接收成功返回 true，队列为空且超时返回 false
 */
bool BSP_Queue_Receive(BSP_QueueHandle queue, void* item, uint32_t timeout_ms) {
    if (queue == NULL) return false;
    uint32_t ticks = (timeout_ms == 0xFFFFFFFF) ? portMAX_DELAY : timeout_ms;
    return (xQueueReceive((QueueHandle_t)queue, item, ticks) == pdTRUE);
}

/**
 * @brief   创建任务线程
 * @details 动态创建一个 FreeRTOS 任务，任务创建后立即进入就绪态等待调度。
 * @param   task_func   任务入口函数指针
 * @param   name        任务名称（用于调试工具识别，最大长度由 configMAX_TASK_NAME_LEN 定义）
 * @param   stack_size  任务栈大小（单位：字，即 4 字节）
 * @param   priority    任务优先级（数字越大优先级越高，范围 0~configMAX_PRIORITIES-1）
 * @note    任务栈大小应根据任务实际需求合理分配，避免栈溢出
 */
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

// 1. DWT 初始化函数 (需要在 App_Main_Run 最开始调用一次)
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