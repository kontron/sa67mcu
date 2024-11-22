// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2025 Kontron Europe GmbH
 */

#include <stdbool.h>
#include "adc.h"
#include "uart.h"
#include "systick.h"
#include "vref.h"
#include "misc.h"

#define IOMUX_BASE		0x40428000
#define IOMUX_PINCM(n)		(IOMUX_BASE + 4 * (n))

#define PINCM24_PF_UART0_TX	0x2
#define PINCM_PC		BIT(7)

static void iomux_init(void)
{
	iow(IOMUX_PINCM(24), PINCM24_PF_UART0_TX | PINCM_PC); /* PA23 UART0_TX */
}

#define SYSCTL_BASE		0x400af000
#define SYSCTL_MCLKCFG		(SYSCTL_BASE + 0x1104)
#define MCLKCFG_USEMFTICK	BIT(12)
#define MCLKCFG_MDIV_DIV_BY(n)	((n)-1)
static void clocks_init(void)
{
	/* enable MFCLK (4MHz) */
	iow(SYSCTL_MCLKCFG, MCLKCFG_USEMFTICK);
}

int main(void)
{
	clocks_init();
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
