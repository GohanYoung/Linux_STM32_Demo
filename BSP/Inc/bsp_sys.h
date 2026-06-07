#ifndef __BSP_SYS_H
#define __BSP_SYS_H

#include <stdint.h>

// 获取系统自启动以来的毫秒数
uint32_t BSP_Sys_GetTick(void);

// 毫秒级阻塞延时 (应用层初始化时偶尔使用，主循环中禁用)
void BSP_Sys_Delay(uint32_t ms);

#endif /* __BSP_SYS_H */