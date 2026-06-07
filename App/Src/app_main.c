#include "app_main.h"
#include "bsp_sys.h"       // 引入纯净的系统调用
#include "encoder_motor.h" // 引入电机状态机
#include "oled.h"          // 引入屏幕驱动
#include <stdio.h>

void App_Main_Run(void) {
    // 1. 业务层的硬件初始化序列
    BSP_Sys_Delay(200); 
    Device_OLED_Init();
    Device_OLED_Clear();
    
    Device_Motor_Init();
    
    // 2. 设定业务初始状态
    Device_Motor_SetTargetRPM(120); 
    
    // 3. 调度器时间戳定义
    uint32_t last_control_tick = 0;
    uint32_t last_oled_tick = 0;
    char display_buf[20];

    // 4. 真正的业务死循环！(这里绝对看不到任何底层的影子)
    while (1) {
        // 使用 BSP 提供的纯净时钟
        uint32_t current_tick = BSP_Sys_GetTick();

        // --- 任务 1：10ms 极速闭环控制核心 ---
        if (current_tick - last_control_tick >= 10) {
            last_control_tick = current_tick;
            Device_Motor_ControlLoop(); 
        }

        // --- 任务 2：200ms 人机交互界面刷新 ---
        if (current_tick - last_oled_tick >= 200) {
            last_oled_tick = current_tick;
            
            snprintf(display_buf, sizeof(display_buf), "TAR: %4d RPM", g_motor.target_rpm);
            Device_OLED_ShowString(0, 0, display_buf);
            
            snprintf(display_buf, sizeof(display_buf), "CUR: %4d RPM", g_motor.real_rpm);
            Device_OLED_ShowString(0, 2, display_buf);
            
            snprintf(display_buf, sizeof(display_buf), "PWM: %4d   ", g_motor.current_pwm);
            Device_OLED_ShowString(0, 4, display_buf);
        }
    }
}