#include "app_main.h"
#include "comm.h"
#include "oled.h"
#include "dht11.h"
#include "motor.h"
#include "pid.h"
#include "bsp_sys.h"
#include <stdio.h>
#include <string.h>

/* =======================================================
 * 全局系统状态
 * ======================================================= */
SystemState_t g_sys_state = {0};
volatile uint32_t g_status_counter = 0;// 用于状态显示任务的计数器

/* =======================================================
 * App_Init: 应用层初始化 (在 RTOS 调度器启动前调用)
 * ======================================================= */
void App_Init(void)
{
    BSP_Delay_ms(100);
    memset(&g_sys_state, 0, sizeof(g_sys_state));

    Device_OLED_Init();
    Device_OLED_Clear();
    Device_OLED_ShowString(0, 0, "System Boot...");

    Device_DHT11_Init();
    Motor_Init();
    Comm_Init();

    BSP_Delay_ms(500);
    Device_OLED_Clear();
}

/* =======================================================
 * Task_Safety: 安全监控任务 (高优先级)
 *   周期: 10ms
 *   职责: 监测电机超速、系统故障，紧急停机
 * ======================================================= */
void AppTask_Safety(void *argument)
{
    (void)argument;

    for (;;) {
        if (Motor_IsRunning()) {
            osMutexAcquire(Mutex_StateHandle, osWaitForever);
            int16_t rpm = g_sys_state.motor_rpm;
            osMutexRelease(Mutex_StateHandle);

            if (rpm > 1000 || rpm < -1000) {
                osMutexAcquire(Mutex_StateHandle, osWaitForever);
                g_sys_state.system_fault = 1;
                osMutexRelease(Mutex_StateHandle);

                Motor_Stop();
            }
        }

        osDelay(10);
    }
}

/* =======================================================
 * Task_MotorPID: 电机 PID 控制任务 (高于普通优先级)
 *   周期: 10ms (100Hz)
 *   职责: 状态同步 + 电机监控 (PID 计算由 TIM4 ISR 执行)
 * ======================================================= */
void AppTask_MotorPID(void *argument)
{
    (void)argument;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xPeriod = pdMS_TO_TICKS(10);

    for (;;) {
        vTaskDelayUntil(&xLastWakeTime, xPeriod);

        osMutexAcquire(Mutex_StateHandle, osWaitForever);
        g_sys_state.motor_rpm = Motor_GetCurrentRPM();
        osMutexRelease(Mutex_StateHandle);
    }
}

/* =======================================================
 * Task_CommRX: 指令接收任务 (普通优先级)
 *   触发: 消息队列阻塞等待
 *   职责: 解析并执行控制指令
 * ======================================================= */
void AppTask_CommRX(void *argument)
{
    (void)argument;
    ControlMsg_t msg;

    for (;;) {
        if (osMessageQueueGet(Queue_ControlCmdHandle, &msg, NULL, osWaitForever) == osOK) {
            switch (msg.cmd) {
            case CMD_SET_MOTOR_RPM:
                Motor_SetTargetRPM(msg.value);

                osMutexAcquire(Mutex_StateHandle, osWaitForever);
                g_sys_state.motor_target_rpm = msg.value;
                osMutexRelease(Mutex_StateHandle);
                break;

            case CMD_START_MOTOR:
                Motor_Start();
                break;

            case CMD_STOP_MOTOR:
                Motor_Stop();
                break;

            default:
                break;
            }
        }
    }
}

/* =======================================================
 * Task_Sensor: 传感器采集任务 (低于普通优先级)
 *   周期: 1000ms
 *   职责: 读取 DHT11 温湿度数据，存入共享状态
 * ======================================================= */
void AppTask_Sensor(void *argument)
{
    (void)argument;
    SensorDHT11_t local;

    for (;;) {
        if (Device_DHT11_Read(&local) == DEV_OK) {
            osMutexAcquire(Mutex_StateHandle, osWaitForever);
            memcpy(&g_sys_state.sensor, &local, sizeof(SensorDHT11_t));
            g_sys_state.sensor.is_online = 1;
            osMutexRelease(Mutex_StateHandle);
        } else {
            osMutexAcquire(Mutex_StateHandle, osWaitForever);
            g_sys_state.sensor.is_online = 0;
            osMutexRelease(Mutex_StateHandle);
        }

        osDelay(1000);
    }
}

/* =======================================================
 * Task_Status: 状态显示任务 (最低优先级)
 *   周期: 500ms
 *   职责: 在 OLED 上刷新系统状态、传感器数据、电机信息
 * ======================================================= */
void AppTask_Status(void *argument)
{
    (void)argument;
    char buf[20];
    SensorDHT11_t local_sensor;
    int16_t  local_rpm;
    uint8_t  local_fault;

    for (;;) {
        g_status_counter++;

        osMutexAcquire(Mutex_StateHandle, osWaitForever);
        memcpy(&local_sensor, &g_sys_state.sensor, sizeof(SensorDHT11_t));
        local_rpm    = g_sys_state.motor_rpm;
        local_fault  = g_sys_state.system_fault;
        osMutexRelease(Mutex_StateHandle);

        /* Row 0: 系统状态 */
        snprintf(buf, sizeof(buf), "SYS:%lu", g_status_counter);
        Device_OLED_ShowStringLine(0, buf);

        /* Row 1: 温度 (页2-3) */
        if (local_sensor.is_online) {
            snprintf(buf, sizeof(buf), "TEMP: %d.%d C",
                        local_sensor.temperature / 10,
                        local_sensor.temperature % 10);
        } else {
            snprintf(buf, sizeof(buf), "TEMP: --.- C");
        }
        Device_OLED_ShowStringLine(2, buf);

        /* Row 2: 湿度 (页4-5) */
        if (local_sensor.is_online) {
            snprintf(buf, sizeof(buf), "HUMI: %d.%d %%",
                        local_sensor.humidity / 10,
                        local_sensor.humidity % 10);
        } else {
            snprintf(buf, sizeof(buf), "HUMI: --.- %%");
        }
        Device_OLED_ShowStringLine(4, buf);

        /* Row 3: 电机状态 (页6-7) */
        if (Motor_IsRunning()) {
            snprintf(buf, sizeof(buf), "MOTOR: %d RPM", local_rpm);
        } else {
            snprintf(buf, sizeof(buf), "MOTOR: STOP");
        }
        Device_OLED_ShowStringLine(6, buf);

        osDelay(500);
    }
}