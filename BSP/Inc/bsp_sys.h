#ifndef __BSP_SYS_H
#define __BSP_SYS_H

#include <stdint.h>
#include <stdbool.h>

// 获取系统运行毫秒数
uint32_t BSP_Sys_GetTick(void);

// 毫秒级延时 (底层自动判断：有 RTOS 就引发任务调度，无 RTOS 就死等)
void BSP_Sys_Delay(uint32_t ms);

// 进入临界区 (关闭中断/挂起调度器，保护极速时序)
void BSP_Sys_EnterCritical(void);

// 退出临界区 (恢复中断/调度器)
void BSP_Sys_ExitCritical(void);

// 互斥锁接口
typedef void* BSP_MutexHandle; // 使用 void* 隐藏底层具体类型
BSP_MutexHandle BSP_Mutex_Create(void);
bool BSP_Mutex_Lock(BSP_MutexHandle mutex, uint32_t timeout_ms);
void BSP_Mutex_Unlock(BSP_MutexHandle mutex);

// 消息队列接口
typedef void* BSP_QueueHandle;
BSP_QueueHandle BSP_Queue_Create(uint32_t length, uint32_t item_size);
bool BSP_Queue_Send(BSP_QueueHandle queue, const void* item, uint32_t timeout_ms);
bool BSP_Queue_Receive(BSP_QueueHandle queue, void* item, uint32_t timeout_ms);

// 线程创建接口
typedef void (*BSP_TaskFunction_t)(void const *);
void BSP_Thread_Create(BSP_TaskFunction_t task_func, const char* name, uint32_t stack_size, uint8_t priority);

//1. 微妙延迟函数初始化
void BSP_Delay_us_Init(void);
//2. 完美的微秒延时实现
void BSP_Delay_us(uint32_t us);

#endif /* __BSP_SYS_H */