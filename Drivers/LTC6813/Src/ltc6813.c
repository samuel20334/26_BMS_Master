/*
 * ltc6813.c
 *
 *  Created on: 13 Mar 2026
 *      Author: smpet
 */

#include "ltc6813.h"

extern SPI_HandleTypeDef hspi1;
extern SPI_HandleTypeDef hspi1;
extern UART_HandleTypeDef huart1;
extern TIM_HandleTypeDef htim2;

const uint16_t crc15Table[256] = {0x0,0xc599, 0xceab, 0xb32, 0xd8cf, 0x1d56, 0x1664, 0xd3fd, 0xf407, 0x319e, 0x3aac,  // precomputed CRC15 Table
                                0xff35, 0x2cc8, 0xe951, 0xe263, 0x27fa, 0xad97, 0x680e, 0x633c, 0xa6a5, 0x7558, 0xb0c1,
                                0xbbf3, 0x7e6a, 0x5990, 0x9c09, 0x973b, 0x52a2, 0x815f, 0x44c6, 0x4ff4, 0x8a6d, 0x5b2e,
                                0x9eb7, 0x9585, 0x501c, 0x83e1, 0x4678, 0x4d4a, 0x88d3, 0xaf29, 0x6ab0, 0x6182, 0xa41b,
                                0x77e6, 0xb27f, 0xb94d, 0x7cd4, 0xf6b9, 0x3320, 0x3812, 0xfd8b, 0x2e76, 0xebef, 0xe0dd,
                                0x2544, 0x2be, 0xc727, 0xcc15, 0x98c, 0xda71, 0x1fe8, 0x14da, 0xd143, 0xf3c5, 0x365c,
                                0x3d6e, 0xf8f7,0x2b0a, 0xee93, 0xe5a1, 0x2038, 0x7c2, 0xc25b, 0xc969, 0xcf0, 0xdf0d,
                                0x1a94, 0x11a6, 0xd43f, 0x5e52, 0x9bcb, 0x90f9, 0x5560, 0x869d, 0x4304, 0x4836, 0x8daf,
                                0xaa55, 0x6fcc, 0x64fe, 0xa167, 0x729a, 0xb703, 0xbc31, 0x79a8, 0xa8eb, 0x6d72, 0x6640,
                                0xa3d9, 0x7024, 0xb5bd, 0xbe8f, 0x7b16, 0x5cec, 0x9975, 0x9247, 0x57de, 0x8423, 0x41ba,
                                0x4a88, 0x8f11, 0x57c, 0xc0e5, 0xcbd7, 0xe4e, 0xddb3, 0x182a, 0x1318, 0xd681, 0xf17b,
                                0x34e2, 0x3fd0, 0xfa49, 0x29b4, 0xec2d, 0xe71f, 0x2286, 0xa213, 0x678a, 0x6cb8, 0xa921,
                                0x7adc, 0xbf45, 0xb477, 0x71ee, 0x5614, 0x938d, 0x98bf, 0x5d26, 0x8edb, 0x4b42, 0x4070,
                                0x85e9, 0xf84, 0xca1d, 0xc12f, 0x4b6, 0xd74b, 0x12d2, 0x19e0, 0xdc79, 0xfb83, 0x3e1a, 0x3528,
                                0xf0b1, 0x234c, 0xe6d5, 0xede7, 0x287e, 0xf93d, 0x3ca4, 0x3796, 0xf20f, 0x21f2, 0xe46b, 0xef59,
                                0x2ac0, 0xd3a, 0xc8a3, 0xc391, 0x608, 0xd5f5, 0x106c, 0x1b5e, 0xdec7, 0x54aa, 0x9133, 0x9a01,
                                0x5f98, 0x8c65, 0x49fc, 0x42ce, 0x8757, 0xa0ad, 0x6534, 0x6e06, 0xab9f, 0x7862, 0xbdfb, 0xb6c9,
                                0x7350, 0x51d6, 0x944f, 0x9f7d, 0x5ae4, 0x8919, 0x4c80, 0x47b2, 0x822b, 0xa5d1, 0x6048, 0x6b7a,
                                0xaee3, 0x7d1e, 0xb887, 0xb3b5, 0x762c, 0xfc41, 0x39d8, 0x32ea, 0xf773, 0x248e, 0xe117, 0xea25,
                                0x2fbc, 0x846, 0xcddf, 0xc6ed, 0x374, 0xd089, 0x1510, 0x1e22, 0xdbbb, 0xaf8, 0xcf61, 0xc453,
                                0x1ca, 0xd237, 0x17ae, 0x1c9c, 0xd905, 0xfeff, 0x3b66, 0x3054, 0xf5cd, 0x2630, 0xe3a9, 0xe89b,
                                0x2d02, 0xa76f, 0x62f6, 0x69c4, 0xac5d, 0x7fa0, 0xba39, 0xb10b, 0x7492, 0x5368, 0x96f1, 0x9dc3,
                                0x585a, 0x8ba7, 0x4e3e, 0x450c, 0x8095
                               };

void cs_low(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
  output_low(GPIOx, GPIO_Pin);
}

void cs_high(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
  output_high(GPIOx, GPIO_Pin);
}

void delay_u(uint16_t micro)
{
  uint32_t start = __HAL_TIM_GET_COUNTER(&htim2);
  uint32_t duration = micro;
  while ((__HAL_TIM_GET_COUNTER(&htim2) - start) < duration);
}

void delay_m(uint16_t milli)
{
  HAL_Delay(milli);
}

/*
Writes an array of bytes out of the SPI port
*/
void spi_write_array(SPI_HandleTypeDef *spi,
					 uint16_t len, // Option: Number of bytes to be written on the SPI port
                     uint8_t *data //Array of bytes to be written on the SPI port
                    )
{
	HAL_StatusTypeDef status = HAL_SPI_Transmit(spi, data, len, HAL_MAX_DELAY);

	if (status != HAL_OK)
	{
	    Error_Handler();
	}
}

/*
 Writes and read a set number of bytes using the SPI port.

*/

void spi_write_read(SPI_HandleTypeDef *spi,
					uint8_t *tx_data,//array of data to be written on SPI port
					uint16_t tx_len,
                    uint8_t *rx_data,//Input: array that will store the data read by the SPI port
                    uint16_t rx_len // length of tx and rx arrays (must be the same)
                   )
{
	HAL_SPI_Transmit(spi, tx_data, tx_len, 1000);
	HAL_SPI_Receive(spi, rx_data, rx_len, 1000);

}


HAL_StatusTypeDef spi_read_byte(SPI_HandleTypeDef *spi, uint8_t *buf)
{
    uint8_t dummy_byte = 0x00;
	return HAL_SPI_TransmitReceive(spi, &dummy_byte, buf, 1, HAL_MAX_DELAY);
}

HAL_StatusTypeDef spi_read_array(SPI_HandleTypeDef *spi, uint8_t len, uint8_t *buf) {
    return HAL_SPI_Receive(spi, buf, len, HAL_MAX_DELAY);
}


void uart_print(char *str)
{
    HAL_UART_Transmit(&huart1, (uint8_t*)str, strlen(str), HAL_MAX_DELAY);
}

void uart_print_hex(uint8_t val)
{
    char buf[5];
    snprintf(buf, sizeof(buf), "%02X ", val);
    uart_print(buf);
}

void uart_print_dec(uint16_t val)
{
    char buf[8];
    snprintf(buf, sizeof(buf), "%d ", val);
    uart_print(buf);
}

void uart_print_uint(uint32_t val)
{
    char buf[12]; // Enough for 32-bit uint + null
    snprintf(buf, sizeof(buf), "%lu", val);
    uart_print(buf);
}

/* Wake isoSPI up from IDlE state and enters the READY state */
void wakeup_idle(uint8_t total_ic) //Number of ICs in the system
{
	uint8_t buf[16];
	for (int i =0; i<total_ic; i++)
	{
	   cs_low(CS_PORT, CS_PIN);
	   spi_read_byte(&hspi1, buf);//Guarantees the isoSPI will be in ready mode
	   cs_high(CS_PORT, CS_PIN);
	}
}

/* Generic wakeup command to wake the LTC681x from sleep state */
void wakeup_sleep(uint8_t total_ic) //Number of ICs in the system
{
	for (int i =0; i<total_ic; i++)
	{
	   cs_low(CS_PORT, CS_PIN);
	   delay_m(1); // Guarantees the LTC681x will be in standby
	   cs_high(CS_PORT, CS_PIN);
	   delay_m(1);
	}
}

/* Generic function to write 68xx commands. Function calculates PEC for tx_cmd data. */
void cmd_68(uint8_t tx_cmd[2]) //The command to be transmitted
{
	uint8_t cmd[4];
	uint16_t cmd_pec;

	cmd[0] = tx_cmd[0];
	cmd[1] =  tx_cmd[1];
	cmd_pec = pec15_calc(2, cmd);
	cmd[2] = (uint8_t)(cmd_pec >> 8);
	cmd[3] = (uint8_t)(cmd_pec);

	cs_low(CS_PORT, CS_PIN);
	spi_write_array(&hspi1, 4, cmd);
	cs_high(CS_PORT, CS_PIN);
}

/*
Generic function to write 68xx commands and write payload data.
Function calculates PEC for tx_cmd data and the data to be transmitted.
 */
void write_68(uint8_t total_ic, //Number of ICs to be written to
			  uint8_t tx_cmd[2], //The command to be transmitted
			  uint8_t* data // Payload Data
			  )
{
	const uint8_t BYTES_IN_REG = 6;
	const uint8_t CMD_LEN = 4+(8*total_ic);
	uint8_t cmd[CMD_LEN];
	uint16_t data_pec;
	uint16_t cmd_pec;
	uint8_t cmd_index;

	cmd[0] = tx_cmd[0];
	cmd[1] = tx_cmd[1];
	cmd_pec = pec15_calc(2, cmd);
	cmd[2] = (uint8_t)(cmd_pec >> 8);
	cmd[3] = (uint8_t)(cmd_pec);

	cmd_index = 4;
	for (uint8_t current_ic = total_ic; current_ic > 0; current_ic--)               // Executes for each LTC681x, this loops starts with the last IC on the stack.
    {	                                                                            //The first configuration written is received by the last IC in the daisy chain
		for (uint8_t current_byte = 0; current_byte < BYTES_IN_REG; current_byte++)
		{
			cmd[cmd_index] = data[((current_ic-1)*6)+current_byte];
			cmd_index = cmd_index + 1;
		}

		data_pec = (uint16_t)pec15_calc(BYTES_IN_REG, &data[(current_ic-1)*6]);    // Calculating the PEC for each ICs configuration register data
		cmd[cmd_index] = (uint8_t)(data_pec >> 8);
		cmd[cmd_index + 1] = (uint8_t)data_pec;
		cmd_index = cmd_index + 2;
	}

	cs_low(CS_PORT, CS_PIN);
	spi_write_array(&hspi1, CMD_LEN, cmd);
	cs_high(CS_PORT, CS_PIN);
}

