#ifndef __APP_MAIN_H
#define __APP_MAIN_H

#include "cmsis_os.h"
#include "device_cfg.h"

/* =======================================================
 * 1. 指令系统定义
 * ======================================================= */
typedef enum {
    CMD_NONE = 0,
    CMD_SET_MOTOR_RPM,
    CMD_START_MOTOR,
    CMD_STOP_MOTOR,
    CMD_REQUEST_STATUS,
} ControlCmd_t;

typedef struct {
    ControlCmd_t cmd;
    int16_t      value;
} ControlMsg_t;

/* =======================================================
 * 2. 系统共享状态
 * ======================================================= */
typedef struct {
    SensorDHT11_t sensor;
    int16_t       motor_rpm;
    int16_t       motor_target_rpm;
    uint8_t       system_fault;
} SystemState_t;

/* =======================================================
 * 3. RTOS 对象句柄 (定义在 freertos.c)
 * ======================================================= */
extern osMessageQueueId_t Queue_ControlCmdHandle;
extern osMutexId_t        Mutex_StateHandle;
extern osSemaphoreId_t    Sem_UartRxHandle;
extern SystemState_t      g_sys_state;
extern volatile uint32_t  g_status_counter;

/* =======================================================
 * 4. 应用层接口
 * ======================================================= */
void App_Init(void);

void AppTask_Safety(void *argument);
void AppTask_MotorPID(void *argument);
void AppTask_CommRX(void *argument);
void AppTask_Sensor(void *argument);
void AppTask_Status(void *argument);

#endif