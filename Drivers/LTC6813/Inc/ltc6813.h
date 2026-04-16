/*
 * ltc6813.h
 *
 *  Created on: 13 Mar 2026
 *      Author: smpet
 */

#ifndef LTC6813_H
#define LTC6813_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "stm32h5xx_hal.h"

#define CELL 1
#define AUX 2
#define STAT 3
#define IC_LTC6813

#define MD_422HZ_1KHZ 0
#define MD_27KHZ_14KHZ 1
#define MD_7KHZ_3KHZ 2
#define MD_26HZ_2KHZ 3

#define ADC_OPT_ENABLED 1
#define ADC_OPT_DISABLED 0

#define CELL_CH_ALL 0
#define CELL_CH_1and7 1
#define CELL_CH_2and8 2
#define CELL_CH_3and9 3
#define CELL_CH_4and10 4
#define CELL_CH_5and11 5
#define CELL_CH_6and12 6

#define SELFTEST_1 1
#define SELFTEST_2 2

#define AUX_CH_ALL 0
#define AUX_CH_GPIO1 1
#define AUX_CH_GPIO2 2
#define AUX_CH_GPIO3 3
#define AUX_CH_GPIO4 4
#define AUX_CH_GPIO5 5
#define AUX_CH_VREF2 6

#define STAT_CH_ALL 0
#define STAT_CH_SOC 1
#define STAT_CH_ITEMP 2
#define STAT_CH_VREGA 3
#define STAT_CH_VREGD 4

#define REG_ALL 0
#define REG_1 1
#define REG_2 2
#define REG_3 3
#define REG_4 4
#define REG_5 5
#define REG_6 6

#define DCP_DISABLED 0
#define DCP_ENABLED 1

#define PULL_UP_CURRENT 1
#define PULL_DOWN_CURRENT 0

#define NUM_RX_BYT 8
#define CELL 1
#define AUX 2
#define STAT 3
#define CFGR 0
#define CFGRB 4
#define CS_PIN GPIO_PIN_2
#define CS_PORT GPIOC
#define CELLS_PER_IC 14
#define TEMPS_PER_IC 13

#define TOTAL_IC 10

/*! Cell Voltage data structure. */
typedef struct
{
  uint16_t c_codes[18]; //!< Cell Voltage Codes
  uint8_t pec_match[6]; //!< If a PEC error was detected during most recent read cmd
} cv;

/*! AUX Reg Voltage Data structure */
typedef struct
{
  uint16_t a_codes[9]; //!< Aux Voltage Codes
  uint8_t pec_match[4]; //!< If a PEC error was detected during most recent read cmd
} ax;

/*! Status Reg data structure. */
typedef struct
{
  uint16_t stat_codes[4]; //!< Status codes.
  uint8_t flags[3]; //!< Byte array that contains the uv/ov flag data
  uint8_t mux_fail[1]; //!< Mux self test status flag
  uint8_t thsd[1]; //!< Thermal shutdown status
  uint8_t pec_match[2]; //!< If a PEC error was detected during most recent read cmd
} st;

/*! IC register structure. */
typedef struct
{
  uint8_t tx_data[6];  //!< Stores data to be transmitted
  uint8_t rx_data[8];  //!< Stores received data
  uint8_t rx_pec_match; //!< If a PEC error was detected during most recent read cmd
} ic_register;

/*! PEC error counter structure. */
typedef struct
{
  uint16_t pec_count; //!< Overall PEC error count
  uint16_t cfgr_pec;  //!< Configuration register data PEC error count
  uint16_t cell_pec[6]; //!< Cell voltage register data PEC error count
  uint16_t aux_pec[4];  //!< Aux register data PEC error count
  uint16_t stat_pec[2]; //!< Status register data PEC error count
} pec_counter;

/*! Register configuration structure */
typedef struct
{
  uint8_t cell_channels; //!< Number of Cell channels
  uint8_t stat_channels; //!< Number of Stat channels
  uint8_t aux_channels;  //!< Number of Aux channels
  uint8_t num_cv_reg;    //!< Number of Cell voltage register
  uint8_t num_gpio_reg;  //!< Number of Aux register
  uint8_t num_stat_reg;  //!< Number of  Status register
} register_cfg;