/* Generic function to write 68xx commands and read data. Function calculated PEC for tx_cmd data */
int8_t read_68( uint8_t total_ic, // Number of ICs in the system
				uint8_t tx_cmd[2], // The command to be transmitted
				uint8_t *rx_data // Data to be read
				)
{
	const uint8_t BYTES_IN_REG = 8;
	uint8_t cmd[4];
	uint8_t data[256];
	int8_t pec_error = 0;
	uint16_t cmd_pec;
	uint16_t data_pec;
	uint16_t received_pec;

	cmd[0] = tx_cmd[0];
	cmd[1] = tx_cmd[1];
	cmd_pec = pec15_calc(2, cmd);
	cmd[2] = (uint8_t)(cmd_pec >> 8);
	cmd[3] = (uint8_t)(cmd_pec);

	cs_low(CS_PORT, CS_PIN);
	spi_write_read(&hspi1, cmd, 4, data, BYTES_IN_REG*total_ic);       //Transmits the command and reads the configuration data of all ICs on the daisy chain into rx_data[] array
	cs_high(CS_PORT, CS_PIN);

	for (uint8_t current_ic = 0; current_ic < total_ic; current_ic++) //Executes for each LTC681x in the daisy chain and packs the data
	{																//into the rx_data array as well as check the received data for any bit errors
		for (uint8_t current_byte = 0; current_byte < 6; current_byte++)
		{
			rx_data[(current_ic*8)+current_byte] = data[current_byte + (current_ic*BYTES_IN_REG)];
		}
		received_pec = (rx_data[(current_ic*8)+6]<<8) + rx_data[(current_ic*8)+7];
		data_pec = pec15_calc(6, &rx_data[current_ic*8]);

		if (received_pec != data_pec)
				{
				  pec_error = -1;
				}
	}

	return(pec_error);
}

/* Calculates  and returns the CRC15 */
uint16_t pec15_calc(uint8_t len, //Number of bytes that will be used to calculate a PEC
                    uint8_t *data //Array of data that will be used to calculate  a PEC
                   )
{
	uint16_t remainder,addr;
	remainder = 16;//initialize the PEC

	for (uint8_t i = 0; i<len; i++) // loops for each byte in data array
	{
		addr = ((remainder>>7)^data[i])&0xff;//calculate PEC table address
	    remainder = (remainder<<8)^crc15Table[addr];
	}

	return(remainder*2);//The CRC15 has a 0 in the LSB so the remainder must be multiplied by 2
}
/* Helper function to initialize register limits. */
void LTC6813_init_reg_limits(uint8_t total_ic, //Number of ICs in the system
							 cell_asic *ic // A two dimensional array that will store the data
							 )
{
    for(uint8_t cic=0; cic<total_ic; cic++)
    {
        ic[cic].ic_reg.cell_channels=18;
        ic[cic].ic_reg.stat_channels=4;
        ic[cic].ic_reg.aux_channels=9;
        ic[cic].ic_reg.num_cv_reg=6;
        ic[cic].ic_reg.num_gpio_reg=4;
        ic[cic].ic_reg.num_stat_reg=2;
    }
}

 /*
This command will write the configuration registers of the LTC6813-1s
connected in a daisy chain stack. The configuration is written in descending
order so the last device's configuration is written first.
*/
void LTC6813_wrcfg(uint8_t total_ic, //The number of ICs being written to
                     cell_asic *ic //A two dimensional array of the configuration data that will be written
                    )
{
	uint8_t cmd[2] = {0x00 , 0x01} ;
	uint8_t write_buffer[256];
	uint8_t write_count = 0;
	uint8_t c_ic = 0;

	for (uint8_t current_ic = 0; current_ic<total_ic; current_ic++)
	{
		if (ic[0].isospi_reverse == false)
		{
			c_ic = current_ic;
		}
		else
		{
			c_ic = total_ic - current_ic - 1;
		}

		for (uint8_t data = 0; data<6; data++)
		{
			write_buffer[write_count] = ic[c_ic].config.tx_data[data];
			write_count++;
		}
	}
	write_68(total_ic, cmd, write_buffer);
}

/*
This command will write the configuration b registers of the LTC6813-1s
connected in a daisy chain stack. The configuration is written in descending
order so the last device's configuration is written first.
*/
void LTC6813_wrcfgb(uint8_t total_ic, //The number of ICs being written to
                    cell_asic *ic //A two dimensional array of the configuration data that will be written
                   )
{
	uint8_t cmd[2] = {0x00 , 0x24} ;
	uint8_t write_buffer[256];
	uint8_t write_count = 0;
	uint8_t c_ic = 0;

	for (uint8_t current_ic = 0; current_ic<total_ic; current_ic++)
	{
		if (ic[0].isospi_reverse == false)
		{
			c_ic = current_ic;
		}
		else
		{
			c_ic = total_ic - current_ic - 1;
		}

		for (uint8_t data = 0; data<6; data++)
		{
			write_buffer[write_count] = ic[c_ic].configb.tx_data[data];
			write_count++;
		}
	}
	write_68(total_ic, cmd, write_buffer);
}

/* Reads configuration registers of a LTC6813 daisy chain */
int8_t LTC6813_rdcfg(uint8_t total_ic, //Number of ICs in the system
				   cell_asic *ic //A two dimensional array that the function stores the read configuration data.
				  )
{
	uint8_t cmd[2]= {0x00 , 0x02};
	uint8_t read_buffer[256];
	int8_t pec_error = 0;
	uint16_t data_pec;
	uint16_t calc_pec;
	uint8_t c_ic = 0;

	pec_error = read_68(total_ic, cmd, read_buffer);

	for (uint8_t current_ic = 0; current_ic<total_ic; current_ic++)
	{
		if (ic[0].isospi_reverse == false)
		{
			c_ic = current_ic;
		}
		else
		{
			c_ic = total_ic - current_ic - 1;
		}

		for (int byte=0; byte<8; byte++)
		{
			ic[c_ic].config.rx_data[byte] = read_buffer[byte+(8*current_ic)];
		}

		calc_pec = pec15_calc(6,&read_buffer[8*current_ic]);
		data_pec = read_buffer[7+(8*current_ic)] | (read_buffer[6+(8*current_ic)]<<8);
		if (calc_pec != data_pec )
		{
			ic[c_ic].config.rx_pec_match = 1;
		}
		else ic[c_ic].config.rx_pec_match = 0;
	}
	LTC6813_check_pec(total_ic,CFGR,ic);

	return(pec_error);
}

/* Reads configuration b registers of a LTC6813 daisy chain */
int8_t LTC6813_rdcfgb(uint8_t total_ic, //Number of ICs in the system
                   cell_asic *ic //A two dimensional array that the function stores the read configuration data.
                  )
{
	uint8_t cmd[2]= {0x00 , 0x26};
	uint8_t read_buffer[256];
	int8_t pec_error = 0;
	uint16_t data_pec;
	uint16_t calc_pec;
	uint8_t c_ic = 0;

	pec_error = read_68(total_ic, cmd, read_buffer);

	for (uint8_t current_ic = 0; current_ic<total_ic; current_ic++)
	{
		if (ic[0].isospi_reverse == false)
		{
			c_ic = current_ic;
		}
		else
		{
			c_ic = total_ic - current_ic - 1;
		}

		for (int byte=0; byte<8; byte++)
		{
			ic[c_ic].configb.rx_data[byte] = read_buffer[byte+(8*current_ic)];
		}

		calc_pec = pec15_calc(6,&read_buffer[8*current_ic]);
		data_pec = read_buffer[7+(8*current_ic)] | (read_buffer[6+(8*current_ic)]<<8);
		if (calc_pec != data_pec )
		{
			ic[c_ic].configb.rx_pec_match = 1;
		}
		else ic[c_ic].configb.rx_pec_match = 0;
	}
	LTC6813_check_pec(total_ic,CFGRB,ic);

	return(pec_error);
}

/* Starts cell voltage conversion */
void LTC6813_adcv(uint8_t MD, //ADC Mode
				  uint8_t DCP, //Discharge Permit
				  uint8_t CH //Cell Channels to be measured
				 )
{
	uint8_t cmd[2];
	uint8_t md_bits;

	md_bits = (MD & 0x02) >> 1;
	cmd[0] = md_bits + 0x02;
	md_bits = (MD & 0x01) << 7;
	cmd[1] =  md_bits + 0x60 + (DCP<<4) + CH;

	cmd_68(cmd);
}

/* Start a GPIO and Vref2 Conversion */
void LTC6813_adax(uint8_t MD, //ADC Mode
				  uint8_t CHG //GPIO Channels to be measured)
                 )
{
	uint8_t cmd[4];
	uint8_t md_bits;

	md_bits = (MD & 0x02) >> 1;
	cmd[0] = md_bits + 0x04;
	md_bits = (MD & 0x01) << 7;
	cmd[1] = md_bits + 0x60 + CHG ;

	cmd_68(cmd);
}

/* Start a Status ADC Conversion */
void LTC6813_adstat(uint8_t MD, //ADC Mode
					uint8_t CHST //Stat Channels to be measured
)
{
	uint8_t cmd[4];
	uint8_t md_bits;

	md_bits = (MD & 0x02) >> 1;
	cmd[0] = md_bits + 0x04;
	md_bits = (MD & 0x01) << 7;
	cmd[1] = md_bits + 0x68 + CHST ;

	cmd_68(cmd);
}

/* Starts cell voltage and GPIO 1&2 conversion */
void LTC6813_adcvax(uint8_t MD, //ADC Mode
					uint8_t DCP //Discharge Permit
					)
{
	uint8_t cmd[2];
	uint8_t md_bits;

	md_bits = (MD & 0x02) >> 1;
	cmd[0] = md_bits | 0x04;
	md_bits = (MD & 0x01) << 7;
	cmd[1] =  md_bits | (((DCP&0x01)<<4) + 0x6F);

	cmd_68(cmd);
}

/* Starts cell voltage and SOC conversion */
void LTC6813_adcvsc( uint8_t MD, //ADC Mode
                     uint8_t DCP //Discharge Permit
                   )
{
	uint8_t cmd[2];
	uint8_t md_bits;

	md_bits = (MD & 0x02) >> 1;
	cmd[0] = md_bits | 0x04;
	md_bits = (MD & 0x01) << 7;
	cmd[1] =  md_bits | 0x60 | (DCP<<4) | 0x07;

	cmd_68(cmd);
}

/*  Reads and parses the LTC6813 cell voltage registers */
uint8_t LTC6813_rdcv(uint8_t reg, // Controls which cell voltage register is read back.
                     uint8_t total_ic, // The number of ICs in the system
                     cell_asic *ic // Array of the parsed cell codes
                    )
{
	const uint8_t REG_LEN = 8;  // 6 data + 2 PEC
	uint8_t rx_buf[REG_LEN * total_ic];
	uint8_t pec_error = 0;

	for (uint8_t reg = 1; reg <= 6; reg++)
	{
	    memset(rx_buf, 0xFF, sizeof(rx_buf));  // Prepare dummy TX buffer
	    LTC6813_rdcv_reg(reg, total_ic, rx_buf);

	    for (uint8_t ic_idx = 0; ic_idx < total_ic; ic_idx++)
	    {
	        uint8_t mapped_ic = ic[0].isospi_reverse ? (total_ic - ic_idx - 1) : ic_idx;
	        uint8_t *data_ptr = &rx_buf[REG_LEN * ic_idx];

	        pec_error += parse_cells(ic_idx, reg, data_ptr,
	                                 &ic[mapped_ic].cells.c_codes[0],
	                                 &ic[mapped_ic].cells.pec_match[0]);
	    }
	}

	LTC6813_check_pec(total_ic, CELL, ic);
	return pec_error;
}

