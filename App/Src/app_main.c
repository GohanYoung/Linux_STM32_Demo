#include "app_main.h"
#include "bsp_sys.h"       
#include "encoder_motor.h"
#include "oled.h"
#include "dht11.h"
#include <stdio.h>

// 全局资源句柄 (类型已经被抽象为 void*)
BSP_MutexHandle DhtMutex;
BSP_QueueHandle RpmQueue;

SensorDHT11_t env_sensor = {0};
int8_t dht11_status = DEV_ERROR;

// ==========================================
// 业务线程定义
// ==========================================
void Task_MotorControl(void const * argument) {
    int16_t new_target_rpm = 0;
    while (1) {
        // 永远死等队列，哪怕等一万年。只要不来数据，这个线程就不消耗 CPU
        // 0xFFFFFFFF 代表无限等待 (osWaitForever)
        if (BSP_Queue_Receive(RpmQueue, &new_target_rpm, 0xFFFFFFFF)) {
            // 收到蓝牙或按键发来的新指令，更新底层目标值
            Device_Motor_SetTargetRPM(new_target_rpm);
        } 
    }
}

void Task_DHT11(void const * argument) {
    SensorDHT11_t temp_sensor = {0};
    int8_t temp_status;
    while (1) {
        temp_status = Device_DHT11_Read(&temp_sensor); 
        // 0xFFFFFFFF 相当于无限等待 (WaitForever)
        if (BSP_Mutex_Lock(DhtMutex, 0xFFFFFFFF)) { 
            env_sensor = temp_sensor;
            dht11_status = temp_status;
            BSP_Mutex_Unlock(DhtMutex);
        }
        BSP_Sys_Delay(1500); 
    }
}

void Task_Bluetooth(void const * argument) {
    int16_t simulated_rpm_cmd = 300; 
    while (1) {
        // 等待硬件串口中断的数据 (此处暂用死循环延时模拟)
        BSP_Sys_Delay(5000); 
        
        // 解析到新指令后，扔进队列
        BSP_Queue_Send(RpmQueue, &simulated_rpm_cmd, 10);
        
        // 模拟指令不断变化
        simulated_rpm_cmd += 50; 
        if(simulated_rpm_cmd > 800) simulated_rpm_cmd = 100;
    }
}

void Task_OLED(void const * argument) {
    char display_buf[20];
    SensorDHT11_t local_sensor = {0};
    int8_t local_status;

    while (1) {
        int16_t tar_rpm = g_motor.target_rpm;
        int16_t cur_rpm = g_motor.real_rpm;
        int16_t cur_pwm = g_motor.current_pwm;

        if (BSP_Mutex_Lock(DhtMutex, 10)) {
            local_sensor = env_sensor;
            local_status = dht11_status;
            BSP_Mutex_Unlock(DhtMutex);
        }

        // ... (此处省略相同的 snprintf 和 Oled 显示代码) ...
        
        BSP_Sys_Delay(200); 
    }
}

// ==========================================
// 启动入口
// ==========================================
void App_Main_Run(void) {
    BSP_Sys_Delay(200); 
    Device_OLED_Init();
    Device_OLED_Clear();
    Device_Motor_Init();    //已经将控制函数挂载钩子上
    Device_DHT11_Init();
    Device_Motor_SetTargetRPM(120); 
    //开启中断TIM4
    BSP_Sys_TIM4_Start();

    // 创建纯净版 IPC 资源
    DhtMutex = BSP_Mutex_Create();
    RpmQueue = BSP_Queue_Create(4, sizeof(int16_t));

    // 创建纯净版线程 (优先级数字越大优先级越高，具体视 OS 映射而定)
    BSP_Thread_Create(Task_MotorControl, "Motor", 128, 4);
    BSP_Thread_Create(Task_Bluetooth,    "BT",    128, 3);
    BSP_Thread_Create(Task_OLED,         "OLED",  256, 2);
    BSP_Thread_Create(Task_DHT11,        "DHT",   128, 1);
}