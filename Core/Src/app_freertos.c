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
#define OVERVOLTAGE 41000
#define UNDERTEMP 25700
#define OVERTEMP 9900

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
extern cell_asic IC[TOTAL_IC];

uint16_t max_voltages[TOTAL_SEGMENTS];
uint16_t min_voltages[TOTAL_SEGMENTS];
uint16_t max_temps[TOTAL_SEGMENTS];
uint16_t min_temps[TOTAL_SEGMENTS];
uint32_t packVoltage = 0;

uint32_t err;
uint32_t rx;
uint32_t tx;
FDCAN_ErrorCountersTypeDef error_counts;

uint16_t temps[TOTAL_IC][TEMPS_PER_IC];

bool firstMeasurementDone = false;
bool fault_state = false;
uint8_t fault_mask = 0;
uint8_t fault_data[3];

volatile uint16_t ELCON_MaxVoltage = 0;
volatile uint16_t ELCON_MaxCurrent = 0;
volatile bool startCharging = false;
volatile bool stopCharging = false;
extern CAN_RingBuffer_t canTxBuf;

volatile uint16_t target_voltage = 30000;
volatile bool startBalancing = false;
volatile bool stopBalancing = false;

FDCAN_TxHeaderTypeDef hTxHeader;

extern uint32_t IVTS_Current;
extern uint16_t delta_t;
uint16_t SoC = 0;

uint8_t balancingDone = 0;

FDCAN_ProtocolStatusTypeDef status;

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
/* Definitions for ChargingTimer */
osTimerId_t ChargingTimerHandle;
const osTimerAttr_t ChargingTimer_attributes = {
  .name = "ChargingTimer"
};
/* Definitions for canTimer */
osTimerId_t canTimerHandle;
const osTimerAttr_t canTimer_attributes = {
  .name = "canTimer"
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

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */
  /* creation of ChargingTimer */
  ChargingTimerHandle = osTimerNew(ChargingTimerCallback, osTimerPeriodic, NULL, &ChargingTimer_attributes);

  /* creation of canTimer */
  canTimerHandle = osTimerNew(canTimer, osTimerPeriodic, NULL, &canTimer_attributes);

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
  osThreadFlagsWait(0x01, osFlagsWaitAny, osWaitForever);
  TickType_t lastWakeTime = xTaskGetTickCount();

  for(;;)
  {
	  osMutexAcquire(icLockHandle, osWaitForever);
	  print_cell_voltages(TOTAL_IC, IC);
	  print_temps_25(temps);	// 25 CODE
	  print_faults(fault_mask);
	  osMutexRelease(icLockHandle);

	  //print_cell_temps(TOTAL_IC, IC);			// 26 CODE

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
	  if (osMutexAcquire(icLockHandle, osWaitForever) == osOK) {
		  read_cell_voltages(TOTAL_IC, IC);

		  if (osMutexAcquire(tempLockHandle, osWaitForever) == osOK) {
			  read_temps_25(TOTAL_IC, IC, temps);

			  if (osMutexAcquire(canDataLockHandle, osWaitForever) == osOK) {
				  temp_analytics(TOTAL_IC, temps, max_temps, min_temps);
				  packVoltage = voltage_analytics(TOTAL_IC, IC, max_voltages, min_voltages);
				  osMutexRelease(canDataLockHandle);
			  }

			  osMutexRelease(tempLockHandle);
		  }
		  osMutexRelease(icLockHandle);
	  }

	  /*if (IVTS_Current == 0) {
		  SoC = soc_ocv(packVoltage);
	  }

	  else {
		  SoC = soc_cc(IVTS_Current, SoC, delta_t);
	  }*/

	  if (!firstMeasurementDone) {
		  firstMeasurementDone = true;
		  osThreadFlagsSet(SerialTaskHandle, 0x01);
		  osThreadFlagsSet(SafetyTaskHandle, 0x01);
		  osThreadFlagsSet(CANTaskHandle, 0x01);
		  osThreadFlagsSet(BalancingTaskHandle, 0x01);
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
  osThreadFlagsWait(0x01, osFlagsWaitAny, osWaitForever);
  TickType_t lastWakeTime = xTaskGetTickCount();
  /* Infinite loop */
  for(;;)
  {
	  if (osMutexAcquire(icLockHandle, osWaitForever) == osOK) {
		  fault_state |= check_uv_ov_fault(TOTAL_IC, IC, UNDERVOLTAGE, OVERVOLTAGE, &fault_mask, fault_data);
		  osMutexRelease(icLockHandle);
	  }

	  if (osMutexAcquire(tempLockHandle, osWaitForever) == osOK) {
		  fault_state |= check_ut_ot_fault(TOTAL_IC, temps, UNDERTEMP, OVERTEMP, &fault_mask, fault_data);
		  osMutexRelease(tempLockHandle);
	  }

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
  osThreadFlagsWait(0x01, osFlagsWaitAny, osWaitForever);
  TickType_t lastWakeTime = xTaskGetTickCount();
  //osTimerStart(canTimerHandle, pdMS_TO_TICKS(100));
  /* Infinite loop */
  for(;;)
  {
	err = HAL_FDCAN_GetError(&hfdcan1);
	tx = HAL_FDCAN_GetTxFifoFreeLevel(&hfdcan1);
	HAL_FDCAN_GetProtocolStatus(&hfdcan1, &status);
	HAL_FDCAN_GetErrorCounters(&hfdcan1, &error_counts);

	if (osMutexAcquire(canDataLockHandle, osWaitForever) == osOK) {
		CAN_Logging(&hfdcan1, max_voltages, min_voltages, max_temps, min_temps, packVoltage);
		osMutexRelease(canDataLockHandle);
	}

	if (fault_state) {
		FDCAN_SendFault(&hfdcan1, &hTxHeader, fault_mask, fault_data);
	}

	// Charging Code
	if (startCharging) {
		CAN_Charging(&fault_state);
		osTimerStart(ChargingTimerHandle, pdMS_TO_TICKS(1000));
	}
	if (stopCharging) {
		FDCAN_StopCharging();
		osTimerStop(ChargingTimerHandle);
	}

	osMutexAcquire(canDataLockHandle, osWaitForever);
	CAN_TX_Process(&canTxBuf);
	osMutexRelease(canDataLockHandle);

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
  osThreadFlagsWait(0x01, osFlagsWaitAny, osWaitForever);
  TickType_t lastWakeTime = xTaskGetTickCount();
  /* Infinite loop */
  for(;;)
  {
	  if (osMutexAcquire(icLockHandle, osWaitForever) == osOK) {
		  if (startBalancing && !balancingDone && !fault_state) {
			  balancingDone = balance_cells(TOTAL_IC, IC, target_voltage);
		  }
		  osMutexRelease(icLockHandle);
	  }

	  vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(100));
  }
  /* USER CODE END BalancingTask */
}

/* ChargingTimerCallback function */
void ChargingTimerCallback(void *argument)
{
  /* USER CODE BEGIN ChargingTimerCallback */
  FDCAN_SendChargerMessage(ELCON_MaxVoltage, ELCON_MaxCurrent, 0);
  /* USER CODE END ChargingTimerCallback */
}

/* canTimer function */
void canTimer(void *argument)
{
  /* USER CODE BEGIN canTimer */
  osMutexAcquire(canDataLockHandle, osWaitForever);
  CAN_TX_Process(&canTxBuf);
  osMutexRelease(canDataLockHandle);
  /* USER CODE END canTimer */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