/*
The function is used
to read the  parsed GPIO codes of the LTC6813. This function will send the requested
read commands parse the data and store the gpio voltages in aux_codes variable
*/
int8_t LTC6813_rdaux(uint8_t reg, //Determines which GPIO voltage register is read back.
				     uint8_t total_ic,//The number of ICs in the system
				     cell_asic *ic//A two dimensional array of the gpio voltage codes.
				    )
{
	uint8_t *data;
	int8_t pec_error = 0;
	uint8_t c_ic =0;
	data = (uint8_t *) malloc((NUM_RX_BYT*total_ic)*sizeof(uint8_t));

	if (reg == 0)
	{
		for (uint8_t gpio_reg = 1; gpio_reg<ic[0].ic_reg.num_gpio_reg+1; gpio_reg++) //Executes once for each of the LTC6813 aux voltage registers
		{
			LTC6813_rdaux_reg(gpio_reg, total_ic,data);                 //Reads the raw auxiliary register data into the data[] array
			for (int current_ic = 0; current_ic<total_ic; current_ic++)
			{
				if (ic[0].isospi_reverse == false)
				{
				  c_ic = current_ic;
				}
				else
				{
				  c_ic = total_ic - current_ic - 1;
				}
				pec_error = parse_cells(current_ic, gpio_reg,
				                        &data[current_ic * NUM_RX_BYT],
				                        &ic[c_ic].aux.a_codes[0],
				                        &ic[c_ic].aux.pec_match[0]);
			}
		}
	}
	else
	{
		LTC6813_rdaux_reg(reg, total_ic, data);

		for (int current_ic = 0; current_ic<total_ic; current_ic++)
		{
			if (ic[0].isospi_reverse == false)
			{
			c_ic = current_ic;
			}
			else
			{
			c_ic = total_ic - current_ic - 1;
			}
			pec_error = parse_cells(current_ic, reg,
			                        &data[current_ic * NUM_RX_BYT],
			                        &ic[c_ic].aux.a_codes[0],
			                        &ic[c_ic].aux.pec_match[0]);
		}
	}
	LTC6813_check_pec(total_ic,AUX,ic);
	free(data);

	return (pec_error);
}

/*
Reads and parses the LTC6813 stat registers.
The function is used
to read the  parsed stat codes of the LTC6813. This function will send the requested
read commands parse the data and store the stat voltages in stat_codes variable
*/
int8_t LTC6813_rdstat(uint8_t reg, //Determines which Stat  register is read back.
                      uint8_t total_ic,//The number of ICs in the system
                      cell_asic *ic //A two dimensional array of the stat codes.
                       )
{
	const uint8_t BYT_IN_REG = 6;
	const uint8_t STAT_IN_REG = 3;
	uint8_t *data;
	uint8_t data_counter = 0;
	int8_t pec_error = 0;
	uint16_t parsed_stat;
	uint16_t received_pec;
	uint16_t data_pec;
	uint8_t c_ic = 0;

	data = (uint8_t *) malloc((12*total_ic)*sizeof(uint8_t));

	if (reg == 0)
	{
		for (uint8_t stat_reg = 1; stat_reg< 3; stat_reg++)                      //Executes once for each of the LTC6813 stat voltage registers
		{
			data_counter = 0;
			LTC6813_rdstat_reg(stat_reg, total_ic,data);                            //Reads the raw status register data into the data[] array

			for (uint8_t current_ic = 0 ; current_ic < total_ic; current_ic++)      // Executes for every LTC6813 in the daisy chain
			{																		// current_ic is used as the IC counter
				if (ic[0].isospi_reverse == false)
				{
					c_ic = current_ic;
				}
				else
				{
					c_ic = total_ic - current_ic - 1;
				}

				if (stat_reg ==1)
				{
					for (uint8_t current_stat = 0; current_stat< STAT_IN_REG; current_stat++) // This loop parses the read back data into Status registers,
					{																		 // it loops once for each of the 3 stat codes in the register
						parsed_stat = data[data_counter] + (data[data_counter+1]<<8);       //Each stat codes is received as two bytes and is combined to create the parsed status code
						ic[c_ic].stat.stat_codes[current_stat] = parsed_stat;
						data_counter=data_counter+2;                                       //Because stat codes are two bytes the data counter
					}
				}
				else if (stat_reg == 2)
				{
					parsed_stat = data[data_counter] + (data[data_counter+1]<<8);          //Each stat is received as two bytes and is combined to create the parsed status code
					data_counter = data_counter +2;
					ic[c_ic].stat.stat_codes[3] = parsed_stat;
					ic[c_ic].stat.flags[0] = data[data_counter++];
					ic[c_ic].stat.flags[1] = data[data_counter++];
					ic[c_ic].stat.flags[2] = data[data_counter++];
					ic[c_ic].stat.mux_fail[0] = (data[data_counter] & 0x02)>>1;
					ic[c_ic].stat.thsd[0] = data[data_counter++] & 0x01;
				}

				received_pec = (data[data_counter]<<8)+ data[data_counter+1];        //The received PEC for the current_ic is transmitted as the 7th and 8th
																					//after the 6 status data bytes
				data_pec = pec15_calc(BYT_IN_REG, &data[current_ic*NUM_RX_BYT]);

				if (received_pec != data_pec)
				{
					pec_error = -1;                         //The pec_error variable is simply set negative if any PEC errors
					ic[c_ic].stat.pec_match[stat_reg-1]=1;  //are detected in the received serial data

				}
				else
				{
					ic[c_ic].stat.pec_match[stat_reg-1]=0;
				}

				data_counter=data_counter+2;    //Because the transmitted PEC code is 2 bytes long the data_counter
											//must be incremented by 2 bytes to point to the next ICs status data
			}
		}
	}
	else
	{
		LTC6813_rdstat_reg(reg, total_ic, data);
		for (int current_ic = 0 ; current_ic < total_ic; current_ic++)            // Executes for every LTC6813 in the daisy chain
		{																		  // current_ic is used as an IC counter
			if (ic[0].isospi_reverse == false)
			{
			c_ic = current_ic;
			}
			else
			{
			c_ic = total_ic - current_ic - 1;
			}
			if (reg ==1)
			{
				for (uint8_t current_stat = 0; current_stat< STAT_IN_REG; current_stat++) // This loop parses the read back data into Status voltages, it
				{																		  // loops once for each of the 3 stat codes in the register

					parsed_stat = data[data_counter] + (data[data_counter+1]<<8);           //Each stat codes is received as two bytes and is combined to
																							// create the parsed stat code

					ic[c_ic].stat.stat_codes[current_stat] = parsed_stat;
					data_counter=data_counter+2;                     //Because stat codes are two bytes the data counter
																	//must increment by two for each parsed stat code
				}
			}
			else if (reg == 2)
			{
				parsed_stat = data[data_counter];	//Each stat codes is received as two bytes and is combined to
				data_counter++;
				parsed_stat |= (data[data_counter] << 8);
				data_counter++;
				ic[c_ic].stat.stat_codes[3] = parsed_stat;
				ic[c_ic].stat.flags[0] = data[data_counter++];
				ic[c_ic].stat.flags[1] = data[data_counter++];
				ic[c_ic].stat.flags[2] = data[data_counter++];
				ic[c_ic].stat.mux_fail[0] = (data[data_counter] & 0x02)>>1;
				ic[c_ic].stat.thsd[0] = data[data_counter++] & 0x01;
			}

			received_pec = (data[data_counter]<<8)+ data[data_counter+1]; //The received PEC for the current_ic is transmitted as the 7th and 8th
																		  //after the 6 status data bytes
			data_pec = pec15_calc(BYT_IN_REG, &data[current_ic*NUM_RX_BYT]);
			if (received_pec != data_pec)
			{
				pec_error = -1;                  //The pec_error variable is simply set negative if any PEC errors
				ic[c_ic].stat.pec_match[reg-1]=1;
			}

			data_counter=data_counter+2;
		}
	}
	LTC6813_check_pec(total_ic,STAT,ic);

	free(data);

	return (pec_error);
}

/* Writes the command and reads the raw cell voltage register data */
void LTC6813_rdcv_reg(uint8_t reg, uint8_t total_ic, uint8_t *data)
{
    const uint8_t REG_LEN = 8; // 6 bytes data + 2 bytes PEC
    uint8_t cmd[4] = {0};
    uint16_t cmd_pec = 0;

    // Set command bytes
    switch(reg)
    {
        case 1: cmd[1] = 0x04; break; // RDCVA
        case 2: cmd[1] = 0x06; break; // RDCVB
        case 3: cmd[1] = 0x08; break; // RDCVC
        case 4: cmd[1] = 0x0A; break; // RDCVD
        case 5: cmd[1] = 0x09; break; // RDCVE
        case 6: cmd[1] = 0x0B; break; // RDCVF
        default: return;
    }

    // Compute PEC
    cmd_pec = pec15_calc(2, cmd);
    cmd[2] = (uint8_t)(cmd_pec >> 8);
    cmd[3] = (uint8_t)(cmd_pec & 0xFF);

    // Pull CS low to start transaction
    cs_low(CS_PORT, CS_PIN);
    HAL_SPI_Transmit(&hspi1, cmd, 4, 100);
    uint8_t dummy_tx[total_ic * REG_LEN];
    memset(dummy_tx, 0xFF, sizeof(dummy_tx));

    HAL_SPI_TransmitReceive(&hspi1, dummy_tx, data, total_ic * REG_LEN, 100);
    cs_high(CS_PORT, CS_PIN);
}

/*
The function reads a single GPIO voltage register and stores the read data
in the *data point as a byte array. This function is rarely used outside of
the LTC681x_rdaux() command.
*/
void LTC6813_rdaux_reg(uint8_t reg, //Determines which GPIO voltage register is read back
                       uint8_t total_ic, //The number of ICs in the system
                       uint8_t *data //Array of the unparsed auxiliary codes
                      )
{
	const uint8_t REG_LEN = 8; // Number of bytes in the register + 2 bytes for the PEC
	uint8_t cmd[4];
	uint16_t cmd_pec;

	if (reg == 1)     //Read back auxiliary group A
	{
		cmd[1] = 0x0C;
		cmd[0] = 0x00;
	}
	else if (reg == 2)  //Read back auxiliary group B
	{
		cmd[1] = 0x0E;
		cmd[0] = 0x00;
	}
	else if (reg == 3)  //Read back auxiliary group C
	{
		cmd[1] = 0x0D;
		cmd[0] = 0x00;
	}
	else if (reg == 4)  //Read back auxiliary group D
	{
		cmd[1] = 0x0F;
		cmd[0] = 0x00;
	}
	else          //Read back auxiliary group A
	{
		cmd[1] = 0x0C;
		cmd[0] = 0x00;
	}

	cmd_pec = pec15_calc(2, cmd);
	cmd[2] = (uint8_t)(cmd_pec >> 8);
	cmd[3] = (uint8_t)(cmd_pec);

	cs_low(CS_PORT, CS_PIN);
    HAL_SPI_Transmit(&hspi1, cmd, 4, 100);
    uint8_t dummy_tx[total_ic * REG_LEN];
    memset(dummy_tx, 0xFF, sizeof(dummy_tx));

    HAL_SPI_TransmitReceive(&hspi1, dummy_tx, data, total_ic * REG_LEN, 100);
	cs_high(CS_PORT, CS_PIN);
}

