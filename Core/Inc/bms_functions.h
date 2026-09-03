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
#include "m95p32.h"

#define CAN_TX_BUFFER_SIZE 256
#define CAN_FAULT_MSG_ID 0xF0
#define CAN_PACK_DATA_MSG_ID 0xF1
#define CAN_SEGMENT1_DATA_MSG_ID 0xF2
#define CAN_SEGMENT2_DATA_MSG_ID 0xF3
#define CAN_SEGMENT3_DATA_MSG_ID 0xF4
#define CAN_SEGMENT4_DATA_MSG_ID 0xF5
#define CAN_SEGMENT5_DATA_MSG_ID 0xF6
#define FAULT_UNDERVOLTAGE  (1 << 0)
#define FAULT_OVERVOLTAGE   (1 << 1)
#define FAULT_UNDERTEMP     (1 << 2)
#define FAULT_OVERTEMP      (1 << 3)

#define CAN_IVTS_CURRENT_ID	0x521
#define CAN_IVTS_VOLTAGE1_ID 0x522
#define CAN_IVTS_VOLTAGE2_ID 0x523
#define P45B_CAPACITY	4500	// P45B capacity in mAh

#define UART_TIMEOUT 1000

#define EEPROM_CS_GPIO_Port GPIOB
#define EEPROM_CS_Pin	GPIO_PIN_12
#define LOG_BASE_ADDR   0x000200UL
#define RECORD_SIZE     256UL
#define NUM_RECORDS     16382UL
#define REC_ADDR(i)     (LOG_BASE_ADDR + ((uint32_t)(i) * RECORD_SIZE))

extern TIM_HandleTypeDef htim6;

extern SPI_HandleTypeDef hspi2;

extern FDCAN_HandleTypeDef hfdcan1;
extern FDCAN_HandleTypeDef hfdcan2;

extern FDCAN_TxHeaderTypeDef    txHeader1;
extern FDCAN_RxHeaderTypeDef    rxHeader1;

extern FDCAN_TxHeaderTypeDef    txHeader2;
extern FDCAN_RxHeaderTypeDef    rxHeader2;

extern UART_HandleTypeDef huart1;

extern bool CAN2_StartCharging;

extern cell_asic IC[TOTAL_IC];

typedef enum {
    CAN_MODE_NORMAL,
    CAN_MODE_CHARGING,
} CAN2_Mode_e;

typedef struct
{
    FDCAN_TxHeaderTypeDef header;
    uint8_t data[8];
} CAN_TxMsg_t;

typedef struct
{
    CAN_TxMsg_t buffer[CAN_TX_BUFFER_SIZE];
    volatile uint16_t head;
    volatile uint16_t tail;
} CAN_RingBuffer_t;

#pragma pack(push, 1)
typedef struct {
    uint32_t seq;            /* monotonic, never reset */
    uint16_t timestamp;      /* min + sec bige endian */
    uint8_t  voltages[140];
    uint8_t  temperatures[60];
    uint8_t  reserved[48];   /* pads record to 256 bytes, spare for later */
    uint16_t crc16;          /* CRC-16/CCITT over the fields above */
} LogRecord_t;               /* sizeof == 256 */
#pragma pack(pop)


uint16_t code_to_mV(uint16_t code);

int binary_search(const uint16_t *array, uint16_t size, uint16_t target);

void read_cell_voltages(uint8_t total_ic, cell_asic *ic);

void read_cell_temps(uint8_t total_ic, cell_asic *ic, int32_t temps[TOTAL_IC][TEMPS_PER_IC]);

uint16_t soc_ocv(uint16_t packVoltage);

uint16_t soc_cc(uint16_t I_curr, uint16_t soc_prev, uint16_t delta_t);

void print_cell_voltages(uint8_t total_ic, cell_asic *ic);

void print_cell_temps(uint8_t total_ic, int32_t temps[TOTAL_IC][TEMPS_PER_IC]);

void print_faults(uint8_t fault_mask);

bool check_uv_ov_fault(uint8_t total_ic, cell_asic *ic, uint16_t uv, uint16_t ov, uint8_t *mask, uint8_t *data);

bool check_ut_ot_fault(uint8_t total_ic, cell_asic *ic, uint16_t ut, uint16_t ot, uint8_t *mask, uint8_t *data);

uint32_t voltage_analytics(uint8_t total_ic, cell_asic *ic, uint16_t max_voltages[TOTAL_SEGMENTS], uint16_t min_voltages[TOTAL_SEGMENTS]);

void temp_analytics(uint8_t total_ic, int32_t temps[TOTAL_IC][TEMPS_PER_IC], int32_t max_temps[TOTAL_SEGMENTS], int32_t min_temps[TOTAL_SEGMENTS]);

void FDCAN1_Init(FDCAN_HandleTypeDef* fdcanHandle);

void FDCAN2_Init(FDCAN_HandleTypeDef* fdcanHandle);

void FDCAN_SendCellData(
        FDCAN_HandleTypeDef* hfdcan,
		uint32_t can_id,
        uint16_t minV,
        uint16_t maxV,
        int32_t minT,
        int32_t maxT
    );

void FDCAN_SendPackData(
        FDCAN_HandleTypeDef* hfdcan,
        uint32_t pack_voltage
    );

