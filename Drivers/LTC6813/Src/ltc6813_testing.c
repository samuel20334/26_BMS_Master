#include "stm32h5xx_hal.h" // or your STM32 HAL header
#include <stdint.h>
#include <stdlib.h>
#include "ltc6813.h"
#include "ltc681x.h"

// External UART handle
extern UART_HandleTypeDef huart1;

#define TOTAL_IC 1  // Test a single IC
#define UART_TIMEOUT 100

// Helper function to send string over UART}

int test(void)
{
    HAL_Init();
    // Make sure SystemClock_Config() and MX_USART1_UART_Init() have been called in main

    cell_asic ic[TOTAL_IC];
    uint8_t cell_data[8];        // 6 data bytes + 2 PEC bytes
    uint16_t cell_codes[3];      // 3 cell codes per register
    uint8_t ic_pec[1];           // PEC status for register

    ic[0].isospi_reverse = false;

    // Read first cell voltage register
    LTC681x_rdcv_reg(1, TOTAL_IC, cell_data);

    uart_print("Raw SPI data:\r\n");
    for (int i = 0; i < 8; i++)
        uart_print_hex(cell_data[i]);
    uart_print("\r\n");

    // Parse and check PEC
    int8_t pec_error = parse_cells(0, 1, cell_data, cell_codes, ic_pec);

    uart_print("Parsed cell codes:\r\n");
    for (int i = 0; i < 3; i++)
    {
        uart_print("Cell ");
        char buf[3];
        snprintf(buf, sizeof(buf), "%d: ", i + 1);
        uart_print(buf);
        uart_print_dec(cell_codes[i]);
        uart_print("\r\n");
    }

    if (pec_error)
    {
        uart_print("PEC error detected! ic_pec[0] = ");
        char buf[3];
        snprintf(buf, sizeof(buf), "%d\r\n", ic_pec[0]);
        uart_print(buf);
    }
    else
        uart_print("PEC OK!\r\n");

    // Manual PEC calculation
    uint16_t calc_pec = pec15_calc(6, cell_data);          // Only 6 data bytes
    uint16_t received_pec = (cell_data[6] << 8) | cell_data[7];

    uart_print("Received PEC: 0x");
    char buf[5];
    snprintf(buf, sizeof(buf), "%04X\r\n", received_pec);
    uart_print(buf);

    uart_print("Calculated PEC: 0x");
    snprintf(buf, sizeof(buf), "%04X\r\n", calc_pec);
    uart_print(buf);

    if (received_pec == calc_pec)
        uart_print("Manual PEC check PASSED\r\n");
    else
        uart_print("Manual PEC check FAILED\r\n");

    while (1)
    {
        HAL_Delay(1000); // Just loop
    }
}