/*
The function reads a single stat  register and stores the read data
in the *data point as a byte array. This function is rarely used outside of
the LTC681x_rdstat() command.
*/
void LTC6813_rdstat_reg(uint8_t reg, //Determines which stat register is read back
                        uint8_t total_ic, //The number of ICs in the system
                        uint8_t *data //Array of the unparsed stat codes
                       )
{
	const uint8_t REG_LEN = 8; // number of bytes in the register + 2 bytes for the PEC
	uint8_t cmd[4];
	uint16_t cmd_pec;

	if (reg == 1)     //Read back status group A
	{
		cmd[1] = 0x10;
		cmd[0] = 0x00;
	}
	else if (reg == 2)  //Read back status group B
	{
		cmd[1] = 0x12;
		cmd[0] = 0x00;
	}

	else          //Read back status group A
	{
		cmd[1] = 0x10;
		cmd[0] = 0x00;
	}

	cmd_pec = pec15_calc(2, cmd);
	cmd[2] = (uint8_t)(cmd_pec >> 8);
	cmd[3] = (uint8_t)(cmd_pec);

	cs_low(CS_PORT, CS_PIN);
	spi_write_read(&hspi1, cmd, 4, data, (REG_LEN*total_ic));
	cs_high(CS_PORT, CS_PIN);
}

/* Helper function that parses voltage measurement registers */
uint8_t parse_cells(uint8_t ic_idx, uint8_t reg_num, uint8_t *data_ptr, uint16_t *c_codes, uint8_t *pec_match)
{
    uint8_t errors = 0;
    uint16_t raw;

    // Compute PEC of received data
    uint16_t received_pec = ((uint16_t)data_ptr[6] << 8) | data_ptr[7];
    uint16_t calc_pec = pec15_calc(6, data_ptr);  // 6 bytes of data

    pec_match[reg_num - 1] = (received_pec == calc_pec) ? 0 : 1;
    if (pec_match[reg_num - 1])
        errors++;

    // Map register to cell indices
    uint8_t start_cell = 0;
    switch (reg_num)
    {
        case 1: start_cell = 0; break;  // Cells 1–3
        case 2: start_cell = 3; break;  // Cells 4–6
        case 3: start_cell = 6; break;  // Cells 7–9
        case 4: start_cell = 9; break;  // Cells 10–12
        case 5: start_cell = 12; break; // Cells 13–14 (pad one)
        case 6: start_cell = 14; break; // Reserved / future
        default: return errors;
    }

    // Parse 3 cells per register (except last register may have 2)
    for (uint8_t i = 0; i < 3; i++)
    {
        uint8_t cell_idx = start_cell + i;
        if (cell_idx >= 14) break;  // Only 14 cells per IC

        // LTC6813 stores each cell as 2 bytes (LSB first)
        raw = (uint16_t)data_ptr[i*2] | ((uint16_t)data_ptr[i*2 + 1] << 8);
        c_codes[cell_idx] = raw;
    }

    return errors;
}

/* Sends the poll ADC command */
uint8_t LTC6813_pladc()
{
	uint8_t cmd[4];
	uint8_t adc_state = 0xFF;
	uint16_t cmd_pec;

	cmd[0] = 0x07;
	cmd[1] = 0x14;
	cmd_pec = pec15_calc(2, cmd);
	cmd[2] = (uint8_t)(cmd_pec >> 8);
	cmd[3] = (uint8_t)(cmd_pec);

	cs_low(CS_PORT, CS_PIN);
	spi_write_array(&hspi1, 4, cmd);
	spi_read_byte(&hspi1, &adc_state);
	cs_high(CS_PORT, CS_PIN);

	return(adc_state);
}

/* This function will block operation until the ADC has finished it's conversion */
uint32_t LTC6813_pollAdc()
{
	uint32_t counter = 0;
	uint8_t finished = 0;
	uint8_t current_time = 0;
	uint8_t cmd[4];
	uint16_t cmd_pec;

	cmd[0] = 0x07;
	cmd[1] = 0x14;
	cmd_pec = pec15_calc(2, cmd);
	cmd[2] = (uint8_t)(cmd_pec >> 8);
	cmd[3] = (uint8_t)(cmd_pec);

	cs_low(CS_PORT, CS_PIN);
	spi_write_array(&hspi1, 4, cmd);
	while ((counter<200000)&&(finished == 0))
	{
		spi_read_byte(&hspi1, &current_time);
		if (current_time>0)
		{
			finished = 1;
		}
		else
		{
			counter = counter + 10;
		}
	}
	cs_high(CS_PORT, CS_PIN);

	return(counter);
}

/*
The command clears the cell voltage registers and initializes
all values to 1. The register will read back hexadecimal 0xFF
after the command is sent.
*/
void LTC6813_clrcell()
{
	uint8_t cmd[2]= {0x07 , 0x11};
	cmd_68(cmd);
}

/*
The command clears the Auxiliary registers and initializes
all values to 1. The register will read back hexadecimal 0xFF
after the command is sent.
*/
void LTC6813_clraux()
{
	uint8_t cmd[2]= {0x07 , 0x12};
	cmd_68(cmd);
}

/*
The command clears the Stat registers and initializes
all values to 1. The register will read back hexadecimal 0xFF
after the command is sent.
*/
void LTC6813_clrstat()
{
	uint8_t cmd[2]= {0x07 , 0x13};
	cmd_68(cmd);
}

/* Starts the Mux Decoder diagnostic self test */
void LTC6813_diagn()
{
	uint8_t cmd[2] = {0x07 , 0x15};
	cmd_68(cmd);
}

/* Starts cell voltage self test conversion */
void LTC6813_cvst(uint8_t MD, //ADC Mode
                  uint8_t ST //Self Test
                 )
{
	uint8_t cmd[2];
	uint8_t md_bits;

	md_bits = (MD & 0x02) >> 1;
	cmd[0] = md_bits + 0x02;
	md_bits = (MD & 0x01) << 7;
	cmd[1] =  md_bits + ((ST)<<5) +0x07;

	cmd_68(cmd);
}

/* Start an Auxiliary Register Self Test Conversion */
void LTC6813_axst(uint8_t MD, //ADC Mode
				  uint8_t ST //Self Test
                 )
{
	uint8_t cmd[2];
	uint8_t md_bits;

	md_bits = (MD & 0x02) >> 1;
	cmd[0] = md_bits + 0x04;
	md_bits = (MD & 0x01) << 7;
	cmd[1] =  md_bits + ((ST&0x03)<<5) +0x07;

	cmd_68(cmd);
}

/* Start a Status Register Self Test Conversion */
void LTC6813_statst(uint8_t MD, //ADC Mode
                    uint8_t ST //Self Test
					)
{
	uint8_t cmd[2];
	uint8_t md_bits;

	md_bits = (MD & 0x02) >> 1;
	cmd[0] = md_bits + 0x04;
	md_bits = (MD & 0x01) << 7;
	cmd[1] =  md_bits + ((ST&0x03)<<5) +0x0F;

	cmd_68(cmd);
}

/* Starts cell voltage overlap conversion */
void LTC6813_adol(uint8_t MD, //ADC Mode
                  uint8_t DCP //Discharge Permit
                 )
{
	uint8_t cmd[2];
	uint8_t md_bits;

	md_bits = (MD & 0x02) >> 1;
	cmd[0] = md_bits + 0x02;
	md_bits = (MD & 0x01) << 7;
	cmd[1] =  md_bits + (DCP<<4) +0x01;

	cmd_68(cmd);
}

/* Start an GPIO Redundancy test */
void LTC6813_adaxd(uint8_t MD, //ADC Mode
				   uint8_t CHG //GPIO Channels to be measured
                   )
{
	uint8_t cmd[4];
	uint8_t md_bits;

	md_bits = (MD & 0x02) >> 1;
	cmd[0] = md_bits + 0x04;
	md_bits = (MD & 0x01) << 7;
	cmd[1] = md_bits + CHG ;

	cmd_68(cmd);
}

/* Start a Status register redundancy test Conversion */
void LTC6813_adstatd(uint8_t MD, //ADC Mode
					 uint8_t CHST //Stat Channels to be measured
                    )
{
	uint8_t cmd[2];
	uint8_t md_bits;

	md_bits = (MD & 0x02) >> 1;
	cmd[0] = md_bits + 0x04;
	md_bits = (MD & 0x01) << 7;
	cmd[1] = md_bits + 0x08 + CHST ;

	cmd_68(cmd);
}

/* Runs the Digital Filter Self Test */
int16_t LTC6813_run_cell_adc_st(uint8_t adc_reg, // Type of register
								uint8_t total_ic, // Number of ICs in the system
								cell_asic *ic, // A two dimensional array that will store the data
								uint8_t md, //ADC Mode
								bool adcopt // The adcopt bit in the configuration register
								)
{
	int16_t error = 0;
	uint16_t expected_result = 0;

	for (int self_test = 1; self_test<3; self_test++)
	{
		expected_result = LTC6813_st_lookup(md,self_test,adcopt);
		wakeup_idle(total_ic);

		switch (adc_reg)
		{
		  case CELL:
			  wakeup_idle(total_ic);
			  LTC6813_clrcell();
			  LTC6813_cvst(md,self_test);
			  LTC6813_pollAdc();

			  wakeup_idle(total_ic);
			  error = LTC6813_rdcv(0,total_ic,ic);
			  for (int cic = 0; cic < total_ic; cic++)
				{
				  for (int channel=0; channel< ic[cic].ic_reg.cell_channels; channel++)
				  {

					if (ic[cic].cells.c_codes[channel] != expected_result)
					{
					  error = error+1;
					}
				  }
				}
			break;
		  case AUX:
			  error = 0;
			  wakeup_idle(total_ic);
			  LTC6813_clraux();
			  LTC6813_axst(md,self_test);
			  LTC6813_pollAdc();

			  wakeup_idle(total_ic);
			  LTC6813_rdaux(0, total_ic,ic);
			  for (int cic = 0; cic < total_ic; cic++)
				{
				  for (int channel=0; channel< ic[cic].ic_reg.aux_channels; channel++)
				  {

					if (ic[cic].aux.a_codes[channel] != expected_result)
					{
					  error = error+1;
					}
				  }
				}
			break;
		  case STAT:
			  wakeup_idle(total_ic);
			  LTC6813_clrstat();
			  LTC6813_statst(md,self_test);
			  LTC6813_pollAdc();

			  wakeup_idle(total_ic);
			  error = LTC6813_rdstat(0,total_ic,ic);
			  for (int cic = 0; cic < total_ic; cic++)
				{
				  for (int channel=0; channel< ic[cic].ic_reg.stat_channels; channel++)
				  {
					if (ic[cic].stat.stat_codes[channel] != expected_result)
					{
					  error = error+1;
					}
				  }
				}
			break;

		  default:
			error = -1;
			break;
		}
	}

	return(error);
}

