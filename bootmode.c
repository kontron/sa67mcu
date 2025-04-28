// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2025 Kontron Europe GmbH
 */

#include "bootmode.h"
#include "config.h"
#include "gpio.h"
#include "misc.h"

#define BOOTMODE_PIN_FAILSAFE	0x0000000c
#define BOOTMODE_PIN_MASK	0x00c007fc

static unsigned short bootmode;

void bootmode_enable(unsigned short mode)
{
	unsigned int mask;

	printf("%s mode=%04x\n", __func__, mode);

	gpio_mask_clr(BOOTMODE_PIN_MASK);
	/* PA8 .. PA2 */
	mask  = ((unsigned int)mode >>  1) & 0x000001fc;
	/* PA23 */
	mask |= ((unsigned int)mode << 13) & 0x00800000;
	/* PA22 */
	mask |= ((unsigned int)mode << 11) & 0x00400000;
	/* PA10 .. PA9 */
	mask |= ((unsigned int)mode >>  3) & 0x00000600;
	gpio_mask_set(mask);
	gpio_out_mask(BOOTMODE_PIN_MASK);
}

void bootmode_write(unsigned char offset, unsigned char value)
{
	switch (offset) {
	case 0:
		bootmode = (bootmode & 0x00ff) | (value << 8);
		break;
	case 1:
		bootmode = (bootmode & 0xff00) | value;
		bootmode_enable(bootmode);
		break;
	}
}

unsigned char bootmode_read(unsigned char offset)
{
	switch (offset) {
	case 0:
		return bootmode & 0xff;
	case 1:
		return bootmode >> 8;
	default:
		return 0xff;
	}
}

void bootmode_disable(void)
{
	gpio_in_mask(BOOTMODE_PIN_MASK);
}

void bootmode_init(void)
{
	if (config->flags & CFG_F_DRIVE_BOOTMODE)
		bootmode_enable(config->bootmode);
	else
		bootmode_disable();
}
