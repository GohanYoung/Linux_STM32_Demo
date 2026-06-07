#ifndef __BSP_SYS_H
#define __BSP_SYS_H

#include <stdint.h>

// 获取系统运行毫秒数
uint32_t BSP_Sys_GetTick(void);

// 毫秒级延时 (底层自动判断：有 RTOS 就引发任务调度，无 RTOS 就死等)
void BSP_Sys_Delay(uint32_t ms);

// 进入临界区 (关闭中断/挂起调度器，保护极速时序)
void BSP_Sys_EnterCritical(void);

// 退出临界区 (恢复中断/调度器)
void BSP_Sys_ExitCritical(void);

#endif /* __BSP_SYS_H */