/*  Runs the ADC overlap test for the IC */
uint16_t LTC6813_run_adc_overlap(uint8_t total_ic, // Number of ICs in the system
								cell_asic *ic // A two dimensional array that will store the data
								)
{
	uint16_t error = 0;
	int32_t measure_delta =0;
	int16_t failure_pos_limit = 20;
	int16_t failure_neg_limit = -20;
	//uint32_t conv_time=0;
	wakeup_idle(total_ic);
	LTC6813_adol(MD_7KHZ_3KHZ,DCP_DISABLED);
	//conv_time = LTC6813_pollAdc();

	wakeup_idle(total_ic);
	error = LTC6813_rdcv(0,total_ic,ic);
	for (int cic = 0; cic<total_ic; cic++)
	{


		measure_delta = (int32_t)ic[cic].cells.c_codes[6]-(int32_t)ic[cic].cells.c_codes[7];
		if ((measure_delta>failure_pos_limit) || (measure_delta<failure_neg_limit))
		{
		  error = error | (1<<(cic-1));
		}
		measure_delta = (int32_t)ic[cic].cells.c_codes[12]-(int32_t)ic[cic].cells.c_codes[13];
		if ((measure_delta>failure_pos_limit) || (measure_delta<failure_neg_limit))
		{
		  error = error | (1<<(cic-1));
		}
	}
	return(error);
}

/* Runs the redundancy self test */
int16_t LTC6813_run_adc_redundancy_st(uint8_t adc_mode, //ADC Mode
									  uint8_t adc_reg, // Type of register
									  uint8_t total_ic, // Number of ICs in the system
									  cell_asic *ic // A two dimensional array that will store the data
									  )
{
	int16_t error = 0;
	for (int self_test = 1; self_test<3; self_test++)
	{
		wakeup_idle(total_ic);
		switch (adc_reg)
		{
			case AUX:
			LTC6813_clraux();
			LTC6813_adaxd(adc_mode,AUX_CH_ALL);
			LTC6813_pollAdc();

			wakeup_idle(total_ic);
			error = LTC6813_rdaux(0, total_ic,ic);
			for (int cic = 0; cic < total_ic; cic++)
			{
				for (int channel=0; channel< ic[cic].ic_reg.aux_channels; channel++)
				{
					if (ic[cic].aux.a_codes[channel] >= 65280)
					{
						error = error+1;
					}
				}
			}
			break;
			case STAT:
			LTC6813_clrstat();
			LTC6813_adstatd(adc_mode,STAT_CH_ALL);
			LTC6813_pollAdc();
			wakeup_idle(total_ic);
			error = LTC6813_rdstat(0,total_ic,ic);
			for (int cic = 0; cic < total_ic; cic++)
			{
				for (int channel=0; channel< ic[cic].ic_reg.stat_channels; channel++)
				{
					if (ic[cic].stat.stat_codes[channel] >= 65280)
					{
						error = error+1;
					}
				}
			}
			break;

			default:
			error = -1;
			break;
		}
	}
	return(error);
}

/* Looks up the result pattern for digital filter self test */
uint16_t LTC6813_st_lookup(uint8_t MD, //ADC Mode
						   uint8_t ST, //Self Test
						   bool adcopt // ADCOPT bit in the configuration register
						  )
{
	uint16_t test_pattern = 0;

    if (MD == 1)
    {
		if ( adcopt == false)
		{
			if (ST == 1)
			{
				test_pattern = 0x9565;
			}
			else
			{
				test_pattern = 0x6A9A;
			}
		}
		else
		{
			if (ST == 1)
			{
				test_pattern = 0x9553;
			}
			else
			{
				test_pattern = 0x6AAC;
			}
		}
    }
    else
    {
		if (ST == 1)
		{
		   test_pattern = 0x9555;
		}
		else
		{
		   test_pattern = 0x6AAA;
		}
    }
    return(test_pattern);
}

/* Start an open wire Conversion */
void LTC6813_adow(uint8_t MD,   //ADC Mode
                  uint8_t PUP, //Pull up/Pull down current
				  uint8_t CH,  //Sets which Cell channels are converted
				  uint8_t DCP //Discharge Permit
				  )
{
	uint8_t cmd[2];
	uint8_t md_bits;

	md_bits = (MD & 0x02) >> 1;
	cmd[0] = md_bits + 0x02;
	md_bits = (MD & 0x01) << 7;
	cmd[1] =  md_bits + 0x28 + (PUP<<6) + CH+(DCP<<4);

	cmd_68(cmd);
}

/* Start GPIOs open wire ADC conversion */
void LTC6813_axow(uint8_t MD, //ADC Mode
				  uint8_t PUP //Pull up/Pull down current
				 )
{
	uint8_t cmd[2];
	uint8_t md_bits;

	md_bits = (MD & 0x02) >> 1;
	cmd[0] = md_bits + 0x04;
	md_bits = (MD & 0x01) << 7;
	cmd[1] =  md_bits + 0x10+ (PUP<<6) ;//+ CH;

	cmd_68(cmd);
}

/* Runs the data sheet algorithm for open wire for single cell detection */
void LTC6813_run_openwire_single(uint8_t total_ic, // Number of ICs in the system
								cell_asic *ic // A two dimensional array that will store the data
								)
{
	uint16_t OPENWIRE_THRESHOLD = 4000;
	const uint8_t  N_CHANNELS = ic[0].ic_reg.cell_channels;

	uint16_t pullUp[total_ic][N_CHANNELS];
	uint16_t pullDwn[total_ic][N_CHANNELS];
	int16_t openWire_delta[total_ic][N_CHANNELS];

	//int8_t error;
	int8_t i;
	//uint32_t conv_time=0;

	wakeup_sleep(total_ic);
	LTC6813_clrcell();

	// Pull Ups
	for (i = 0; i < 3; i++)
	{
	  wakeup_idle(total_ic);
	  LTC6813_adow(MD_26HZ_2KHZ,PULL_UP_CURRENT,CELL_CH_ALL,DCP_DISABLED);
	  //conv_time =LTC6813_pollAdc();
	}

	wakeup_idle(total_ic);
	//error=LTC6813_rdcv(0,total_ic,ic);

	for (int cic=0; cic<total_ic; cic++)
	{
	    for (int cell=0; cell<N_CHANNELS; cell++)
		{
		  pullUp[cic][cell] = ic[cic].cells.c_codes[cell];
		}
	}

	// Pull Downs
	for (i = 0; i < 3; i++)
	{
	  wakeup_idle(total_ic);
	  LTC6813_adow(MD_26HZ_2KHZ,PULL_DOWN_CURRENT,CELL_CH_ALL,DCP_DISABLED);
	  //conv_time =LTC6813_pollAdc();
	}

	wakeup_idle(total_ic);
	//error=LTC6813_rdcv(0,total_ic,ic);

	for (int cic=0; cic<total_ic; cic++)
	{
	    for (int cell=0; cell<N_CHANNELS; cell++)
		{
		   pullDwn[cic][cell] = ic[cic].cells.c_codes[cell];
		}
	}

	for (int cic=0; cic<total_ic; cic++)
	{
	  ic[cic].system_open_wire = 0xFFFF;

		for (int cell=0; cell<N_CHANNELS; cell++)
		{
			if (pullDwn[cic][cell] < pullUp[cic][cell])
			{
				openWire_delta[cic][cell] = (pullUp[cic][cell] - pullDwn[cic][cell]);
			}
			else
			{
				openWire_delta[cic][cell] = 0;
			}

			if (openWire_delta[cic][cell]>OPENWIRE_THRESHOLD)
			{
				ic[cic].system_open_wire = cell+1;
			}
		}

		if (pullUp[cic][0] == 0)
		{
		  ic[cic].system_open_wire = 0;
		}

		if (pullUp[cic][(N_CHANNELS-1)] == 0)//checking the Pull up value of the top measured channel
		{
		  ic[cic].system_open_wire = N_CHANNELS;
		}
	}
}

