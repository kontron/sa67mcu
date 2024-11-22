// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2025 Kontron Europe GmbH
 */

#ifndef __ADC_H
#define __ADC_H

#define ADC_VREF 1400

void adc_init(void);
unsigned int adc_value(unsigned int chan);
int adc_temperature(void);

#endif /* #ifndef __ADC_H */
