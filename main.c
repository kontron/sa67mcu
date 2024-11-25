// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2025 Kontron Europe GmbH
 */

#include <stdbool.h>
#include "adc.h"
#include "iomux.h"
#include "misc.h"
#include "sysctl.h"
#include "systick.h"
#include "uart.h"
#include "vref.h"

int main(void)
{
	clocks_init();
	iomux_init();
	systick_init();
	iomux_init();
	uart_init();
	vref_init();
	adc_init();

	printf("Hello World\n");
	while (true) {
		udelay(250000);
		printf("AIN0 %d\n", adc_value(0));
		printf("AIN8 %d\n", adc_value(1));
		printf("AT %d\n", adc_value(2));
		unsigned int temp = adc_temperature();
		printf("T %d.%03d\n", temp / 1000, temp % 1000);
	}
}
