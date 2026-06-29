#include "app_main.h"
#include "bsp_sys.h"       
#include "encoder_motor.h"
#include "oled.h"
#include "dht11.h"
#include <stdio.h>
#include <string.h>

BSP_MutexHandle DhtMutex;

SensorDHT11_t env_sensor = {0};
int8_t dht11_status = DEV_ERROR;

volatile int32_t motor_cycle_count = 0;

void Task_MotorControl(void const * argument) {
    while (1) {
        motor_cycle_count++;
        Device_Motor_SetTargetRPM(100);
        BSP_Sys_Delay(3000);
        Device_Motor_SetTargetRPM(0);
        BSP_Sys_Delay(3000);
        Device_Motor_SetTargetRPM(-100);
        BSP_Sys_Delay(3000);
        Device_Motor_SetTargetRPM(0);
        BSP_Sys_Delay(3000);
    }
}

void Task_DHT11(void const * argument) {
    SensorDHT11_t temp_sensor = {0};
    int8_t temp_status;
    while (1) {
        temp_status = Device_DHT11_Read(&temp_sensor); 
        if (BSP_Mutex_Lock(DhtMutex, 0xFFFFFFFF)) { 
            env_sensor = temp_sensor;
            dht11_status = temp_status;
            BSP_Mutex_Unlock(DhtMutex);
        }
        BSP_Sys_Delay(1500); 
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

        snprintf(display_buf, sizeof(display_buf), "T:%4d R:%4d", tar_rpm, cur_rpm);
        Device_OLED_ShowString(0, 0, display_buf);

        if (local_status == DEV_OK) {
            snprintf(display_buf, sizeof(display_buf), "P:%4d T:%02d H:%02d",
                     cur_pwm, local_sensor.temperature / 10, local_sensor.humidity / 10);
        } else {
            snprintf(display_buf, sizeof(display_buf), "P:%4d T:-- H:--", cur_pwm);
        }
        Device_OLED_ShowString(0, 2, display_buf);

        snprintf(display_buf, sizeof(display_buf), "CYC:%ld", motor_cycle_count);
        Device_OLED_ShowString(0, 4, display_buf);
        
        BSP_Sys_Delay(200); 
    }
}

void App_Main_Run(void) {
    BSP_HAL_Delay(200); 
    BSP_Delay_us_Init();
    Device_OLED_Init();
    Device_OLED_Clear();
    Device_Motor_Init();
    Device_DHT11_Init();
    Device_Motor_SetTargetRPM(0); 
    BSP_Sys_TIM4_Start();

    DhtMutex = BSP_Mutex_Create();

    BSP_Thread_Create(Task_MotorControl, "Motor", 128, 4);
    BSP_Thread_Create(Task_OLED,         "OLED",  256, 2);
    BSP_Thread_Create(Task_DHT11,        "DHT",   128, 1);
}