/*! Cell variable structure */
typedef struct
{
  ic_register config;
  ic_register configb;
  cv  cells;
  ax  aux;
  st  stat;
  ic_register com;
  ic_register pwm;
  ic_register pwmb;
  ic_register sctrl;
  ic_register sctrlb;
  uint8_t sid[6];
  bool isospi_reverse;
  pec_counter crc_count;
  register_cfg ic_reg;
  long system_open_wire;
} cell_asic;

// Macros
//! Set "pin" low
//! @param pin pin to be driven LOW
#define output_low(port, pin)   HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET)
//! Set "pin" high
//! @param pin pin to be driven HIGH
#define output_high(port, pin)  HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET)
//! Return the state of pin "pin"
//! @param pin pin to be read (HIGH or LOW).
//! @return the state of pin "pin"
#define input(port, pin)        HAL_GPIO_ReadPin(port, pin)

void cs_low(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);

void cs_high(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);

// void delay_u(uint16_t micro);

void delay_m(uint16_t milli);

// void set_spi_freq();


/*
Writes an array of bytes out of the SPI port
*/
void spi_write_array(SPI_HandleTypeDef *spi,
					 uint16_t len, // Option: Number of bytes to be written on the SPI port
                     uint8_t data[] //Array of bytes to be written on the SPI port
                    );
/*
 Writes and read a set number of bytes using the SPI port.

*/

void spi_write_read(SPI_HandleTypeDef *spi,
					uint8_t *tx_data,//array of data to be written on SPI port
					uint16_t tx_len,
                    uint8_t *rx_data,//Input: array that will store the data read by the SPI port
                    uint16_t rx_len // length of tx and rx arrays (must be the same)
                   );

HAL_StatusTypeDef spi_read_byte(SPI_HandleTypeDef *spi, uint8_t *buf);

HAL_StatusTypeDef spi_read_array(SPI_HandleTypeDef *spi, uint8_t len, uint8_t *buf);

void uart_print(char *str);
void uart_print_hex(uint8_t val);
void uart_print_dec(uint16_t val);
void uart_print_uint(uint32_t val);
/*!
 Wake isoSPI up from IDlE state and enters the READY state
 @return void
 */
void wakeup_idle(uint8_t total_ic);//!< Number of ICs in the daisy chain

/*!
 Wake the LTC681x from the sleep state
 @return void
 */
void wakeup_sleep(uint8_t total_ic); //!< Number of ICs in the daisy chain

/*!
 Sends a command to the BMS IC. This code will calculate the PEC code for the transmitted command
 @return void
 */
void cmd_68(uint8_t tx_cmd[2]); //!< 2 byte array containing the BMS command to be sent

/*!
 Writes an array of data to the daisy chain
 @return void
 */
void write_68(uint8_t total_ic , //!< Number of ICs in the daisy chain
              uint8_t tx_cmd[2], //!< 2 byte array containing the BMS command to be sent
              uint8_t data[] //!< Array containing the data to be written to the BMS ICs
             );

/*!
 Issues a command onto the daisy chain and reads back 6*total_ic data in the rx_data array
 @return int8_t, PEC Status.
  0: Data read back has matching PEC
 -1: Data read back has incorrect PEC
 */
int8_t read_68( uint8_t total_ic, //!< Number of ICs in the daisy chain
                uint8_t tx_cmd[2], //!< 2 byte array containing the BMS command to be sent
                uint8_t *rx_data); //!< Array that the read back data will be stored in.

/*!
 Calculates  and returns the CRC15
 @returns The calculated pec15 as an unsigned int
  */
uint16_t pec15_calc(uint8_t len, //!< The length of the data array being passed to the function
                    uint8_t *data //!< The array of data that the PEC will be generated from
                   );

/*!
 Helper function to initialize register limits
 @return void
 */
void LTC6813_init_reg_limits(uint8_t total_ic, //!< Number of ICs in the system
							cell_asic *ic //!< A two dimensional array that will store the data
							);

