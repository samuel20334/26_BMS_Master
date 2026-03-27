/*
 * ltc6813_testing.h
 *
 *  Created on: 14 Mar 2026
 *      Author: smpet
 */

#ifndef LTC6813_LTC6813_TESTING_H_
#define LTC6813_LTC6813_TESTING_H_

#include "ltc681x.h"
#include "ltc6813.h"
#include <stdio.h>
#include <stdbool.h>

int test_cfg_rw(uint8_t total_ic, cell_asic *ic);
int test_cell_adc(uint8_t total_ic, cell_asic *ic);
int test_aux_adc(uint8_t total_ic, cell_asic *ic);
int test_status(uint8_t total_ic, cell_asic *ic);
int test_adc_selftest(uint8_t total_ic, cell_asic *ic);
int test_adc_overlap(uint8_t total_ic, cell_asic *ic);
int test_openwire(uint8_t total_ic, cell_asic *ic);
int test_discharge(uint8_t total_ic, cell_asic *ic);
void run_all_tests(uint8_t total_ic, cell_asic *ic);

#endif /* LTC6813_LTC6813_TESTING_H_ */
