// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2025 Kontron Europe GmbH
 */

#include "misc.h"
#include "gpio.h"

#define GPIO_BASE		0x400a0000
#define GPIO_PWREN		(GPIO_BASE + 0x0800)
#define PWREN_KEY		0x26000000
#define PWREN_ENABLE		BIT(0)

#define GPIO_DOUTSET31_0	(GPIO_BASE + 0x1290)
#define GPIO_DOUTCLR31_0	(GPIO_BASE + 0x12a0)
#define GPIO_DOUTTGL31_0	(GPIO_BASE + 0x12b0)
#define GPIO_DOESET31_0		(GPIO_BASE + 0x12d0)
#define GPIO_DOECLR31_0		(GPIO_BASE + 0x12e0)
#define GPIO_DIN31_0		(GPIO_BASE + 0x1380)

void gpio_init(void)
{
	iow(GPIO_PWREN, PWREN_KEY | PWREN_ENABLE);
}

void gpio_out(int pin)
{
	iow(GPIO_DOESET31_0, 1 << pin);
}

void gpio_in(int pin)
{
	iow(GPIO_DOECLR31_0, 1 << pin);
}

bool gpio_get(int pin)
{
	return ior(GPIO_DIN31_0) & (1 << pin);
}

void gpio_set(int pin, bool value)
{
	if (value)
		iow(GPIO_DOUTSET31_0, 1 << pin);
	else
		iow(GPIO_DOUTCLR31_0, 1 << pin);
}

void gpio_toggle(int pin)
{
	iow(GPIO_DOUTTGL31_0, 1 << pin);
}

void gpio_mask_set(unsigned int mask)
{
	iow(GPIO_DOUTSET31_0, mask);
}

void gpio_mask_clr(unsigned int mask)
{
	iow(GPIO_DOUTCLR31_0, mask);
}

void gpio_mask_toggle(unsigned int mask)
{
	iow(GPIO_DOUTTGL31_0, mask);
}