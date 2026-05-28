#include "bms_functions.h"

FDCAN_TxHeaderTypeDef    txHeader1;
FDCAN_RxHeaderTypeDef    rxHeader1;

FDCAN_TxHeaderTypeDef    txHeader2;
FDCAN_RxHeaderTypeDef    rxHeader2;

uint8_t                  txData1[8];
uint8_t                  rxData1[8];

uint8_t                  txData2[8];
uint8_t                  rxData2[8];

extern uint16_t ELCON_Voltage;
extern uint16_t ELCON_Current;
extern TIM_HandleTypeDef htim6;

CAN2_Mode_e     CAN2_Mode;
bool            CAN2_StartCharging = true;

// GENERAL FUNCTIONS

uint16_t code_to_mV(uint16_t code)
{
    // LTC6813 datasheet: Vcell = code * 0.0001 V
    // Multiply by 1000 to get mV
    return (uint16_t)((code * 1000UL) / 10000UL);
}

int binary_search(const uint16_t *array, uint16_t size, uint16_t target) {
	int i = 0;
	int j = size - 1;
	int m;

	while (i <= j) {
		m = (i + j) / 2;

		if (array[m] == target) {
			return m;
		}
		else if (array[m] > target) {
			i = m + 1;
		}

		else {
			j = m - 1;
		}
	}

	// If not found, return the closes index
	if (target - array[j] <= array[i] - target) {
		return j;
	}

	else {
		return i;
	}
}

float_t ntc_to_temp(uint16_t ntc_voltage) {
    return ((-3.1598 * ((float_t)ntc_voltage/100)) + 81.327)*100;
}

uint32_t voltage_analytics(uint8_t total_ic, cell_asic *ic, uint16_t *max_voltages, uint16_t *min_voltages) {
	uint32_t packVoltage = 0;

	for (uint8_t ic_idx=0;ic_idx<total_ic-1;ic_idx++) {
		uint16_t max_voltage = 0;
		uint16_t min_voltage = 65535;

		for (uint8_t cell = 0;cell < CELLS_PER_IC;cell++) {
			uint16_t voltage_mV = code_to_mV(ic[ic_idx].cells.c_codes[cell]);
			packVoltage += voltage_mV;

			if (voltage_mV > max_voltage) {
				max_voltage = voltage_mV;
			}
			if (voltage_mV < min_voltage) {
				min_voltage = voltage_mV;
			}
		}

		max_voltages[ic_idx] = max_voltage;
		min_voltages[ic_idx] = min_voltage;
	}

	return packVoltage;
}

void temp_analytics(uint8_t total_ic, uint16_t temps[TOTAL_IC][TEMPS_PER_IC], uint16_t *max_temps, uint16_t *min_temps) {

	for (uint8_t ic_idx=0;ic_idx<total_ic-1;ic_idx++) {
		uint16_t max_temp = 0;
		uint16_t min_temp = 65535;

		for (uint8_t ch = 1;ch < TEMPS_PER_IC;ch++) {
			uint16_t voltage_mV = code_to_mV(temps[ic_idx][ch]);

			if (voltage_mV > max_temp) {
				max_temp = voltage_mV;
			}
			if (voltage_mV < min_temp) {
				min_temp = voltage_mV;
			}
		}

		max_temps[ic_idx] = max_temp;
		min_temps[ic_idx] = min_temp;
	}
}

// MEASUREMENT FUNCTIONS

void read_cell_voltages(uint8_t total_ic, cell_asic *ic) {
	wakeup_idle(total_ic);
	HAL_Delay(1);

	LTC6813_adcv(2, 0, 0);
	HAL_Delay(300);  // Wait for ADC to finish

	wakeup_idle(total_ic);
	HAL_Delay(1);

	LTC6813_rdcv(0, total_ic, ic);
}

void read_cell_temps(uint8_t total_ic, cell_asic *ic) {
	wakeup_idle(total_ic);
	HAL_Delay(1);

	LTC6813_adax(2, 0);
	HAL_Delay(300);  // Wait for ADC to finish

	wakeup_idle(total_ic);
	HAL_Delay(1);

	for (int j = 1; j<5; j++) {
		LTC6813_rdaux(j, TOTAL_IC, IC);
	}

}