/*!
 Write the LTC6813 configuration register A
 @return void
 */
void LTC6813_wrcfg(uint8_t total_ic, //!< Number of ICs in the system
                   cell_asic *ic //!< A two dimensional array of the configuration data that will be written
                   );

/*!
 Write the LTC6813 configuration register B
 @return void
 */
void LTC6813_wrcfgb(uint8_t total_ic, //!< Number of ICs in the system
                   cell_asic *ic //!< A two dimensional array of the configuration data that will be written
                    );

/*!
 Reads configuration register A of a LTC6813 daisy chain
 @return int8_t, PEC Status.
  0: Data read back has matching PEC
 -1: Data read back has incorrect PEC
 */
int8_t LTC6813_rdcfg(uint8_t total_ic, //!< Number of ICs in the system
                     cell_asic *ic //!< A two dimensional array that the function stores the read configuration data
                    );

/*!
 Reads configuration register B of a LTC6813 daisy chain
 @return int8_t, pec_error PEC Status.
  0: Data read back has matching PEC
 -1: Data read back has incorrect PEC
 */
int8_t LTC6813_rdcfgb(uint8_t total_ic, //!< Number of ICs in the system
                     cell_asic *ic //!< A two dimensional array that the function stores the read configuration data
                    );

/*!
 Starts cell voltage conversion
 @return void
 */
void LTC6813_adcv(uint8_t MD, //!< ADC Conversion Mode
                  uint8_t DCP, //!< Controls if Discharge is permitted during conversion
                  uint8_t CH //!< Sets which Cell channels are converted
                 );

/*!
 Start a GPIO and Vref2 Conversion
 @return void
 */
void LTC6813_adax(uint8_t MD, //!< ADC Conversion Mode
				  uint8_t CHG //!< Sets which GPIO channels are converted
                  );

/*!
 Start a Status ADC Conversion
 @return void
 */
void LTC6813_adstat( uint8_t MD, //!< ADC Conversion Mode
					 uint8_t CHST //!< Sets which Stat channels are converted
					);

/*!
 Starts cell voltage  and GPIO 1 & 2 conversion
 @return void
 */
void LTC6813_adcvax(uint8_t MD, //!< ADC Conversion Mode
					uint8_t DCP //!< Controls if Discharge is permitted during conversion
					);

/*! Starts cell voltage and Sum of cells conversion
 @return void
 */
void LTC6813_adcvsc(uint8_t MD, //!< ADC Conversion Mode
					uint8_t DCP //!< Controls if Discharge is permitted during conversion
					);

/*!
 Reads and parses the LTC6813 cell voltage registers.
 @return uint8_t, pec_error PEC Status.
 0: No PEC error detected
 -1: PEC error detected, retry read
 */
uint8_t LTC6813_rdcv(uint8_t reg, //!< Controls which cell voltage register is read back.
                     uint8_t total_ic, //!< The number of ICs in the daisy chain
                     cell_asic *ic //!< Array of the parsed cell codes from lowest to highest.
                    );

/*! Reads and parses the LTC6813 auxiliary registers.
 @return  int8_t, pec_error PEC Status
   0: No PEC error detected
  -1: PEC error detected, retry read
  */
int8_t LTC6813_rdaux(uint8_t reg, //!< Controls which GPIO voltage register is read back
                     uint8_t nIC, //!< The number of ICs in the daisy chain
                     cell_asic *ic //!< A two dimensional array of the parsed gpio voltage codes
                    );

/*!
 Reads and parses the LTC6813 stat registers.
 @return  int8_t, pec_error PEC Status
  0: No PEC error detected
  -1: PEC error detected, retry read
  */
int8_t LTC6813_rdstat(uint8_t reg, //!< Determines which Stat  register is read back.
                      uint8_t total_ic,//!< Number of ICs in the system
                      cell_asic *ic //!< A two dimensional array that will store the data
                     );

/*!
 Reads the raw cell voltage register data
 @return void
 */
