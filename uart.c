// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2025 Kontron Europe GmbH
 */

#include "misc.h"

/* UART0 */
#define UART_BASE 0x40108000

#define UART_PWREN		(UART_BASE + 0x0800)
#define PWREN_KEY		0x26000000
#define PWREN_ENABLE		BIT(0)

#define UART_CLKSEL		(UART_BASE + 0x1008)
#define CLKSEL_MFCLK_SEL	BIT(2)

#define UART_CTL0		(UART_BASE + 0x1100)
#define CTL0_HSE_3X		(2 << 15)
#define CTL0_TXE		BIT(4)
#define CTL0_ENABLE		BIT(0)
#define UART_LCRH		(UART_BASE + 0x1104)
#define LCRH_WLEN(n)		(((n)-5) << 4)
#define UART_STAT		(UART_BASE + 0x1108)
#define STAT_TXFF		BIT(7)

#define UART_IBRD		(UART_BASE + 0x1110)
#define UART_FBRD		(UART_BASE + 0x1114)
#define UART_TXDATA		(UART_BASE + 0x1120)

#ifndef CFG_UART_BAUDRATE
#define CFG_UART_BAUDRATE 115200
#endif

/* clock is MFCLK which is always 4MHz */
#define UART_BASE_CLK 4000000

/* we have a 3x oversampling */
#define UART_BAUDRATE_DIV ((float)(UART_BASE_CLK) / 3.0f / CFG_UART_BAUDRATE)
#define UART_IBRD_VAL ((int)UART_BAUDRATE_DIV)
#define UART_FBRD_VAL ((int)((UART_BAUDRATE_DIV - UART_IBRD_VAL) * 64 + 0.5f))

void uart_init(void)
{
	/* enable power */
	iow(UART_PWREN, PWREN_KEY | PWREN_ENABLE);

	/* set clock */
	iow(UART_CLKSEL, CLKSEL_MFCLK_SEL);

	/* set baudrate, after changing IBRD, LCRH must be written */
	iow(UART_IBRD, UART_IBRD_VAL);
	iow(UART_FBRD, UART_FBRD_VAL);

	iow(UART_LCRH, LCRH_WLEN(8));

	/* enable transmitter */
	iow(UART_CTL0, CTL0_TXE | CTL0_HSE_3X | CTL0_ENABLE);
}

void uart_putc(char c)
{
	if (c == '\n')
		uart_putc('\r');

	while (ior(UART_STAT) & STAT_TXFF);
	iow(UART_TXDATA, c);
}