uint16_t soc_ocv(uint16_t packVoltage) {
	uint16_t avg_cell_voltage = packVoltage/(CELLS_PER_IC * TOTAL_IC);
	int idx = binary_search(ocv_lookup, sizeof(ocv_lookup), avg_cell_voltage);
	uint16_t soc = (200-idx)*1000 / 200;

	return soc;
}

uint16_t soc_cc(uint16_t I_curr, uint16_t soc_prev, uint16_t delta_t) {
	return (soc_prev + (I_curr*delta_t)/P45B_CAPACITY);
}

// SERIAL FUNCTIONS

void print_cell_voltages(uint8_t total_ic, cell_asic *ic)
{
    for (uint8_t ic_idx = 0; ic_idx < total_ic; ic_idx++)
    {
        uart_print("IC ");
        uart_print_uint(ic_idx);
        uart_print(" cell voltages (mV): ");

        // Loop through all cells in this IC
        for (uint8_t cell = 0; cell < CELLS_PER_IC; cell++)
        {
            uint16_t mv = (ic[ic_idx].cells.c_codes[cell])/10;  // convert to mV
            uart_print("Cell ");
            uart_print_uint(cell + 1);
            uart_print(":");
            uart_print_uint(mv);
            uart_print(" ");

            // Optional: line break every 3 cells for readability
            if ((cell + 1) % 3 == 0) uart_print("\r\n");
        }
    }
    uart_print("\r\n");
}

void print_cell_temps(uint8_t total_ic, cell_asic *ic)
{
    for (uint8_t ic_idx = 0; ic_idx < total_ic; ic_idx++)
    {
        uart_print("IC ");
        uart_print_uint(ic_idx);
        uart_print(" cell temps (mV): ");

        // Loop through all cells in this IC
        for (uint8_t cell = 0; cell < TEMPS_PER_IC; cell++)
        {
            uint16_t mv = (ic[ic_idx].aux.a_codes[cell])/10;  // convert to mV
            uart_print("Cell ");
            uart_print_uint(cell + 1);
            uart_print(":");
            uart_print_uint(mv);
            uart_print(" ");

            // Optional: line break every 3 cells for readability
            if ((cell + 1) % 3 == 0) uart_print("\r\n");
        }
    }
    uart_print("\r\n");
}

// FAULT FUNCTIONS

bool check_uv_ov_fault(uint8_t total_ic, cell_asic *ic, uint16_t uv, uint16_t ov, uint8_t *mask, uint8_t *data) {
    bool fault = false;
    uint8_t fault_counter = 0;

    for (uint8_t ic_idx = 0; ic_idx < 9; ic_idx++) {
        for (uint8_t cell = 0; cell < CELLS_PER_IC; cell++) {
        	if (ic[ic_idx].cells.c_codes[cell] < uv) {
        		*mask |= FAULT_UNDERVOLTAGE;
        		fault = true;

        		if (fault_counter < 3) {
        			uint8_t fault_data = ((ic_idx & 0x0F) << 4) | (cell & 0x0F);
        			data[fault_counter] = fault_data;
        		}

        		fault_counter++;
        	}
        	else if (ic[ic_idx].cells.c_codes[cell] > ov) {
            	*mask |= FAULT_OVERVOLTAGE;
        		fault = true;

        		if (fault_counter < 3) {
        			uint8_t fault_data = ((ic_idx & 0x0F) << 4) | (cell & 0x0F);
        			data[fault_counter] = fault_data;
        		}

        		fault_counter++;
            }
        }
    }

    return fault;
}

bool check_ut_ot_fault(uint8_t total_ic, uint16_t temps[TOTAL_IC][TEMPS_PER_IC], uint16_t ut, uint16_t ot, uint8_t *mask, uint8_t *data)
{
    bool fault = false;
    uint8_t fault_counter = 0;

    for(uint8_t ic_idx = 0; ic_idx < total_ic-1; ic_idx++)
    {
        for(uint8_t ch = 1; ch < TEMPS_PER_IC; ch++)
        {

            if(temps[ic_idx][ch] > ut)
            {
                *mask |= FAULT_UNDERTEMP;
                fault = true;

        		if (fault_counter < 3) {
        			uint8_t fault_data = ((ic_idx & 0x0F) << 4) | (ch & 0x0F);
        			data[fault_counter] = fault_data;
        		}

        		fault_counter++;

            }
            else if(temps[ic_idx][ch] < ot)
            {
            	*mask |= FAULT_OVERTEMP;
            	fault = true;

        		if (fault_counter < 3) {
        			uint8_t fault_data = ((ic_idx & 0x0F) << 4) | (ch & 0x0F);
        			data[fault_counter] = fault_data;
        		}

        		fault_counter++;
            }
        }
    }

    return fault;
}