void LTC6813_rdcv_reg(uint8_t reg, //!< Determines which cell voltage register is read back
                      uint8_t total_ic, //!< The number of ICs in the
                      uint8_t *data //!< An array of the unparsed cell codes
                     );

/*!
 Read the raw data from the LTC681x auxiliary register
 The function reads a single GPIO voltage register and stores the read data in the *data point as a byte array.
 This function is rarely used outside of the LTC681x_rdaux() command.
 @return void
 */
void LTC6813_rdaux_reg(  uint8_t reg, //!< Determines which GPIO voltage register is read back
                         uint8_t total_ic, //!< The number of ICs in the system
                         uint8_t *data //!< Array of the unparsed auxiliary codes
                      );

/*!
 Helper function that parses voltage measurement registers
 @return int8_t, pec_error PEC Status.
  0: Data read back has matching PEC
 -1: Data read back has incorrect PEC
 */
uint8_t parse_cells(uint8_t ic_idx, uint8_t reg_num, uint8_t *data_ptr, uint16_t *c_codes, uint8_t *pec_match);  // Array of pointers to each IC's PEC flags

/*!
  Sends the poll ADC command
  @returns 1 byte read back after a pladc command. If the byte is not 0xFF ADC conversion has completed
  */
uint8_t LTC6813_pladc();

/*!
  This function will block operation until the ADC has finished it's conversion
  @returns uint32_t, counter The approximate time it took for the ADC function to complete.
  */
uint32_t LTC6813_pollAdc();

/*!
 Clears the LTC6813 cell voltage registers
 @return void
 */
void LTC6813_clrcell();

/*!
 Clears the LTC6813 Auxiliary registers
 @return void
 */
void LTC6813_clraux();

/*!
 Clears the LTC6813 Stat registers
 @return void
 */
void LTC6813_clrstat();

/*!
 Starts the Mux Decoder diagnostic self test
 Running this command will start the Mux Decoder Diagnostic Self Test
 This test takes roughly 1mS to complete. The MUXFAIL bit will be updated,
 the bit will be set to 1 for a failure and 0 if the test has been passed.
 @return void
 */
void LTC6813_diagn();

/*!
 Starts cell voltage self test conversion
 @return void
 */
void LTC6813_cvst(uint8_t MD, //!< ADC Conversion Mode
				  uint8_t ST //!< Self Test Mode
				 );

/*!
 Start an Auxiliary Register Self Test Conversion
 @return void
 */
void LTC6813_axst(uint8_t MD, //!< ADC Conversion Mode
				  uint8_t ST //!< Sets if self test 1 or 2 is run
				 );

/*!
 Start a Status Register Self Test Conversion
 @return void
 */
void LTC6813_statst(uint8_t MD, //!< ADC Conversion Mode
					uint8_t ST //!< Sets if self test 1 or 2 is run
					);

/*!
 Starts cell voltage overlap conversion
 @return void
 */
void LTC6813_adol(uint8_t MD, //!< ADC Conversion Mode
				  uint8_t DCP //!< Discharge permitted during conversion
				 );

/*!
 Start an GPIO Redundancy test
 @return void
 */
void LTC6813_adaxd(uint8_t MD, //!< ADC Conversion Mode
				   uint8_t CHG //!< Sets which GPIO channels are converted
				   );

/*!
 Start a Status register redundancy test Conversion
 @return void
  */
void LTC6813_adstatd(uint8_t MD, //!< ADC Mode
					 uint8_t CHST //!< Sets which Status channels are converted
					);

/*!
 Helper function that runs the ADC Self Tests
 @return int16_t, error Number of errors detected.
 */
int16_t LTC6813_run_cell_adc_st(uint8_t adc_reg, //!< Type of register
                                uint8_t total_ic, //!< Number of ICs in the system
                                cell_asic *ic, //!< A two dimensional array that will store the data
								uint8_t md, //!< ADC Mode
								bool adcopt //!< The adcopt bit in the configuration register
								);

/*!
 Helper Function that runs the ADC Overlap test
 @return uint16_t, error
  0: Pass
 -1: False, Error detected
 */
