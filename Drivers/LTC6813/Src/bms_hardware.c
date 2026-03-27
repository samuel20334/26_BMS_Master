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

extern UART_HandleTypeDef huart1;
extern TIM_HandleTypeDef htim2;

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