// CAN FUNCTIONS
void FDCAN1_Init(FDCAN_HandleTypeDef* fdcanHandle)
{
    /* IVT Current Sensor CAN IDs */
    FDCAN_FilterTypeDef filterConfig;

    filterConfig.IdType = FDCAN_STANDARD_ID;
    filterConfig.FilterIndex = 0;
    filterConfig.FilterType = FDCAN_FILTER_MASK;
    filterConfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
    filterConfig.FilterID1 = 0x520;
    filterConfig.FilterID2 = 0x5F0;

    if (HAL_FDCAN_ConfigFilter(fdcanHandle, &filterConfig) != HAL_OK)
    {
        Error_Handler();
    }

    txHeader1.Identifier = 0;
    txHeader1.IdType = FDCAN_STANDARD_ID;
    txHeader1.TxFrameType = FDCAN_DATA_FRAME;
    txHeader1.DataLength = FDCAN_DLC_BYTES_8;
    txHeader1.ErrorStateIndicator = FDCAN_ESI_PASSIVE;
    txHeader1.BitRateSwitch = FDCAN_BRS_OFF;
    txHeader1.FDFormat = FDCAN_CLASSIC_CAN;
    txHeader1.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    txHeader1.MessageMarker = 0;

    // Start CAN1
    if (HAL_FDCAN_Start(fdcanHandle) != HAL_OK) {
        Error_Handler();
    }

    // Activate notifications for new data
    if (HAL_FDCAN_ActivateNotification(fdcanHandle, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0) != HAL_OK) {
        Error_Handler();
    }
}


void FDCAN2_Init(FDCAN_HandleTypeDef* fdcanHandle)
{
    /* ELCON Charger CAN IDs */
    FDCAN_FilterTypeDef filterConfig;

    filterConfig.IdType = FDCAN_EXTENDED_ID;
    filterConfig.FilterIndex = 0;
    filterConfig.FilterType = FDCAN_FILTER_DUAL;
    filterConfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO1;
    filterConfig.FilterID1 = 0x1806E5F4; // BMS -> Charger
    filterConfig.FilterID2 = 0x18FF50E5; // Charger -> Broadcast

    if (HAL_FDCAN_ConfigFilter(fdcanHandle, &filterConfig) != HAL_OK)
    {
        Error_Handler();
    }

    txHeader2.Identifier = 0;
    txHeader2.IdType = FDCAN_STANDARD_ID;
    txHeader2.TxFrameType = FDCAN_DATA_FRAME;
    txHeader2.DataLength = FDCAN_DLC_BYTES_8;
    txHeader2.ErrorStateIndicator = FDCAN_ESI_PASSIVE;
    txHeader2.BitRateSwitch = FDCAN_BRS_OFF;
    txHeader2.FDFormat = FDCAN_CLASSIC_CAN;
    txHeader2.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    txHeader2.MessageMarker = 0;

    /* Configure all filters */
    // if (HAL_FDCAN_ConfigGlobalFilter(fdcanHandle, FDCAN_REJECT, FDCAN_REJECT, FDCAN_FILTER_REMOTE, FDCAN_FILTER_REMOTE) != HAL_OK)
    // {
    //     Error_Handler();
    // }

    // Start in normal mode
    CAN2_Mode = CAN_MODE_NORMAL;
    // CAN2_Mode = CAN_MODE_CHARGING;

    // Start CAN2
    if (HAL_FDCAN_Start(fdcanHandle) != HAL_OK) {
        Error_Handler();
    }

    // Activate notifications for new data
    if (HAL_FDCAN_ActivateNotification(fdcanHandle, FDCAN_IT_RX_FIFO1_NEW_MESSAGE, 0) != HAL_OK) {
        Error_Handler();
    }

}

