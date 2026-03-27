/*
 * bms_hardware.h
 *
 *  Created on: 13 Mar 2026
 *      Author: smpet
 */

#ifndef BMSHARDWARE_H
#define BMSHARDWARE_H




#include <stdint.h>
#include "stm32h5xx_hal.h"


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


#endif