/* Runs the data sheet algorithm for open wire for multiple cell and two consecutive cells detection */
void LTC6813_run_openwire_multi(uint8_t total_ic, // Number of ICs in the system
								cell_asic *ic // A two dimensional array that will store the data
								)
{
	uint16_t OPENWIRE_THRESHOLD = 4000;
	const uint8_t  N_CHANNELS = ic[0].ic_reg.cell_channels;

	uint16_t pullUp[total_ic][N_CHANNELS];
	uint16_t pullDwn[total_ic][N_CHANNELS];
	uint16_t openWire_delta[total_ic][N_CHANNELS];

	//int8_t error;
	int8_t opencells[N_CHANNELS];
	int8_t n=0;
	int8_t i,j,k;
	//uint32_t conv_time=0;

	wakeup_sleep(total_ic);
	LTC6813_clrcell();

	// Pull Ups
	for (i = 0; i < 5; i++)
	{
		wakeup_idle(total_ic);
		LTC6813_adow(MD_26HZ_2KHZ,PULL_UP_CURRENT,CELL_CH_ALL,DCP_DISABLED);
		//conv_time =LTC6813_pollAdc();
	}

	wakeup_idle(total_ic);
	//error = LTC6813_rdcv(0,total_ic,ic);

	for (int cic=0; cic<total_ic; cic++)
	{
	    for (int cell=0; cell<N_CHANNELS; cell++)
		{
		  pullUp[cic][cell] = ic[cic].cells.c_codes[cell];
		}
	}

	// Pull Downs
	for (i = 0; i < 5; i++)
	{
	  wakeup_idle(total_ic);
	  LTC6813_adow(MD_26HZ_2KHZ,PULL_DOWN_CURRENT,CELL_CH_ALL,DCP_DISABLED);
	  //conv_time =   LTC6813_pollAdc();
	}

	wakeup_idle(total_ic);
	//error = LTC6813_rdcv(0,total_ic,ic);

	for (int cic=0; cic<total_ic; cic++)
	{
		for (int cell=0; cell<N_CHANNELS; cell++)
		{
		   pullDwn[cic][cell] = ic[cic].cells.c_codes[cell];
		}
	}

	for (int cic=0; cic<total_ic; cic++)
	{
		for (int cell=0; cell<N_CHANNELS; cell++)
		{
			if (pullDwn[cic][cell] < pullUp[cic][cell])
				{
					openWire_delta[cic][cell] = (pullUp[cic][cell] - pullDwn[cic][cell]);
				}
				else
				{
					openWire_delta[cic][cell] = 0;
				}
		}
	}

	for (int cic=0; cic<total_ic; cic++)
	{
		n=0;

		char cicbuf[16];
		sprintf(cicbuf, "%d", cic+1);
		HAL_UART_Transmit(&huart1, (uint8_t *)"IC:", 3, HAL_MAX_DELAY);
		HAL_UART_Transmit(&huart1, (uint8_t *)cicbuf, strlen(cicbuf), HAL_MAX_DELAY);

		for (int cell=0; cell<N_CHANNELS; cell++)
		{

		  if (openWire_delta[cic][cell]>OPENWIRE_THRESHOLD)
			{
				opencells[n] = cell+1;
				n++;
				for (int j = cell; j < N_CHANNELS-3 ; j++)
				{
					if (pullUp[cic][j + 2] == 0)
					{
					opencells[n] = j+2;
					n++;
					}
				}
				if((cell==N_CHANNELS-4) && (pullDwn[cic][N_CHANNELS-3] == 0))
				{
					  opencells[n] = N_CHANNELS-2;
					  n++;
				}
			}
		}
		if (pullDwn[cic][0] == 0)
		{
		  opencells[n] = 0;
		  HAL_UART_Transmit(&huart1,
				  	  	  	(uint8_t *)"Cell 0 is Open and multiple open wires maybe possible.",
							strlen("Cell 0 is Open and multiple open wires maybe possible."),
							HAL_MAX_DELAY
		  	  	  	  	    );
		  n++;
		}

		if (pullDwn[cic][N_CHANNELS-1] == 0)
		{
		  opencells[n] = N_CHANNELS;
		  n++;
		}

		if (pullDwn[cic][N_CHANNELS-2] == 0)
		{
		  opencells[n] = N_CHANNELS-1;
		  n++;
		}

	//Removing repetitive elements
		for(i=0;i<n;i++)
		{
			for(j=i+1;j<n;)
			{
				if(opencells[i]==opencells[j])
				{
					for(k=j;k<n;k++)
						opencells[k]=opencells[k+1];

					n--;
				}
				else
					j++;
			}
		}

	// Sorting open cell array
		for(int i=0; i<n; i++)
		{
			for(int j=0; j<n-1; j++)
			{
				if( opencells[j] > opencells[j+1] )
				{
					k = opencells[j];
					opencells[j] = opencells[j+1];
					opencells[j+1] = k;
				}
			}
		}

	//Checking the value of n
		char nbuf[16];
		sprintf(nbuf, "%d", n);
		HAL_UART_Transmit(&huart1, (uint8_t *)"Number of Open wires:", strlen("Number of Open wires:"), HAL_MAX_DELAY);
		HAL_UART_Transmit(&huart1, (uint8_t *)nbuf, strlen(nbuf), HAL_MAX_DELAY);

	//Printing open cell array
		HAL_UART_Transmit(&huart1, (uint8_t *)"OPEN CELLS:", strlen("OPEN CELLS:"), HAL_MAX_DELAY);
		if(n==0)
		{
			HAL_UART_Transmit(&huart1, (uint8_t *)"No Open wires", strlen("No Open wires"), HAL_MAX_DELAY);
		}
		else
		{
			for(i=0;i<n;i++)
			{
					char cellbuf[16];
				    sprintf(cellbuf, "%d", opencells[i]);
					HAL_UART_Transmit(&huart1, (uint8_t *)cellbuf, strlen(cellbuf), HAL_MAX_DELAY);
			}
		}
	}
	HAL_UART_Transmit(&huart1, (uint8_t *)"\r\n", strlen("\r\n"), HAL_MAX_DELAY);
}

/* Runs open wire for GPIOs */
void LTC6813_run_gpio_openwire(uint8_t total_ic, // Number of ICs in the system
								cell_asic *ic // A two dimensional array that will store the data
								)
{
	uint16_t OPENWIRE_THRESHOLD = 150;
	const uint8_t  N_CHANNELS = ic[0].ic_reg.aux_channels +1;

	uint16_t aux_val[total_ic][N_CHANNELS];
	uint16_t pDwn[total_ic][N_CHANNELS];
	uint16_t ow_delta[total_ic][N_CHANNELS];

	//int8_t error;
	int8_t i;
	//uint32_t conv_time=0;

	wakeup_sleep(total_ic);
	LTC6813_clraux();

	for (i = 0; i < 3; i++)
	{
	   wakeup_idle(total_ic);
	   LTC6813_adax(MD_7KHZ_3KHZ, AUX_CH_ALL);
	   //conv_time= LTC6813_pollAdc();
	}

	wakeup_idle(total_ic);
	//error = LTC6813_rdaux(0, total_ic,ic);

	for (int cic=0; cic<total_ic; cic++)
	{
	    for (int channel=0; channel<N_CHANNELS; channel++)
		{
			aux_val[cic][channel]=ic[cic].aux.a_codes[channel];
		}
	}
	LTC6813_clraux();

	// pull downs
	for (i = 0; i < 3; i++)
	{
	   wakeup_idle(total_ic);
	   LTC6813_axow(MD_7KHZ_3KHZ,PULL_DOWN_CURRENT);
	   //conv_time =LTC6813_pollAdc();
	}

	wakeup_idle(total_ic);
	//error = LTC6813_rdaux(0, total_ic,ic);

	for (int cic=0; cic<total_ic; cic++)
	{
	   for (int channel=0; channel<N_CHANNELS; channel++)
		{
			pDwn[cic][channel]=ic[cic].aux.a_codes[channel] ;
		}
	}

	for (int cic=0; cic<total_ic; cic++)
	{
		ic[cic].system_open_wire = 0xFFFF;

		for (int channel=0; channel<N_CHANNELS; channel++)
		{
			if (pDwn[cic][channel] > aux_val[cic][channel])
			{
				ow_delta[cic][channel] = (pDwn[cic][channel] - aux_val[cic][channel]);
			}
			else
			{
				ow_delta[cic][channel] = 0;
			}

			if(channel<5)
			{
				if (ow_delta[cic][channel] > OPENWIRE_THRESHOLD)
				{
					ic[cic].system_open_wire= channel+1;

				}
			}
			else if(channel>5)
			{
				if (ow_delta[cic][channel] > OPENWIRE_THRESHOLD)
				{
					ic[cic].system_open_wire= channel;

				}
			}
		}
	}
}

/* Helper function to set discharge bit in CFG register */
void LTC6813_set_discharge(int Cell, // Cell to be discharged
						   uint8_t total_ic, // Number of ICs in the system
						   cell_asic *ic // A two dimensional array that will store the data
						   )
{
	for (int i=0; i<total_ic; i++)
	{
		if (Cell==0)
		{
		  ic[i].configb.tx_data[1] = ic[i].configb.tx_data[1] |(0x04);
		}
		else if (Cell<9)
		{
		  ic[i].config.tx_data[4] = ic[i].config.tx_data[4] | (1<<(Cell-1));
		}
		else if (Cell < 13)
		{
		  ic[i].config.tx_data[5] = ic[i].config.tx_data[5] | (1<<(Cell-9));
		}
		else if (Cell<17)
		{
		  ic[i].configb.tx_data[0] = ic[i].configb.tx_data[0] | (1<<(Cell-9));
		}
		else if (Cell<19)
		{
		  ic[i].configb.tx_data[1] = ic[i].configb.tx_data[1] | (1<<(Cell-17));
		}
		else
		{
			break;
		}
	}
}

/* Clears all of the DCC bits in the configuration registers */
void LTC6813_clear_discharge(uint8_t total_ic, // Number of ICs in the system
                             cell_asic *ic // A two dimensional array that will store the data
							 )
{
	for (int i=0; i<total_ic; i++)
	{
	   ic[i].config.tx_data[4] = 0;
	   ic[i].config.tx_data[5] =ic[i].config.tx_data[5]&(0xF0);
	   ic[i].configb.tx_data[0]=ic[i].configb.tx_data[0]&(0x0F);
	   ic[i].configb.tx_data[1]=ic[i].configb.tx_data[1]&(0xF0);
	}
}

/* Writes the pwm registers of a LTC6813 daisy chain  */
void LTC6813_wrpwm(uint8_t total_ic, // Number of ICs in the system
				   uint8_t pwmReg,  // PWM Register A or B
				   cell_asic *ic //A two dimensional array of the configuration data that will be written
				  )
{
	uint8_t cmd[2];
	uint8_t write_buffer[256];
	uint8_t write_count = 0;
	uint8_t c_ic = 0;
	if (pwmReg == 0)
	{
	cmd[0] = 0x00;
	cmd[1] = 0x20;
	}
	else
	{
	cmd[0] = 0x00;
	cmd[1] = 0x1C;
	}

	for (uint8_t current_ic = 0; current_ic<total_ic; current_ic++)
	{
		if (ic[0].isospi_reverse == false)
		{
			c_ic = current_ic;
		}
		else
		{
			c_ic = total_ic - current_ic - 1;
		}

		for (uint8_t data = 0; data<6; data++)
		{
			write_buffer[write_count] = ic[c_ic].pwm.tx_data[data];
			write_count++;
		}
	}
	write_68(total_ic, cmd, write_buffer);
}

/* Reads pwm registers of a LTC6813 daisy chain */
int8_t LTC6813_rdpwm(uint8_t total_ic, //Number of ICs in the system
					 uint8_t pwmReg, // PWM Register A or B
				     cell_asic *ic //A two dimensional array that the function stores the read configuration data.
				    )
{
	//const uint8_t BYTES_IN_REG = 8;
	uint8_t cmd[4];
	uint8_t read_buffer[256];
	int8_t pec_error = 0;
	uint16_t data_pec;
	uint16_t calc_pec;
	uint8_t c_ic = 0;

	if (pwmReg == 0)
	{
		cmd[0] = 0x00;
		cmd[1] = 0x22;
	}
	else
	{
		cmd[0] = 0x00;
		cmd[1] = 0x1E;
	}

	pec_error = read_68(total_ic, cmd, read_buffer);
	for (uint8_t current_ic =0; current_ic<total_ic; current_ic++)
	{
		if (ic[0].isospi_reverse == false)
		{
			c_ic = current_ic;
		}
		else
		{
			c_ic = total_ic - current_ic - 1;
		}

		for (int byte=0; byte<8; byte++)
		{
			ic[c_ic].pwm.rx_data[byte] = read_buffer[byte+(8*current_ic)];
		}

		calc_pec = pec15_calc(6,&read_buffer[8*current_ic]);
		data_pec = read_buffer[7+(8*current_ic)] | (read_buffer[6+(8*current_ic)]<<8);
		if (calc_pec != data_pec )
		{
			ic[c_ic].pwm.rx_pec_match = 1;
		}
		else ic[c_ic].pwm.rx_pec_match = 0;
	}
	return(pec_error);
}