static HAL_StatusTypeDef FDCAN_AddToTxFifoQ(
    FDCAN_HandleTypeDef* hfdcan,
    const FDCAN_TxHeaderTypeDef *pTxHeader,
    const uint8_t* txData)
{
    uint32_t timeout = 10000;

    while (HAL_FDCAN_GetTxFifoFreeLevel(hfdcan) == 0)
    {
        if (--timeout == 0)
        {
            return HAL_TIMEOUT;
        }
    }

    return HAL_FDCAN_AddMessageToTxFifoQ(hfdcan, pTxHeader, txData);
}


void FDCAN_SendCellData(
        FDCAN_HandleTypeDef* hfdcan,
        uint16_t minV,
        uint16_t maxV,
        int16_t minT,
        int16_t maxT
    )
{
	FDCAN_TxHeaderTypeDef txHeader = {0};
	// Package message
    uint8_t data[8];

	packU16(minV, data + 0);
    packU16(maxV, data + 2);
    packS16(minT, data + 4);
    packS16(maxT, data + 6);

    // Set header
    txHeader.Identifier = CAN_CELL_DATA_MSG_ID;
    txHeader.IdType = FDCAN_STANDARD_ID;
    txHeader.DataLength = FDCAN_DLC_BYTES_8;
    txHeader.FDFormat = FDCAN_CLASSIC_CAN;

    if (FDCAN_AddToTxFifoQ(hfdcan, &txHeader, data) != HAL_OK) {
        Error_Handler();
    }
}

void FDCAN_SendPackData(
        FDCAN_HandleTypeDef* hfdcan,
        uint32_t pack_voltage
    )
{
	FDCAN_TxHeaderTypeDef txHeader = {0};
	uint8_t data[8];

	data[0] = (uint8_t)(pack_voltage >> 24);
    data[1] = (uint8_t)(pack_voltage >> 16);
    data[2] = (uint8_t)(pack_voltage >> 8);
    data[3] = (uint8_t)(pack_voltage);

    txHeader.Identifier = CAN_PACK_DATA_MSG_ID;
    txHeader.IdType = FDCAN_STANDARD_ID;
    txHeader.DataLength = FDCAN_DLC_BYTES_4;

    if (FDCAN_AddToTxFifoQ(hfdcan, &txHeader, data) != HAL_OK) {
        Error_Handler();
    }
}

void CAN_Logging(FDCAN_HandleTypeDef* hfdcan, uint16_t max_voltages[TOTAL_IC], uint16_t min_voltages[TOTAL_IC], uint16_t max_temps[TOTAL_IC], uint16_t min_temps[TOTAL_IC], uint32_t packVoltage)
{
    // Send pack votage
    FDCAN_SendPackData(hfdcan, packVoltage);

	for (uint8_t i = 0; i < TOTAL_IC; i++) {
    	// Send values
    	float ntcMin = ntc_to_temp((float)max_temps[i]);
		float ntcMax = ntc_to_temp((float)min_temps[i]);
    	//FDCAN_SendCellData(hfdcan, min_voltages[i], max_voltages[i], ntcMin, ntcMax);
    }
}

void FDCAN_SendFault(
        FDCAN_HandleTypeDef* hfdcan,
        FDCAN_TxHeaderTypeDef* hTxHeader,
        uint8_t bitmask,
		uint8_t *fault_data
    )
{
    /* Fault codes
     * Undervoltage - 	1
     * Overvoltage 	- 	2
     * Undertemp	-	4
     * Overtemp		-	8
     */
	uint8_t data[4];
	// Package message
    data[0] = bitmask;
    data[1] = fault_data[0];
    data[2] = fault_data[1];
    data[3] = fault_data[2];

    // Set header
    hTxHeader->Identifier = CAN_FAULT_MSG_ID;
    hTxHeader->IdType = FDCAN_STANDARD_ID;
    hTxHeader->DataLength = FDCAN_DLC_BYTES_4;
    hTxHeader->FDFormat = FDCAN_CLASSIC_CAN;

    if (FDCAN_AddToTxFifoQ(hfdcan, hTxHeader, data) != HAL_OK) {
            Error_Handler();
    }
}

