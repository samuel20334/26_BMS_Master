/*
 * bms_functions.h
 *
 *  Created on: 10 Apr 2026
 *      Author: smpet
 */

#ifndef INC_BMS_FUNCTIONS_H_
#define INC_BMS_FUNCTIONS_H_

#include "ltc6813.h"
#include "elcon.h"

#define CAN_FAULT_MSG_ID 0xF0
#define CAN_CELL_DATA_MSG_ID 0xF1
#define CAN_PACK_DATA_MSG_ID 0xF2
#define FAULT_UNDERVOLTAGE  (1 << 0)
#define FAULT_OVERVOLTAGE   (1 << 1)
#define FAULT_UNDERTEMP     (1 << 2)
#define FAULT_OVERTEMP      (1 << 3)

extern FDCAN_HandleTypeDef hfdcan1;
extern FDCAN_HandleTypeDef hfdcan2;

extern FDCAN_TxHeaderTypeDef    txHeader1;
extern FDCAN_RxHeaderTypeDef    rxHeader1;

extern FDCAN_TxHeaderTypeDef    txHeader2;
extern FDCAN_RxHeaderTypeDef    rxHeader2;

extern bool CAN2_StartCharging;

extern cell_asic IC[TOTAL_IC];

typedef enum {
    CAN_MODE_NORMAL,
    CAN_MODE_CHARGING,
} CAN2_Mode_e;


uint16_t code_to_mV(uint16_t code);

void read_cell_voltages(uint8_t total_ic, cell_asic *ic);

void read_cell_temps(uint8_t total_ic, cell_asic *ic);

void print_cell_voltages(uint8_t total_ic, cell_asic *ic);

void print_cell_temps(uint8_t total_ic, cell_asic *ic);

bool check_uv_ov_fault(uint8_t total_ic, cell_asic *ic, uint16_t uv, uint16_t ov, uint16_t *mask);

bool check_ut_ot_fault(uint8_t total_ic, uint16_t temps[TOTAL_IC][TEMPS_PER_IC], uint16_t ut, uint16_t ot, uint16_t *mask);

uint32_t voltage_analytics(uint8_t total_ic, cell_asic *ic, uint16_t *max_voltages, uint16_t *min_voltages);

void temp_analytics(uint8_t total_ic, cell_asic *ic, uint16_t *max_temps, uint16_t *min_temps);

void FDCAN1_Init(FDCAN_HandleTypeDef* fdcanHandle);

void FDCAN2_Init(FDCAN_HandleTypeDef* fdcanHandle);

void FDCAN_SendCellData(
        FDCAN_HandleTypeDef* hfdcan,
        FDCAN_TxHeaderTypeDef* hTxHeader,
        uint16_t minV,
        uint16_t maxV,
        uint16_t minT,
        uint16_t maxT
    );

void FDCAN_SendPackData(
        FDCAN_HandleTypeDef* hfdcan,
        FDCAN_TxHeaderTypeDef* hTxHeader,
        uint32_t pack_voltage
    );

void CAN_Logging(FDCAN_HandleTypeDef* hfdcan, FDCAN_TxHeaderTypeDef* hTxHeader);

void FDCAN_SendFault(
        FDCAN_HandleTypeDef* hfdcan,
        FDCAN_TxHeaderTypeDef* hTxHeader,
        uint16_t bitmask
    );

void FDCAN_SendChargerMessage(uint16_t maxVoltage, uint16_t maxCurrent, uint8_t enable);

void FDCAN_StartCharging();

void FDCAN_StopCharging();

void CAN_Charging(bool *fault_state);

uint8_t balance_cells(int8_t total_ic, cell_asic *ic, uint16_t target_voltage);

bool select_temp(uint8_t total_ic, cell_asic *ic, uint8_t channel);

void read_temps_25(uint8_t total_ic, cell_asic *ic, uint16_t temps[TOTAL_IC][TEMPS_PER_IC]);

void print_temps_25(uint16_t temps[TOTAL_IC][TEMPS_PER_IC]);

// Set fault pins high
static inline void FAULT_HIGH()
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_SET);
}

// Set fault pins low
static inline void FAULT_LOW()
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_RESET);
}

// Packs a 16-bit integer into two 8-bit ints, writing them to dst and dst+1; big endian
static inline void packU16(uint16_t val, uint8_t* dst) {
    *(uint16_t*)dst = ((val & 0x00FF) << 8) | ((val & 0xFF00) >> 8);
}


#endif /* INC_BMS_FUNCTIONS_H_ */