/* Writes data in S control register the ltc6813-1  connected in a daisy chain stack */
void LTC6813_wrsctrl(uint8_t total_ic, // number of ICs in the daisy chain
                     uint8_t sctrl_reg, // SCTRL Register A or B
                     cell_asic *ic // A two dimensional array that will store the data
                    )
{
	uint8_t cmd[2];
    uint8_t write_buffer[256];
    uint8_t write_count = 0;
    uint8_t c_ic = 0;
    if (sctrl_reg == 0)
    {
      cmd[0] = 0x00;
      cmd[1] = 0x14;
    }
    else
    {
      cmd[0] = 0x00;
      cmd[1] = 0x1C;
    }

    for(uint8_t current_ic = 0; current_ic<total_ic;current_ic++)
    {
        if(ic[0].isospi_reverse == false){c_ic = current_ic;}
        else{c_ic = total_ic - current_ic - 1;}


        for(uint8_t data = 0; data<6;data++)
        {
            write_buffer[write_count] = ic[c_ic].sctrl.tx_data[data];
            write_count++;
        }
    }
    write_68(total_ic, cmd, write_buffer);
}

/* Reads sctrl registers of a LTC6813 daisy chain */
int8_t LTC6813_rdsctrl(uint8_t total_ic, // number of ICs in the daisy chain
                       uint8_t sctrl_reg, // SCTRL Register A or B
                       cell_asic *ic //< a two dimensional array that the function stores the read data
                      )
{
    uint8_t cmd[4];
    uint8_t read_buffer[256];
    int8_t pec_error = 0;
    uint16_t data_pec;
    uint16_t calc_pec;
    uint8_t c_ic = 0;

    if (sctrl_reg == 0)
    {
      cmd[0] = 0x00;
      cmd[1] = 0x16;
	  }
    else
    {
      cmd[0] = 0x00;
      cmd[1] = 0x1E;
	}

    pec_error = read_68(total_ic, cmd, read_buffer);

    for(uint8_t current_ic =0; current_ic<total_ic; current_ic++)
    {
        if(ic[0].isospi_reverse == false){c_ic = current_ic;}
        else{c_ic = total_ic - current_ic - 1;}


        for(int byte=0; byte<8;byte++)
        {
            ic[c_ic].sctrl.rx_data[byte] = read_buffer[byte+(8*current_ic)];
        }

        calc_pec = pec15_calc(6,&read_buffer[8*current_ic]);
        data_pec = read_buffer[7+(8*current_ic)] | (read_buffer[6+(8*current_ic)]<<8);
        if(calc_pec != data_pec )
        {
            ic[c_ic].sctrl.rx_pec_match = 1;
        }
        else ic[c_ic].sctrl.rx_pec_match = 0;

    }
    return(pec_error);
}

/*
Start Sctrl data communication
This command will start the sctrl pulse communication over the spins
*/
void LTC6813_stsctrl()
{
	uint8_t cmd[4];
    uint16_t cmd_pec;

    cmd[0] = 0x00;
    cmd[1] = 0x19;
    cmd_pec = pec15_calc(2, cmd);
    cmd[2] = (uint8_t)(cmd_pec >> 8);
    cmd[3] = (uint8_t)(cmd_pec);

    cs_low(CS_PORT, CS_PIN);
    spi_write_array(&hspi1, 4, cmd);
    cs_high(CS_PORT, CS_PIN);
}

/*
The command clears the Sctrl registers and initializes
all values to 0. The register will read back hexadecimal 0x00
after the command is sent.
*/
void LTC6813_clrsctrl()
{
	uint8_t cmd[2]= {0x00 , 0x18};
	cmd_68(cmd);
}

/* Write the 6813 PWM/Sctrl Register B  */
void LTC6813_wrpsb(uint8_t total_ic, // Number of ICs in the system
					cell_asic *ic // A two dimensional array that will store the data
					)
{
	uint8_t cmd[2];
	uint8_t write_buffer[256];
	uint8_t c_ic = 0;

	cmd[0] = 0x00;
	cmd[1] = 0x1C;
	for(uint8_t current_ic = 0; current_ic<total_ic;current_ic++)
	{
		if(ic[0].isospi_reverse == true){c_ic = current_ic;}
		else{c_ic = total_ic - current_ic - 1;}

		write_buffer[0] = ic[c_ic].pwmb.tx_data[0];
		write_buffer[1] = ic[c_ic].pwmb.tx_data[1];
		write_buffer[2]= ic[c_ic].pwmb.tx_data[2];
		write_buffer[3] = ic[c_ic].sctrlb.tx_data[3];
		write_buffer[4] = ic[c_ic].sctrlb.tx_data[4];
		write_buffer[5]= ic[c_ic].sctrlb.tx_data[5];
	}
	write_68(total_ic, cmd, write_buffer);
}

/* Reading the 6813 PWM/Sctrl Register B */
uint8_t LTC6813_rdpsb(uint8_t total_ic, //< number of ICs in the daisy chain
                      cell_asic *ic //< a two dimensional array that the function stores the read data
                      )
{
    uint8_t cmd[4];
    uint8_t read_buffer[256];
    int8_t pec_error = 0;
    uint16_t data_pec;
    uint16_t calc_pec;
    uint8_t c_ic = 0;
	cmd[0] = 0x00;
	cmd[1] = 0x1E;
    pec_error = read_68(total_ic, cmd, read_buffer);

    for(uint8_t current_ic =0; current_ic<total_ic; current_ic++)
    {
        if(ic[0].isospi_reverse == false){c_ic = current_ic;}
        else{c_ic = total_ic - current_ic - 1;}
        for(int byte=0; byte<3;byte++)
        {
            ic[c_ic].pwmb.rx_data[byte] = read_buffer[byte+(8*current_ic)];
        }

		for(int byte=3; byte<6;byte++)
        {
            ic[c_ic].sctrlb.rx_data[byte] = read_buffer[byte+(8*current_ic)];
        }

		for(int byte=6; byte<8;byte++)
        {
			ic[c_ic].pwmb.rx_data[byte] = read_buffer[byte+(8*current_ic)];
            ic[c_ic].sctrlb.rx_data[byte] = read_buffer[byte+(8*current_ic)];
        }

        calc_pec = pec15_calc(6,&read_buffer[8*current_ic]);
        data_pec = read_buffer[7+(8*current_ic)] | (read_buffer[6+(8*current_ic)]<<8);
        if(calc_pec != data_pec )
        {
            ic[c_ic].pwmb.rx_pec_match = 1;
			ic[c_ic].sctrlb.rx_pec_match = 1;

        }
        else
		{
			ic[c_ic].pwmb.rx_pec_match = 0;
			ic[c_ic].sctrlb.rx_pec_match = 0;

		}
    }
    return(pec_error);
}

/* Writes the COMM registers of a LTC6813 daisy chain */
void LTC6813_wrcomm(uint8_t total_ic, //The number of ICs being written to
				    cell_asic *ic //A two dimensional array of the comm data that will be written
				   )
{
	uint8_t cmd[2]= {0x07 , 0x21};
	uint8_t write_buffer[256];
	uint8_t write_count = 0;
	uint8_t c_ic = 0;
	for (uint8_t current_ic = 0; current_ic<total_ic; current_ic++)
	{
		if (ic[0].isospi_reverse == false)
		{
			c_ic = current_ic;
		}
		else
		{
			c_ic = total_ic - current_ic - 1;
		}

		for (uint8_t data = 0; data<6; data++)
		{
			write_buffer[write_count] = ic[c_ic].com.tx_data[data];
			write_count++;
		}
	}
	write_68(total_ic, cmd, write_buffer);
}

/* Reads COMM registers of a LTC6813 daisy chain */
int8_t LTC6813_rdcomm(uint8_t total_ic, //Number of ICs in the system
					  cell_asic *ic //A two dimensional array that the function stores the read data.
				     )
{
	uint8_t cmd[2]= {0x07 , 0x22};
	uint8_t read_buffer[256];
	int8_t pec_error = 0;
	uint16_t data_pec;
	uint16_t calc_pec;
	uint8_t c_ic=0;

	pec_error = read_68(total_ic, cmd, read_buffer);

	for (uint8_t current_ic = 0; current_ic<total_ic; current_ic++)
	{
		if (ic[0].isospi_reverse == false)
		{
			c_ic = current_ic;
		}
		else
		{
			c_ic = total_ic - current_ic - 1;
		}

		for (int byte=0; byte<8; byte++)
		{
			ic[c_ic].com.rx_data[byte] = read_buffer[byte+(8*current_ic)];
		}

		calc_pec = pec15_calc(6,&read_buffer[8*current_ic]);
		data_pec = read_buffer[7+(8*current_ic)] | (read_buffer[6+(8*current_ic)]<<8);
		if (calc_pec != data_pec )
		{
			ic[c_ic].com.rx_pec_match = 1;
		}
		else ic[c_ic].com.rx_pec_match = 0;
	}

    return(pec_error);
}

/* Shifts data in COMM register out over LTC6813 SPI/I2C port */
void LTC6813_stcomm(uint8_t len) //Length of data to be transmitted
{
	uint8_t cmd[4];
	uint16_t cmd_pec;

	cmd[0] = 0x07;
	cmd[1] = 0x23;
	cmd_pec = pec15_calc(2, cmd);
	cmd[2] = (uint8_t)(cmd_pec >> 8);
	cmd[3] = (uint8_t)(cmd_pec);

	cs_low(CS_PORT, CS_PIN);
	spi_write_array(&hspi1, 4, cmd);
	for (int i = 0; i<len; i++)
	{
	    uint8_t dummy = 0;
	    spi_read_byte(&hspi1, &dummy);
	}
	cs_high(CS_PORT, CS_PIN);
}

/* Mutes the LTC6813 discharge transistors */
void LTC6813_mute()
{
	uint8_t cmd[2];

	cmd[0] = 0x00;
	cmd[1] = 0x28;
	cmd_68(cmd);
}

/* Clears the LTC6813 Mute Discharge */
void LTC6813_unmute()
{
	uint8_t cmd[2];

	cmd[0] = 0x00;
	cmd[1] = 0x29;
	cmd_68(cmd);
}