void FDCAN_SendChargerMessage(uint16_t maxVoltage, uint16_t maxCurrent, uint8_t enable)
{
    // Setup TX CAN ID
    txHeader2.Identifier = ELCON_COMMAND_ID;
    txHeader2.IdType = FDCAN_EXTENDED_ID;
    txHeader2.DataLength = FDCAN_DLC_BYTES_8;

    // Buffer message
    txData2[0] = (uint8_t)(maxVoltage >> 8);
    txData2[1] = (uint8_t)maxVoltage;
    txData2[2] = (uint8_t)(maxCurrent >> 8);
    txData2[3] = (uint8_t)maxCurrent;
    txData2[4] = enable; // Enable

    // Queue TX
    if (HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan2, &txHeader2, txData2) != HAL_OK) {
        Error_Handler();
    }
}

void FDCAN_StartCharging()
{
    // Reconfigure baud rate to 500Kbps
    hfdcan2.Init.NominalPrescaler = 50;

    HAL_FDCAN_Stop(&hfdcan2);
    HAL_FDCAN_Init(&hfdcan2);
    FDCAN2_Init(&hfdcan2);
    // Send enable message
    FDCAN_SendChargerMessage(ELCON_Voltage, ELCON_Current, 0U);

    // Enable 1s timer with charger callback
    HAL_TIM_Base_Start_IT(&htim6);

    CAN2_Mode = CAN_MODE_CHARGING;
}

void FDCAN_StopCharging()
{
    // Stop 1s timer
    HAL_TIM_Base_Stop_IT(&htim6);

    // Send message to disable
    FDCAN_SendChargerMessage(0, 0, 1);

    // Reconfigure baud rate to 1Mbps
    hfdcan2.Init.NominalPrescaler = 25;

    HAL_FDCAN_Stop(&hfdcan2);
    HAL_FDCAN_Init(&hfdcan2);
    HAL_FDCAN_Start(&hfdcan2);

    CAN2_Mode = CAN_MODE_NORMAL;
}

void CAN_Charging(bool *fault_state) {
	//CAN2_StartCharging = true;
	if (HAL_FDCAN_GetTxFifoFreeLevel(&hfdcan2) > 0) {
		if (CAN2_Mode == CAN_MODE_NORMAL) {
	    // If start charging
			if (CAN2_StartCharging) {
				CAN2_StartCharging = false;
	            FDCAN_StartCharging();
	        }

			FDCAN_StartCharging();
	    }
	    else if (CAN2_Mode == CAN_MODE_CHARGING) {
	    // Stop charging if faulted
	    	/*if (*fault_state) {
	    		FDCAN_StopCharging();
	        }*/
	    }
	 }

}

// BALANCING FUNCTIONS

uint8_t balance_cells(int8_t total_ic, cell_asic *ic, uint16_t target_voltage)
{
    uint8_t done = 1;

    // clear all the dcc bits
    for (int ic_idx = 0; ic_idx < total_ic; ic_idx++) {
    	ic[ic_idx].config.tx_data[4] = 0x00;		// dcc for cells 1-8
    	ic[ic_idx].config.tx_data[5] = 0x00;		// dcc for cells 9-12 (& dcto)
    	ic[ic_idx].configb.tx_data[0] = 0x00;	// dcc for cells 13-16
    	ic[ic_idx].configb.tx_data[1] = 0x00;	// dcc for cells 17-18

    	ic[ic_idx].config.tx_data[0] |= (1 << 2); // enable refon
    }

    for (int ic_idx = 0; ic_idx < total_ic; ic_idx++)
    {
        for (int cell = 0; cell < CELLS_PER_IC; cell++)
        {
        	if (ic[ic_idx].cells.c_codes[cell] > target_voltage) {
            	if (cell < 8)
            	{
            		ic[ic_idx].config.tx_data[4] |= (1 << cell);
            	}
            	else if (cell < 12)
            	{
            		ic[ic_idx].config.tx_data[5] |= (1 << (cell - 8));
            	}
            	else if (cell < 16) // cells 13–15 only
            	{
            		ic[ic_idx].configb.tx_data[0] |= (1 << (cell - 12));
            	}
            	done = 0;
            }
         }
    }

    wakeup_idle(TOTAL_IC);
    HAL_Delay(2);

    LTC6813_wrcfg(total_ic, ic);
    HAL_Delay(2);

    LTC6813_wrcfgb(total_ic, ic);
    HAL_Delay(2);

    HAL_Delay(200);

    return done;
}