void CAN_Logging(FDCAN_HandleTypeDef* hfdcan, uint16_t max_voltages[TOTAL_SEGMENTS], uint16_t min_voltages[TOTAL_SEGMENTS], int32_t max_temps[TOTAL_SEGMENTS], int32_t min_temps[TOTAL_SEGMENTS], uint32_t packVoltage);

void FDCAN_SendFault(
        FDCAN_HandleTypeDef* hfdcan,
        FDCAN_TxHeaderTypeDef* hTxHeader,
        uint8_t bitmask,
		uint8_t *fault_data
    );

void FDCAN_SendChargerMessage(uint16_t maxVoltage, uint16_t maxCurrent, uint8_t enable);

void FDCAN_StartCharging();

void FDCAN_StopCharging();

void CAN_Charging(bool *fault_state);

bool CAN_TX_Enqueue(volatile CAN_RingBuffer_t *q, CAN_TxMsg_t *msg);

void CAN_TX_Process(CAN_RingBuffer_t *q);

uint8_t balance_cells(int8_t total_ic, cell_asic *ic, uint16_t target_voltage);

void EEPROM_Init(M95_Object_t *eeprom_obj);

void EEPROM_Process_Voltages(uint8_t total_ic, cell_asic *ic, uint8_t *write_data);

void EEPROM_Process_Temps(uint8_t total_ic, cell_asic *ic, uint8_t *write_data);

uint16_t crc16(const uint8_t *data, size_t length);

uint16_t getTimestamp(uint64_t ms);

int32_t EEPROM_Write(M95_Object_t *pObj, uint16_t timestamp,
                         uint32_t index, uint32_t *pSeq,
                         const uint8_t *voltages, const uint8_t *temps);

int32_t EEPROM_FindStart(M95_Object_t *pObj, uint32_t *outHead, uint32_t *outNextSeq);

int32_t EEPROM_TransmitAll(UART_HandleTypeDef *huart, M95_Object_t *pObj);

bool select_temp(uint8_t total_ic, cell_asic *ic, uint8_t channel);

void read_temps_25(uint8_t total_ic, cell_asic *ic, uint16_t temps[TOTAL_IC][TEMPS_PER_IC]);

void print_temps_25(int32_t temps[TOTAL_IC][TEMPS_PER_IC]);

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

static inline void packS16(int16_t val, uint8_t* buf) {
    buf[0] = (uint8_t)((val >> 8) & 0xFF);
    buf[1] = (uint8_t)(val & 0xFF);
}

static const uint16_t ocv_lookup[201] = {
    41844,
    41744,
    41644,
    41544,
    41452,
    41370,
    41300,
    41238,
    41184,
    41136,
    41094,
    41056,
    41023,
    40993,
    40966,
    40942,
    40919,
    40898,
    40879,
    40860,
    40843,
    40825,
    40808,
    40792,
    40774,
    40755,
    40734,
    40711,
    40686,
    40657,
    40625,
    40590,
    40553,
    40512,
    40468,
    40422,
    40374,
    40324,
    40272,
    40218,
    40164,
    40109,
    40052,
    39996,
    39939,
    39883,
    39827,
    39772,
    39719,
    39666,
    39615,
    39565,
    39516,
    39469,
    39423,
    39379,
    39335,
    39293,
    39250,
    39208,
    39165,
    39120,
    39074,
    39024,
    38973,
    38923,
    38874,
    38827,
    38782,
    38737,
    38694,
    38650,
    38604,
    38557,
    38510,
    38462,
    38414,
    38366,
    38319,
    38272,
    38226,
    38180,
    38135,
    38091,
    38046,
    38001,
    37956,
    37912,
    37866,
    37822,
    37776,
    37731,
    37685,
    37640,
    37594,
    37548,
    37502,
    37456,
    37410,
    37364,
    37318,
    37271,
    37225,
    37178,
    37132,
    37084,
    37035,
    36984,
    36931,
    36876,
    36822,
    36772,
    36724,
    36676,
    36630,
    36584,
    36539,
    36494,
    36449,
    36405,
    36362,
    36319,
    36276,
    36231,
    36184,
    36136,
    36085,
    36034,
    35982,
    35931,
    35883,
    35834,
    35786,
    35735,
    35682,
    35627,
    35575,
    35526,
    35481,
    35437,
    35396,
    35355,
    35315,
    35274,
    35230,
    35185,
    35136,
    35083,
    35022,
    34947,
    34870,
    34803,
    34739,
    34670,
    34598,
    34526,
    34455,
    34386,
    34319,
    34253,
    34188,
    34122,
    34056,
    33989,
    33921,
    33851,
    33777,
    33700,
    33616,
    33525,
    33429,
    33330,
    33231,
    33132,
    33033,
    32935,
    32837,
    32740,
    32643,
    32545,
    32445,
    32343,
    32238,
    32128,
    32014,
    31894,
    31768,
    31630,
    31477,
    31304,
    31108,
    30888,
    30639,
    30358,
    30039,
    29672,
    29244,
    28740,
    28134,
    27384,
    24998
};



#endif /* INC_BMS_FUNCTIONS_H_ */