uint16_t LTC6813_run_adc_overlap(uint8_t total_ic, //!< Number of ICs in the system
                                 cell_asic *ic //!< A two dimensional array that will store the data
								 );

/*!
 Helper function that runs the ADC Digital Redundancy commands and checks output for errors
 @return int16_t, error Number of errors detected.
 */
int16_t LTC6813_run_adc_redundancy_st(uint8_t adc_mode, //!< ADC Mode
                                      uint8_t adc_reg, //!< Type of register
                                      uint8_t total_ic, //!< Number of ICs in the system
                                      cell_asic *ic //!< A two dimensional array that will store the data
									  );

/*!
 Start an open wire Conversion
 @return void
 */
void LTC6813_adow(uint8_t MD, //!< ADC Conversion Mode
				  uint8_t PUP,//!< Pull up/Pull down current
				  uint8_t CH, //!< Sets which Cell channels are converted
				  uint8_t DCP //!< Discharge permitted during conversion
				 );

/*!
 Start GPIOs open wire ADC conversion
 @return void
 */
void LTC6813_axow(uint8_t MD, //!< ADC Mode
				  uint8_t PUP //!< Pull up/Pull down current
				  );

/*!
 Helper function that runs the data sheet algorithm for open wire for single cell detection
 @return void
 */
void LTC6813_run_openwire_single(uint8_t total_ic, //!< Number of ICs in the system
								 cell_asic *ic //!< A two dimensional array that will store the data
								 );

/*!
 Helper function that runs open wire for multiple cell and two consecutive cells detection
 @return void
 */
void LTC6813_run_openwire_multi(uint8_t total_ic, //!< Number of ICs in the system
								cell_asic *ic //!< A two dimensional array that will store the data
								);

/*!
 Runs open wire for GPIOs
 @return void
 */
void LTC6813_run_gpio_openwire(uint8_t total_ic, //!< Number of ICs in the system
								cell_asic *ic //!< A two dimensional array that will store the data
								);

/*!
 Helper Function to Set DCC bits in the CFGR Registers
 @return void
 */
void LTC6813_set_discharge(int Cell, //!< The cell to be discharged
                           uint8_t total_ic, //!< Number of ICs in the system
                           cell_asic *ic //!< A two dimensional array that will store the data
						   );

/*!
 Helper Function to clear DCC bits in the CFGR Registers
 @return void
 */
void LTC6813_clear_discharge(uint8_t total_ic, //!< Number of ICs in the system
							 cell_asic *ic //!< A two dimensional array that will store the data
							 );

/*!
 Write the LTC6813 PWM register
 @return void
 */
void LTC6813_wrpwm(uint8_t total_ic, //!< Number of ICs in the daisy chain
                   uint8_t pwmReg, //!<  PWM  Register A or B
                   cell_asic *ic //!< A two dimensional array that will store the data
                  );

/*!
 Reads pwm registers of a LTC6813 daisy chain
 @return int8_t, pec_error PEC Status.
  0: Data read back has matching PEC
 -1: Data read back has incorrect PEC
  */
int8_t LTC6813_rdpwm(uint8_t total_ic, //!< Number of ICs in the daisy chain
                     uint8_t pwmReg, //!< PWM  Register A or B
                     cell_asic *ic //!< A two dimensional array that will store the data
                    );

/*!
 Write the LTC6813 Sctrl register
 @return void
 */
void LTC6813_wrsctrl(uint8_t nIC, //!< Number of ICs in the daisy chain
                     uint8_t sctrl_reg,//! SCTRL  Register A or B
                     cell_asic *ic //!< A two dimensional array that will store the data
                    );

/*!
 Reads sctrl registers of a LTC6813 daisy chain
 @return int8_t, pec_error PEC Status.
   0: Data read back has matching PEC
   -1: Data read back has incorrect PEC
   */
int8_t LTC6813_rdsctrl(uint8_t nIC, //!< Number of ICs in the daisy chain
                       uint8_t sctrl_reg,//!< SCTRL Register A or B
                       cell_asic *ic  //!< A two dimensional array that will store the data
                      );

