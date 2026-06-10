#include "app_main.h"
#include "bsp_sys.h"       
#include "encoder_motor.h"
#include "oled.h"
#include "dht11.h"
#include "bsp_uart.h"
#include <stdio.h>
#include <string.h>

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
//Device_DHT11_Read中临界区终端无法进入，可能会错过一次电机的PWM纠正
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
    char cmd_buf[16];
    uint8_t cmd_idx = 0;
    uint8_t ch;
    MotorState_t motor_snapshot;
    char tx_buf[32];
    int16_t rpm;

    while (1) {
        while (BSP_UART_ReadByte(&ch)) {
            if (ch == '\r' || ch == '\n') {
                if (cmd_idx > 0) {
                    cmd_buf[cmd_idx] = '\0';

                    if (sscanf(cmd_buf, "FW %hd", &rpm) == 1) {
                        BSP_Queue_Send(RpmQueue, &rpm, 0);
                        snprintf(tx_buf, sizeof(tx_buf), "OK FW=%d\r\n", rpm);
                        BSP_UART_SendString(tx_buf);
                    }
                    else if (sscanf(cmd_buf, "fw %hd", &rpm) == 1) {
                        BSP_Queue_Send(RpmQueue, &rpm, 0);
                        snprintf(tx_buf, sizeof(tx_buf), "OK FW=%d\r\n", rpm);
                        BSP_UART_SendString(tx_buf);
                    }
                    else if (sscanf(cmd_buf, "RV %hd", &rpm) == 1) {
                        rpm = -rpm;
                        BSP_Queue_Send(RpmQueue, &rpm, 0);
                        snprintf(tx_buf, sizeof(tx_buf), "OK RV=%d\r\n", -rpm);
                        BSP_UART_SendString(tx_buf);
                    }
                    else if (sscanf(cmd_buf, "rv %hd", &rpm) == 1) {
                        rpm = -rpm;
                        BSP_Queue_Send(RpmQueue, &rpm, 0);
                        snprintf(tx_buf, sizeof(tx_buf), "OK RV=%d\r\n", -rpm);
                        BSP_UART_SendString(tx_buf);
                    }
                    else if (strcmp(cmd_buf, "STOP") == 0 || strcmp(cmd_buf, "stop") == 0) {
                        rpm = 0;
                        BSP_Queue_Send(RpmQueue, &rpm, 0);
                        BSP_UART_SendString("STOP\r\n");
                    }
                    else if (strcmp(cmd_buf, "STATUS") == 0 || strcmp(cmd_buf, "status") == 0) {
                        Device_Motor_GetState(&motor_snapshot);
                        snprintf(tx_buf, sizeof(tx_buf),
                                 "T:%d R:%d P:%d\r\n",
                                 motor_snapshot.target_rpm,
                                 motor_snapshot.real_rpm,
                                 motor_snapshot.current_pwm);
                        BSP_UART_SendString(tx_buf);
                    }
                    else {
                        BSP_UART_SendString("ERR\r\n");
                    }
                    cmd_idx = 0;
                }
            } else if (cmd_idx < sizeof(cmd_buf) - 1) {
                cmd_buf[cmd_idx++] = ch;
            }
        }
        BSP_Sys_Delay(50);
    }
}

void Task_OLED(void const * argument) {
    char display_buf[32];
    SensorDHT11_t local_sensor = {0};
    int8_t local_status = DEV_ERROR;

    MotorState_t motor_snapshot;
    
    while (1) {
        Device_Motor_GetState(&motor_snapshot);
        int16_t tar_rpm = motor_snapshot.target_rpm;
        int16_t cur_rpm = motor_snapshot.real_rpm;
        int16_t cur_pwm = motor_snapshot.current_pwm;

        if (BSP_Mutex_Lock(DhtMutex, 10)) {
            local_sensor = env_sensor;
            local_status = dht11_status;
            BSP_Mutex_Unlock(DhtMutex);
        }

        // 第一行：目标转速 & 实际转速
        snprintf(display_buf, sizeof(display_buf), "T:%4d R:%4d", tar_rpm, cur_rpm);
        Device_OLED_ShowString(20, 2, display_buf);

        // 第二行：PWM & 温湿度（DHT11 离线则显示 "--"）
        if (local_status == DEV_OK) {
            snprintf(display_buf, sizeof(display_buf), "P:%4d T:%02d H:%02d",
                     cur_pwm, local_sensor.temperature / 10, local_sensor.humidity / 10);
        } else {
            snprintf(display_buf, sizeof(display_buf), "P:%4d T:-- H:--", cur_pwm);
        }
        Device_OLED_ShowString(20, 4, display_buf);
        
        BSP_Sys_Delay(200); 
    }
}

// ==========================================
// 启动入口
// ==========================================
void App_Main_Run(void) {
    BSP_HAL_Delay(200); 
    BSP_Delay_us_Init(); //开启DWT(微秒)
    BSP_UART_Init(); 
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