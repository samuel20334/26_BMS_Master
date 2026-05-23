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
#include "bms_functions.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define TOTAL_IC 10
#define UNDERVOLTAGE 27000
#define OVERVOLTAGE 42000
#define UNDERTEMP 25700
#define OVERTEMP 9900

#define TARGET_VOLTAGE 28000		// Voltage to balance to
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
extern cell_asic IC[TOTAL_IC];

uint16_t max_voltages[TOTAL_IC];
uint16_t min_voltages[TOTAL_IC];
uint16_t max_temps[TOTAL_IC];
uint16_t min_temps[TOTAL_IC];
uint32_t packVoltage = 0;

uint16_t temps[TOTAL_IC][TEMPS_PER_IC];

bool firstMeasurementDone = false;
bool fault_state = false;
uint16_t fault_mask = 0;
uint8_t fault_data[3];

FDCAN_TxHeaderTypeDef hTxHeader;

uint8_t balancingDone = 0;

/* USER CODE END Variables */
/* Definitions for SerialTask */
osThreadId_t SerialTaskHandle;
const osThreadAttr_t SerialTask_attributes = {
  .name = "SerialTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 256 * 4
};
/* Definitions for MeasurementTask */
osThreadId_t MeasurementTaskHandle;
const osThreadAttr_t MeasurementTask_attributes = {
  .name = "MeasurementTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 256 * 4
};
/* Definitions for SafetyTask */
osThreadId_t SafetyTaskHandle;
const osThreadAttr_t SafetyTask_attributes = {
  .name = "SafetyTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 256 * 4
};
/* Definitions for CANTask */
osThreadId_t CANTaskHandle;
const osThreadAttr_t CANTask_attributes = {
  .name = "CANTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 256 * 4
};
/* Definitions for BalancingTask */
osThreadId_t BalancingTaskHandle;
const osThreadAttr_t BalancingTask_attributes = {
  .name = "BalancingTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 256 * 4
};
/* Definitions for icLock */
osMutexId_t icLockHandle;
const osMutexAttr_t icLock_attributes = {
  .name = "icLock"
};
/* Definitions for tempLock */
osMutexId_t tempLockHandle;
const osMutexAttr_t tempLock_attributes = {
  .name = "tempLock"
};
/* Definitions for canDataLock */
osMutexId_t canDataLockHandle;
const osMutexAttr_t canDataLock_attributes = {
  .name = "canDataLock"
};
/* Definitions for firstMeasurement */
osSemaphoreId_t firstMeasurementHandle;
const osSemaphoreAttr_t firstMeasurement_attributes = {
  .name = "firstMeasurement"
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
  /* creation of icLock */
  icLockHandle = osMutexNew(&icLock_attributes);

  /* creation of tempLock */
  tempLockHandle = osMutexNew(&tempLock_attributes);

  /* creation of canDataLock */
  canDataLockHandle = osMutexNew(&canDataLock_attributes);

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */
  /* creation of firstMeasurement */
  firstMeasurementHandle = osSemaphoreNew(1, 0, &firstMeasurement_attributes);

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

  /* creation of CANTask */
  CANTaskHandle = osThreadNew(CANTask, NULL, &CANTask_attributes);

  /* creation of BalancingTask */
  BalancingTaskHandle = osThreadNew(BalancingTask, NULL, &BalancingTask_attributes);

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
	  /*osMutexAcquire(icLockHandle, osWaitForever);
	  print_cell_voltages(TOTAL_IC, IC);
	  osMutexRelease(icLockHandle);*/

	  /*osMutexAcquire(icLockHandle, osWaitForever);
	  print_cell_temps(TOTAL_IC, IC);	// 26 CODE
	  osMutexRelease(icLockHandle);*/

	  //print_temps_25(temps);			// 25 CODE

	  vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(1000));
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
	  // 26 CODE
	  //read_cell_voltages(TOTAL_IC, IC);
	  //read_cell_temps(TOTAL_IC, IC);

	  // 25 CODE
	  osMutexAcquire(icLockHandle, osWaitForever);
	  read_cell_voltages(TOTAL_IC, IC);

	  osMutexAcquire(tempLockHandle, osWaitForever);
	  read_temps_25(TOTAL_IC, IC, temps);

	  osMutexAcquire(canDataLockHandle, osWaitForever);
	  temp_analytics(TOTAL_IC, temps, max_temps, min_temps);
	  osMutexRelease(tempLockHandle);

	  packVoltage = voltage_analytics(TOTAL_IC, IC, max_voltages, min_voltages);
	  osMutexRelease(canDataLockHandle);
	  osMutexRelease(icLockHandle);

	  if (!firstMeasurementDone) {
		  firstMeasurementDone = true;
		  osSemaphoreRelease(firstMeasurementHandle);
	  }

	  vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(1000));
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
  osSemaphoreAcquire(firstMeasurementHandle, osWaitForever);
  TickType_t lastWakeTime = xTaskGetTickCount();
  /* Infinite loop */
  for(;;)
  {
	  osMutexAcquire(icLockHandle, osWaitForever);
	  fault_state |= check_uv_ov_fault(TOTAL_IC, IC, UNDERVOLTAGE, OVERVOLTAGE, &fault_mask, fault_data);
	  osMutexRelease(icLockHandle);

	  osMutexAcquire(tempLockHandle, osWaitForever);
	  fault_state |= check_ut_ot_fault(TOTAL_IC, temps, UNDERTEMP, OVERTEMP, &fault_mask, fault_data);
	  osMutexRelease(tempLockHandle);

	  if (fault_state) {
		  FAULT_LOW();
	  }

	  vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(1000));
  }
  /* USER CODE END SafetyTask */
}

/* USER CODE BEGIN Header_CANTask */
/**
* @brief Function implementing the CANTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_CANTask */
void CANTask(void *argument)
{
  /* USER CODE BEGIN CANTask */
  TickType_t lastWakeTime = xTaskGetTickCount();
  /* Infinite loop */
  for(;;)
  {
	osMutexAcquire(canDataLockHandle, osWaitForever);
	CAN_Logging(&hfdcan1, max_voltages, min_voltages, max_temps, min_temps, packVoltage);
	osMutexRelease(canDataLockHandle);

	if (fault_state) {
		FDCAN_SendFault(&hfdcan1, &hTxHeader, fault_mask);
	}

	//CAN_Charging(&fault_state);
	vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(1000));
  }
  /* USER CODE END CANTask */
}

/* USER CODE BEGIN Header_BalancingTask */
/**
* @brief Function implementing the BalancingTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_BalancingTask */
void BalancingTask(void *argument)
{
  /* USER CODE BEGIN BalancingTask */
  TickType_t lastWakeTime = xTaskGetTickCount();
  /* Infinite loop */
  for(;;)
  {
	  osMutexAcquire(icLockHandle, osWaitForever);
	  if (!balancingDone) {
		  //balancingDone = balance_cells(TOTAL_IC, IC, TARGET_VOLTAGE);
	  }
	  osMutexRelease(icLockHandle);
	  vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(100));
  }
  /* USER CODE END BalancingTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

