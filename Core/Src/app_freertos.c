/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : app_freertos.c
  * Description        : FreeRTOS applicative file
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
#include "app_freertos.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stm32h5xx_hal.h"
#include "ltc6813.h"
#include "ltc681x.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define TOTAL_IC 10
#define UNDERTEMP 2000
#define OVERTEMP 29500
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
extern cell_asic IC[TOTAL_IC];
TaskHandle_t measurement_handle;
TaskHandle_t serial_handle;

/* USER CODE END Variables */
/* Definitions for SerialTask */
osThreadId_t SerialTaskHandle;
const osThreadAttr_t SerialTask_attributes = {
  .name = "SerialTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 512 * 4
};
/* Definitions for MeasurementTask */
osThreadId_t MeasurementTaskHandle;
const osThreadAttr_t MeasurementTask_attributes = {
  .name = "MeasurementTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 512 * 4
};
/* Definitions for SafetyTask */
osThreadId_t SafetyTaskHandle;
const osThreadAttr_t SafetyTask_attributes = {
  .name = "SafetyTask",
  .priority = (osPriority_t) osPriorityRealtime,
  .stack_size = 512 * 4
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */
  /* creation of SerialTask */
  SerialTaskHandle = osThreadNew(SerialTask, NULL, &SerialTask_attributes);

  /* creation of MeasurementTask */
  MeasurementTaskHandle = osThreadNew(MeasurementTask, NULL, &MeasurementTask_attributes);

  /* creation of SafetyTask */
  SafetyTaskHandle = osThreadNew(SafetyTask, NULL, &SafetyTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}
/* USER CODE BEGIN Header_SerialTask */
/**
* @brief Function implementing the SerialTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_SerialTask */
void SerialTask(void *argument)
{
  /* USER CODE BEGIN SerialTask */
  /* Infinite loop */
  TickType_t lastWakeTime = xTaskGetTickCount();

  for(;;)
  {
	  print_cell_voltages(IC, TOTAL_IC);
	  print_cell_temps(IC, TOTAL_IC);
	  vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(500));
  }
  /* USER CODE END SerialTask */
}

/* USER CODE BEGIN Header_MeasurementTask */
/**
* @brief Function implementing the MeasurementTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_MeasurementTask */
void MeasurementTask(void *argument)
{
  /* USER CODE BEGIN MeasurementTask */
  TickType_t lastWakeTime = xTaskGetTickCount();
  /* Infinite loop */
  for(;;)
  {
	  wakeup_idle(TOTAL_IC);
	  osDelay(1);

	  LTC6813_adcv(2, 0, 0);
	  osDelay(300);  // Wait for ADC to finish

	  wakeup_idle(TOTAL_IC);
	  osDelay(1);

	  LTC681x_rdcv(TOTAL_IC, IC);


	  wakeup_idle(TOTAL_IC);
	  osDelay(1);

	  LTC6813_adax(2, 0);
	  osDelay(300);  // Wait for ADC to finish

	  wakeup_idle(TOTAL_IC);
	  osDelay(1);

	  for (int j = 1; j<5; j++) {
	  LTC681x_rdaux(0, TOTAL_IC, IC);
	  }

	  vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(500));
  }
  /* USER CODE END MeasurementTask */
}

/* USER CODE BEGIN Header_SafetyTask */
/**
* @brief Function implementing the SafetyTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_SafetyTask */
void SafetyTask(void *argument)
{
  /* USER CODE BEGIN SafetyTask */
  TickType_t lastWakeTime = xTaskGetTickCount();
  /* Infinite loop */
  for(;;)
  {
    if (check_uv_ov_fault(TOTAL_IC, IC)) {
    	FAULT_LOW();
    }
    if (check_ut_ot_fault(TOTAL_IC, IC, UNDERTEMP, OVERTEMP)) {
    	FAULT_LOW();
    }
    vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(100));
  }
  /* USER CODE END SafetyTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

