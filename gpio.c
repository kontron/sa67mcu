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

#define GPIO_IIDX		(GPIO_BASE + 0x1020)
#define GPIO_IMASK		(GPIO_BASE + 0x1028)
#define GPIO_ICLR		(GPIO_BASE + 0x1048)
#define GPIO_DOUTSET31_0	(GPIO_BASE + 0x1290)
#define GPIO_DOUTCLR31_0	(GPIO_BASE + 0x12a0)
#define GPIO_DOUTTGL31_0	(GPIO_BASE + 0x12b0)
#define GPIO_DOESET31_0		(GPIO_BASE + 0x12d0)
#define GPIO_DOECLR31_0		(GPIO_BASE + 0x12e0)
#define GPIO_DIN31_0		(GPIO_BASE + 0x1380)
#define GPIO_POLARITY15_0	(GPIO_BASE + 0x1390)
#define GPIO_POLARITY31_16	(GPIO_BASE + 0x13a0)

void gpio_init(void)
{
	iow(GPIO_PWREN, PWREN_KEY | PWREN_ENABLE);
}

void gpio_out(int pin)
{
	iow(GPIO_DOESET31_0, 1 << pin);
}

void gpio_out_mask(unsigned int mask)
{
	iow(GPIO_DOESET31_0, mask);
}

void gpio_in(int pin)
{
	iow(GPIO_DOECLR31_0, 1 << pin);
}

void gpio_in_mask(unsigned int mask)
{
	iow(GPIO_DOECLR31_0, mask);
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

void gpio_conf_irq(int pin, int mode)
{
	unsigned int reg = GPIO_POLARITY15_0;
	unsigned int val;

	if (pin >= 16) {
		reg = GPIO_POLARITY31_16;
		pin -= 16;
	}

	val = ior(reg);
	val &= ~(3 << (pin << 1));
	val |= mode << (pin << 1);

	iow(reg, val);
}

void gpio_irq_ack(int pin)
{
	iow(GPIO_ICLR, (1 << pin));
}

void gpio_irq_unmask(int pin)
{
	iow(GPIO_IMASK, ior(GPIO_IMASK) | (1 << pin));
}

void gpio_irq_mask(int pin)
{
	iow(GPIO_IMASK, ior(GPIO_IMASK) & ~(1 << pin));
}

int gpio_irq_pin(void)
{
	return ior(GPIO_IIDX) - 1;
}