/*!
 Start Sctrl data communication
 This command will start the sctrl pulse communication over the spins
 @return void
 */
void LTC6813_stsctrl();

/*!
 Clears the LTC6813 Sctrl registers
 @return void
 */
void LTC6813_clrsctrl();

/*!
 Write the 6813 PWM/Sctrl Register B
 @return void
 */
void LTC6813_wrpsb(uint8_t total_ic, //!< Number of ICs in the system
					cell_asic *ic //!< A two dimensional array that will store the data
					);

/*!
 Reading pwm/s control register B
 @return uint8_t, pec_error PEC Status.
   0: Data read back has matching PEC
  -1: Data read back has incorrect PEC
  */
uint8_t LTC6813_rdpsb(uint8_t total_ic, //!< Number of ICs in the daisy chain
                       cell_asic *ic //!< A two dimensional array that the function stores the read data
                      );

/*!
 Write the LTC6813 COMM register
 @return void
 */
void LTC6813_wrcomm(uint8_t total_ic, //!< Number of ICs in the daisy chain
                    cell_asic *ic //!< A two dimensional array of the comm data that will be written
                   );

/*!
 Reads comm registers of a LTC6813 daisy chain
 @return int8_t, pec_error PEC Status.
   0: Data read back has matching PEC
  -1: Data read back has incorrect PEC
  */
int8_t LTC6813_rdcomm(uint8_t total_ic, //!< Number of ICs in the daisy chain
                      cell_asic *ic //!< Two dimensional array that the function stores the read comm data.
                     );

/*!
 Issues a stcomm command and clocks data out of the COMM register
 @return void
 */
void LTC6813_stcomm(uint8_t len //!< Length of data to be transmitted
					);

/*!
 Mutes the LTC6813 discharge transistors
 @return void
 */
void LTC6813_mute();

/*!
 Clears the LTC6813 Mute Discharge
 @return void
 */
void LTC6813_unmute();

/*!
 Helper Function that counts overall PEC errors and register/IC PEC errors
 @return void
 */
void LTC6813_check_pec(uint8_t total_ic, //!< Number of ICs in the system
                       uint8_t reg, //!<  Type of register
                       cell_asic *ic //!< A two dimensional array that will store the data
					   );

/*!
 Helper Function that resets the PEC error counters
 @return void
 */
void LTC6813_reset_crc_count(uint8_t total_ic, //!< Number of ICs in the system
                             cell_asic *ic //!< A two dimensional array that will store the data
							 );

/*!
 Helper Function to initialize the CFGR data structures
 @return void
 */
void LTC6813_init_cfg(uint8_t total_ic, //!< Number of ICs in the system
                      cell_asic *ic //!< A two dimensional array that will store the data
					  );

/*!
 Helper function to set appropriate bits in CFGR register based on bit function
 @return void
 */
void LTC6813_set_cfgr(uint8_t nIC,  //!< The number of ICs in the daisy chain
                      cell_asic *ic, //!< A two dimensional array that will store the data
                      bool refon, //!< The REFON bit
                      bool adcopt, //!< The ADCOPT bit
                      bool gpio[5], //!< The GPIO bits
                      bool dcc[12], //!< The DCC bits
					  bool dcto[4], //!< The Dcto bits
					  uint16_t uv, //!< The UV value
					  uint16_t  ov //!< The OV value
					  );

/*!
 Helper function to turn the REFON bit HIGH or LOW
 @return void
 */
void LTC6813_set_cfgr_refon(uint8_t nIC, //!< The number of ICs in the daisy chain
                            cell_asic *ic, //!< A two dimensional array that will store the data
                            bool refon //!< The REFON bit
							);

/*!
 Helper function to turn the ADCOPT bit HIGH or LOW
 @return void
 */
void LTC6813_set_cfgr_adcopt(uint8_t nIC, //!< The number of ICs in the daisy chain
                             cell_asic *ic, //!< A two dimensional array that will store the data
                             bool adcopt //!< The ADCOPT bit
							 );

/*!
 Helper function to turn the GPIO bits HIGH or LOW
 @return void
 */
