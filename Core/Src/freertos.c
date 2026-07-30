/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for Task_Safety */
osThreadId_t Task_SafetyHandle;
const osThreadAttr_t Task_Safety_attributes = {
  .name = "Task_Safety",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for Task_MotorPID */
osThreadId_t Task_MotorPIDHandle;
const osThreadAttr_t Task_MotorPID_attributes = {
  .name = "Task_MotorPID",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal,
};
/* Definitions for Task_CommRX */
osThreadId_t Task_CommRXHandle;
const osThreadAttr_t Task_CommRX_attributes = {
  .name = "Task_CommRX",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for Task_Sensor */
osThreadId_t Task_SensorHandle;
const osThreadAttr_t Task_Sensor_attributes = {
  .name = "Task_Sensor",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityBelowNormal,
};
/* Definitions for Task_Status */
osThreadId_t Task_StatusHandle;
const osThreadAttr_t Task_Status_attributes = {
  .name = "Task_Status",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for Queue_ControlCmd */
osMessageQueueId_t Queue_ControlCmdHandle;
const osMessageQueueAttr_t Queue_ControlCmd_attributes = {
  .name = "Queue_ControlCmd"
};
/* Definitions for Mutex_State */
osMutexId_t Mutex_StateHandle;
const osMutexAttr_t Mutex_State_attributes = {
  .name = "Mutex_State"
};
/* Definitions for Sem_UartRx */
osSemaphoreId_t Sem_UartRxHandle;
const osSemaphoreAttr_t Sem_UartRx_attributes = {
  .name = "Sem_UartRx"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void AppTask_Safety(void *argument);
void AppTask_MotorPID(void *argument);
void AppTask_CommRX(void *argument);
void AppTask_Sensor(void *argument);
void AppTask_Status(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */
  /* Create the mutex(es) */
  /* creation of Mutex_State */
  Mutex_StateHandle = osMutexNew(&Mutex_State_attributes);

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* creation of Sem_UartRx */
  Sem_UartRxHandle = osSemaphoreNew(1, 1, &Sem_UartRx_attributes);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of Queue_ControlCmd */
  Queue_ControlCmdHandle = osMessageQueueNew (5, 16, &Queue_ControlCmd_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of Task_Safety */
  Task_SafetyHandle = osThreadNew(AppTask_Safety, NULL, &Task_Safety_attributes);

  /* creation of Task_MotorPID */
  Task_MotorPIDHandle = osThreadNew(AppTask_MotorPID, NULL, &Task_MotorPID_attributes);

  /* creation of Task_CommRX */
  Task_CommRXHandle = osThreadNew(AppTask_CommRX, NULL, &Task_CommRX_attributes);

  /* creation of Task_Sensor */
  Task_SensorHandle = osThreadNew(AppTask_Sensor, NULL, &Task_Sensor_attributes);

  /* creation of Task_Status */
  Task_StatusHandle = osThreadNew(AppTask_Status, NULL, &Task_Status_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_AppTask_Safety */
/**
  * @brief  Function implementing the Task_Safety thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_AppTask_Safety */
void AppTask_Safety(void *argument)
{
  /* USER CODE BEGIN AppTask_Safety */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END AppTask_Safety */
}

/* USER CODE BEGIN Header_AppTask_MotorPID */
/**
* @brief Function implementing the Task_MotorPID thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_AppTask_MotorPID */
void AppTask_MotorPID(void *argument)
{
  /* USER CODE BEGIN AppTask_MotorPID */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END AppTask_MotorPID */
}

/* USER CODE BEGIN Header_AppTask_CommRX */
/**
* @brief Function implementing the Task_CommRX thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_AppTask_CommRX */
void AppTask_CommRX(void *argument)
{
  /* USER CODE BEGIN AppTask_CommRX */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END AppTask_CommRX */
}

/* USER CODE BEGIN Header_AppTask_Sensor */
/**
* @brief Function implementing the Task_Sensor thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_AppTask_Sensor */
void AppTask_Sensor(void *argument)
{
  /* USER CODE BEGIN AppTask_Sensor */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END AppTask_Sensor */
}

/* USER CODE BEGIN Header_AppTask_Status */
/**
* @brief Function implementing the Task_Status thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_AppTask_Status */
void AppTask_Status(void *argument)
{
  /* USER CODE BEGIN AppTask_Status */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END AppTask_Status */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