/* Helper function that increments PEC counters */
void LTC6813_check_pec(uint8_t total_ic,//Number of ICs in the system
						uint8_t reg, //  Type of register
						cell_asic *ic // A two dimensional array that will store the data
						)
{
	switch (reg)
	{
		case CFGR:
		  for (int current_ic = 0 ; current_ic < total_ic; current_ic++)
		  {
			ic[current_ic].crc_count.pec_count = ic[current_ic].crc_count.pec_count + ic[current_ic].config.rx_pec_match;
			ic[current_ic].crc_count.cfgr_pec = ic[current_ic].crc_count.cfgr_pec + ic[current_ic].config.rx_pec_match;
		  }
		break;

		case CFGRB:
		  for (int current_ic = 0 ; current_ic < total_ic; current_ic++)
		  {
			ic[current_ic].crc_count.pec_count = ic[current_ic].crc_count.pec_count + ic[current_ic].configb.rx_pec_match;
			ic[current_ic].crc_count.cfgr_pec = ic[current_ic].crc_count.cfgr_pec + ic[current_ic].configb.rx_pec_match;
		  }
		break;
		case CELL:
		  for (int current_ic = 0 ; current_ic < total_ic; current_ic++)
		  {
			for (int i=0; i<ic[0].ic_reg.num_cv_reg; i++)
			{
			  ic[current_ic].crc_count.pec_count = ic[current_ic].crc_count.pec_count + ic[current_ic].cells.pec_match[i];
			  ic[current_ic].crc_count.cell_pec[i] = ic[current_ic].crc_count.cell_pec[i] + ic[current_ic].cells.pec_match[i];
			}
		  }
		break;
		case AUX:
		  for (int current_ic = 0 ; current_ic < total_ic; current_ic++)
		  {
			for (int i=0; i<ic[0].ic_reg.num_gpio_reg; i++)
			{
			  ic[current_ic].crc_count.pec_count = ic[current_ic].crc_count.pec_count + (ic[current_ic].aux.pec_match[i]);
			  ic[current_ic].crc_count.aux_pec[i] = ic[current_ic].crc_count.aux_pec[i] + (ic[current_ic].aux.pec_match[i]);
			}
		  }

		break;
		case STAT:
		  for (int current_ic = 0 ; current_ic < total_ic; current_ic++)
		  {

			for (int i=0; i<ic[0].ic_reg.num_stat_reg-1; i++)
			{
			  ic[current_ic].crc_count.pec_count = ic[current_ic].crc_count.pec_count + ic[current_ic].stat.pec_match[i];
			  ic[current_ic].crc_count.stat_pec[i] = ic[current_ic].crc_count.stat_pec[i] + ic[current_ic].stat.pec_match[i];
			}
		  }
		break;
		default:
		break;
	}
}

/* Helper Function to reset PEC counters */
void LTC6813_reset_crc_count(uint8_t total_ic, //Number of ICs in the system
							 cell_asic *ic // A two dimensional array that will store the data
							 )
{
	for (int current_ic = 0 ; current_ic < total_ic; current_ic++)
	{
		ic[current_ic].crc_count.pec_count = 0;
		ic[current_ic].crc_count.cfgr_pec = 0;
		for (int i=0; i<6; i++)
		{
			ic[current_ic].crc_count.cell_pec[i]=0;

		}
		for (int i=0; i<4; i++)
		{
			ic[current_ic].crc_count.aux_pec[i]=0;
		}
		for (int i=0; i<2; i++)
		{
			ic[current_ic].crc_count.stat_pec[i]=0;
		}
	}
}

/* Helper function to initialize CFG variables */
void LTC6813_init_cfg(uint8_t total_ic, cell_asic *ic)
{
	for (uint8_t current_ic = 0; current_ic<total_ic;current_ic++)
	{
		for (int j = 1; j<6; j++)
		{
		  ic[current_ic].config.tx_data[j] = 0;
		}

	}

	bool gpio[5] = {true, false, false, true, true};
	for (int i=0;i<total_ic;i++) {
	  	LTC6813_set_cfgr_refon(i, ic, true);
	  	LTC6813_set_cfgr_gpio(i, ic, gpio);
	}

}

/* Helper function to set CFGR variable */
void LTC6813_set_cfgr(uint8_t nIC, cell_asic *ic, bool refon, bool adcopt, bool gpio[5],bool dcc[12],bool dcto[4], uint16_t uv, uint16_t  ov)
{
    LTC6813_set_cfgr_refon(nIC,ic,refon);
    LTC6813_set_cfgr_adcopt(nIC,ic,adcopt);
    LTC6813_set_cfgr_gpio(nIC,ic,gpio);
    LTC6813_set_cfgr_dis(nIC,ic,dcc);
	LTC6813_set_cfgr_dcto(nIC,ic,dcto);
	LTC6813_set_cfgr_uv(nIC, ic, uv);
    LTC6813_set_cfgr_ov(nIC, ic, ov);
}

/* Helper function to set the REFON bit */
void LTC6813_set_cfgr_refon(uint8_t nIC, cell_asic *ic, bool refon)
{
	if (refon) ic[nIC].config.tx_data[0] = ic[nIC].config.tx_data[0]|0x04;
	else ic[nIC].config.tx_data[0] = ic[nIC].config.tx_data[0]&0xFB;
}

/* Helper function to set the adcopt bit */
void LTC6813_set_cfgr_adcopt(uint8_t nIC, cell_asic *ic, bool adcopt)
{
	if (adcopt) ic[nIC].config.tx_data[0] = ic[nIC].config.tx_data[0]|0x01;
	else ic[nIC].config.tx_data[0] = ic[nIC].config.tx_data[0]&0xFE;
}

/* Helper function to set GPIO bits */
void LTC6813_set_cfgr_gpio(uint8_t nIC, cell_asic *ic,bool gpio[5])
{
	for (int i =0; i<5; i++)
	{
		if (gpio[i])ic[nIC].config.tx_data[0] = ic[nIC].config.tx_data[0]|(0x01<<(i+3));
		else ic[nIC].config.tx_data[0] = ic[nIC].config.tx_data[0]&(~(0x01<<(i+3)));
	}
}

/* Helper function to control discharge */
void LTC6813_set_cfgr_dis(uint8_t nIC, cell_asic *ic,bool dcc[12])
{
	for (int i =0; i<8; i++)
	{
		if (dcc[i])ic[nIC].config.tx_data[4] = ic[nIC].config.tx_data[4]|(0x01<<i);
		else ic[nIC].config.tx_data[4] = ic[nIC].config.tx_data[4]& (~(0x01<<i));
	}
	for (int i =0; i<4; i++)
	{
		if (dcc[i+8])ic[nIC].config.tx_data[5] = ic[nIC].config.tx_data[5]|(0x01<<i);
		else ic[nIC].config.tx_data[5] = ic[nIC].config.tx_data[5]&(~(0x01<<i));
	}
}

/* Helper Function to set uv value in CFG register */
void LTC6813_set_cfgr_uv(uint8_t nIC, cell_asic *ic,uint16_t uv)
{
	uint16_t tmp = (uv/16)-1;
	ic[nIC].config.tx_data[1] = 0x00FF & tmp;
	ic[nIC].config.tx_data[2] = ic[nIC].config.tx_data[2]&0xF0;
	ic[nIC].config.tx_data[2] = ic[nIC].config.tx_data[2]|((0x0F00 & tmp)>>8);
}

/* Helper Function to set dcto value in CFG register */
void LTC6813_set_cfgr_dcto(uint8_t nIC, cell_asic *ic,bool dcto[4])
{
	for(int i =0;i<4;i++)
	{
		if(dcto[i])ic[nIC].config.tx_data[5] = ic[nIC].config.tx_data[5]|(0x01<<(i+4));
		else ic[nIC].config.tx_data[5] = ic[nIC].config.tx_data[5]&(~(0x01<<(i+4)));
	}
}

/* Helper function to set OV value in CFG register */
void LTC6813_set_cfgr_ov(uint8_t nIC, cell_asic *ic,uint16_t ov)
{
	uint16_t tmp = (ov/16);
	ic[nIC].config.tx_data[3] = 0x00FF & (tmp>>4);
	ic[nIC].config.tx_data[2] = ic[nIC].config.tx_data[2]&0x0F;
	ic[nIC].config.tx_data[2] = ic[nIC].config.tx_data[2]|((0x000F & tmp)<<4);
}

/* Helper Function to initialize the CFGRB data structures */
void LTC6813_init_cfgb(uint8_t total_ic,cell_asic *ic)
{
	for (uint8_t current_ic = 0; current_ic<total_ic;current_ic++)
    {
		for(int j =0; j<6;j++)
        {
            ic[current_ic].configb.tx_data[j] = 0;
        }
    }
}

/* Helper Function to set the configuration register B */
void LTC6813_set_cfgrb(uint8_t nIC, cell_asic *ic,bool fdrf,bool dtmen,bool ps[2],bool gpiobits[4],bool dccbits[7])
{
    LTC6813_set_cfgrb_fdrf(nIC,ic,fdrf);
    LTC6813_set_cfgrb_dtmen(nIC,ic,dtmen);
    LTC6813_set_cfgrb_ps(nIC,ic,ps);
    LTC6813_set_cfgrb_gpio_b(nIC,ic,gpiobits);
	LTC6813_set_cfgrb_dcc_b(nIC,ic,dccbits);
}

/* Helper function to set the FDRF bit */
void LTC6813_set_cfgrb_fdrf(uint8_t nIC, cell_asic *ic, bool fdrf)
{
	if(fdrf) ic[nIC].configb.tx_data[1] = ic[nIC].configb.tx_data[1]|0x40;
	else ic[nIC].configb.tx_data[1] = ic[nIC].configb.tx_data[1]&0xBF;
}

/* Helper function to set the DTMEN bit */
void LTC6813_set_cfgrb_dtmen(uint8_t nIC, cell_asic *ic, bool dtmen)
{
	if(dtmen) ic[nIC].configb.tx_data[1] = ic[nIC].configb.tx_data[1]|0x08;
	else ic[nIC].configb.tx_data[1] = ic[nIC].configb.tx_data[1]&0xF7;
}

/* Helper function to set the PATH SELECT bit */
void LTC6813_set_cfgrb_ps(uint8_t nIC, cell_asic *ic, bool ps[])
{
	for(int i =0;i<2;i++)
	{
	  if(ps[i])ic[nIC].configb.tx_data[1] = ic[nIC].configb.tx_data[1]|(0x01<<(i+4));
	  else ic[nIC].configb.tx_data[1] = ic[nIC].configb.tx_data[1]&(~(0x01<<(i+4)));
	}
}

/*  Helper function to set the gpio bits in configb b register  */
void LTC6813_set_cfgrb_gpio_b(uint8_t nIC, cell_asic *ic, bool gpiobits[])
{
	for(int i =0;i<4;i++)
	{
	  if(gpiobits[i])ic[nIC].configb.tx_data[0] = ic[nIC].configb.tx_data[0]|(0x01<<i);
	  else ic[nIC].configb.tx_data[0] = ic[nIC].configb.tx_data[0]&(~(0x01<<i));
	}
}

/*  Helper function to set the dcc bits in configb b register */
void LTC6813_set_cfgrb_dcc_b(uint8_t nIC, cell_asic *ic, bool dccbits[])
{
	for(int i =0;i<7;i++)
	{
		if(i==0)
		{
			if(dccbits[i])ic[nIC].configb.tx_data[1] = ic[nIC].configb.tx_data[1]|0x04;
			else ic[nIC].configb.tx_data[1] = ic[nIC].configb.tx_data[1]&0xFB;
		}
		if(i>0 && i<5)
		{
			if(dccbits[i])ic[nIC].configb.tx_data[0] = ic[nIC].configb.tx_data[0]|(0x01<<(i+3));
			else ic[nIC].configb.tx_data[0] = ic[nIC].configb.tx_data[0]&(~(0x01<<(i+3)));
		}
		if(i>4 && i<7)
		{
			if(dccbits[i])ic[nIC].configb.tx_data[1] = ic[nIC].configb.tx_data[1]|(0x01<<(i-5));
			else ic[nIC].configb.tx_data[1] = ic[nIC].configb.tx_data[1]&(~(0x01<<(i-5)));
		}
	}
}