void LTC6813_set_cfgr_gpio(uint8_t nIC, //!< The number of ICs in the daisy chain
                           cell_asic *ic, //!< A two dimensional array that will store the data
                           bool gpio[] //!< The GPIO bits
						   );

/*!
 Helper function to turn the DCC bits HIGH or LOW
 @return void
 */
void LTC6813_set_cfgr_dis(uint8_t nIC, //!< The number of ICs in the daisy chain
                          cell_asic *ic, //!< A two dimensional array that will store the data
                          bool dcc[] //!< The DCC bits
						  );

/*!
 Helper function to set UV field in CFGRA register
 @return void
 */
void LTC6813_set_cfgr_uv(uint8_t nIC, //!< The number of ICs in the daisy chain
                         cell_asic *ic, //!< A two dimensional array that will store the data
                         uint16_t uv //!< The UV value
						 );

/*!
 Helper function to set DCTO  field in CFGRA register
 @return void
 */
void LTC6813_set_cfgr_dcto(uint8_t nIC, //!< The number of ICs in the daisy chain
                         cell_asic *ic, //!< A two dimensional array that will store the data
                         bool dcto[4] //!< The Dcto bits
						 );

/*!
 Helper function to set OV field in CFGRA register
 @return void
 */
void LTC6813_set_cfgr_ov(uint8_t nIC, //!< The number of ICs in the daisy chain
                         cell_asic *ic, //!< A two dimensional array that will store the data
                         uint16_t ov //!< The OV value
						 );

/*!
 Helper Function to initialize the CFGR B data structures
 @return void
 */
void LTC6813_init_cfgb(uint8_t total_ic, //!< Number of ICs in the system
                      cell_asic *ic //!< A two dimensional array that will store the data
					  );

/*!
 Helper function to set appropriate bits in CFGR register based on bit function
 @return void
 */
void LTC6813_set_cfgrb(uint8_t nIC, //!< The number of ICs in the daisy chain
                      cell_asic *ic, //!< A two dimensional array that will store the data
					  bool fdrf, //!< The FDRF bit
                      bool dtmen, //!< The DTMEN bit
                      bool ps[2], //!< Path selection bits
                      bool gpiobits[4], //!< The GPIO bits
					  bool dccbits[7] //!< The DCC bits
					  );

/*!
 Helper function to turn the FDRF bit HIGH or LOW
 @return void
 */
void LTC6813_set_cfgrb_fdrf(uint8_t nIC, //!< The number of ICs in the daisy chain
                            cell_asic *ic, //!< A two dimensional array that will store the data
                            bool fdrf //!< The FDRF bit
							);

/*!
 Helper function to turn the DTMEN bit HIGH or LOW
 @return void
 */
void LTC6813_set_cfgrb_dtmen(uint8_t nIC, //!< The number of ICs in the daisy chain
                            cell_asic *ic, //!< A two dimensional array that will store the data
                            bool dtmen //!< The DTMEN bit
							);

/*!
 Helper function to turn the Path Select bit HIGH or LOW
 @return void
 */
void LTC6813_set_cfgrb_ps(uint8_t nIC, //!< The number of ICs in the daisy chain
                            cell_asic *ic, //!< A two dimensional array that will store the data
                            bool ps[] //!< Path selection bits
							);

/*!
 Helper function to turn the GPIO bit HIGH or LOW
 @return void
 */
void LTC6813_set_cfgrb_gpio_b(uint8_t nIC, //!< The number of ICs in the daisy chain
                            cell_asic *ic, //!< A two dimensional array that will store the data
                            bool gpiobits[] //!< The GPIO bits
							);

/*!
 Helper function to turn the DCC bit HIGH or LOW
 @return void
 */
void LTC6813_set_cfgrb_dcc_b(uint8_t nIC, //!< The number of ICs in The daisy chain
                            cell_asic *ic, //!< A two dimensional array that will store The data
                            bool dccbits[] //!< The DCC bits
							);

extern void Error_Handler(void);

extern const uint16_t crc15Table[256];


#endif