// MULTIPLEXING FUNCTIONS (FOR CMT25)
bool select_temp(uint8_t total_ic, cell_asic *ic, uint8_t channel)
{
	const uint8_t mux_addr_1 = 0x98;
    const uint8_t mux_addr_2 = 0x9A;

    uint8_t mask1 = 0;
    uint8_t mask2 = 0;

    // Channel → mask
    if (channel < 8) {
        mask1 = 1 << channel;
    }
    else if (channel < 14) {
        mask2 = 1 << (channel - 8);
    }

    // Helper values (same for all ICs)
    uint8_t ICOM0 = 6, ICOM1 = 0, ICOM2 = 7;
    uint8_t FCOM0 = 8, FCOM1 = 9, FCOM2 = 0;

    wakeup_sleep(total_ic);

    for (uint8_t current_ic = 0; current_ic < total_ic; current_ic++)
    {
    	memset(ic[current_ic].com.tx_data, 0, 6);

        uint8_t d0 = mux_addr_1;
        uint8_t d1 = mask1;
        uint8_t d2 = 0x00;

        ic[current_ic].com.tx_data[0] = (ICOM0 << 4) | (d0 >> 4);
        ic[current_ic].com.tx_data[1] = (d0 << 4) | FCOM0;

        ic[current_ic].com.tx_data[2] = (ICOM1 << 4) | (d1 >> 4);
        ic[current_ic].com.tx_data[3] = (d1 << 4) | FCOM1;

        ic[current_ic].com.tx_data[4] = (ICOM2 << 4) | (d2 >> 4);
        ic[current_ic].com.tx_data[5] = (d2 << 4) | FCOM2;
    }

    wakeup_idle(total_ic);

    LTC6813_wrcomm(total_ic, ic);
    LTC6813_stcomm(total_ic);
    delay_m(10);

    for (uint8_t current_ic = 0; current_ic < total_ic; current_ic++)
    {
    	memset(ic[current_ic].com.tx_data, 0, 6);

        uint8_t d0 = mux_addr_2;
        uint8_t d1 = mask2;
        uint8_t d2 = 0x00;

        ic[current_ic].com.tx_data[0] = (ICOM0 << 4) | (d0 >> 4);
        ic[current_ic].com.tx_data[1] = (d0 << 4) | FCOM0;

        ic[current_ic].com.tx_data[2] = (ICOM1 << 4) | (d1 >> 4);
        ic[current_ic].com.tx_data[3] = (d1 << 4) | FCOM1;

        ic[current_ic].com.tx_data[4] = (ICOM2 << 4) | (d2 >> 4);
        ic[current_ic].com.tx_data[5] = (d2 << 4) | FCOM2;
    }

    wakeup_idle(total_ic);

    LTC6813_wrcomm(total_ic, ic);
    LTC6813_stcomm(total_ic);
    delay_m(10);

    return true;
}

void read_temps_25(uint8_t total_ic, cell_asic *ic, uint16_t temps[TOTAL_IC][TEMPS_PER_IC])
{
    for (int ch = 0; ch < TEMPS_PER_IC; ch++)
    {
        // Select mux channel (all ICs)
    	wakeup_idle(total_ic);

    	select_temp(total_ic, ic, ch);

        HAL_Delay(5);

        wakeup_idle(total_ic);
        LTC6813_adax(MD_422HZ_1KHZ, 0);

        LTC6813_pollAdc();
        LTC6813_rdaux(1, total_ic, ic);

        // Store for ALL ICs
        for (int ic_idx = 0; ic_idx < total_ic; ic_idx++)
        {
            temps[ic_idx][ch] = ic[ic_idx].aux.a_codes[0];
        }
    }
}

void print_temps_25(uint16_t temps[TOTAL_IC][TEMPS_PER_IC]) {
	for(int ic_idx = 0; ic_idx < TOTAL_IC; ic_idx++) {
		uart_print("IC ");
		uart_print_uint(ic_idx);
		uart_print(": ");

		for (int cell = 0; cell < CELLS_PER_IC; cell++) {
			uart_print_uint(temps[ic_idx][cell]);
			uart_print(" ");
		}

		uart_print("\r\n");
	}
}



