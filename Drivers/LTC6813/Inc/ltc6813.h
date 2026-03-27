/*
 * ltc6813.h
 *
 *  Created on: 13 Mar 2026
 *      Author: smpet
 */

#ifndef LTC6813_H
#define LTC6813_H

#include <stdint.h>
#include <stdbool.h>
#include "ltc681x.h"

#define CELL 1
#define AUX 2
#define STAT 3

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


#endif
