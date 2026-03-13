/*
 * bms_hardware.c
 *
 *  Created on: 13 Mar 2026
 *      Author: smpet
 */

#include <stdint.h>
#include "bms_hardware.h"
#include "Linduino.h"
#include "stm32h5xx_hal.h"

void cs_low(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
  output_low(GPIOx, GPIO_Pin);
}

void cs_high(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
  output_high(GPIOx, GPIO_Pin);
}

/* void delay_u(uint16_t micro)
{
  delayMicroseconds(micro);
} */

void delay_m(uint16_t milli)
{
  HAL_Delay(milli);
}

/*
Writes an array of bytes out of the SPI port
*/
void spi_write_array(SPI_HandleTypeDef *spi,
					 uint8_t len, // Option: Number of bytes to be written on the SPI port
                     uint8_t data[] //Array of bytes to be written on the SPI port
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
                    uint8_t *rx_data,//Input: array that will store the data read by the SPI port
                    uint16_t size // length of tx and rx arrays (must be the same)
                   )
{
	HAL_StatusTypeDef status = HAL_SPI_TransmitReceive(spi, tx_data, rx_data, size, 10);

	if (status != HAL_OK)
	{
		Error_Handler();
	}
}


HAL_StatusTypeDef spi_read_byte(SPI_HandleTypeDef *spi, uint8_t *buf)
{
    return HAL_SPI_Receive(spi, buf, 1, 10);
}

HAL_StatusTypeDef spi_read_array(SPI_HandleTypeDef *spi, uint8_t len, uint8_t *buf) {
    return HAL_SPI_Receive(spi, buf, len, 10);
}
