// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2025 Kontron Europe GmbH
 *
 * This is a sl28 compatible watchdog implementation to mimic the same behavior.
 * This way, the orignal linux driver can be reused.
 *
 * Emulate the watchdog by using a timer. The original watchdog runs with a
 * period of 1s. Because we cannot run a timer with such a low frequency, we
 * misuse the lower 8 bit of the counter as a third prescaler.
 */

#include <stdbool.h>
//#include "bootmode.h"
#include "misc.h"
#include "sl28wdt.h"

#define TIMG1_BASE		0x40086000

#define TIM_PWREN		(TIMG1_BASE + 0x0800)
#define PWREN_KEY		0x26000000
#define PWREN_ENABLE		BIT(0)

#define TIM_CLKDIV		(TIMG1_BASE + 0x1000)
#define CLKDIV_DIV_BY(n)	((n) - 1)

#define TIM_CLKSEL		(TIMG1_BASE + 0x1008)
#define CLKSEL_LFCLK		BIT(1)

#define TIM_CCPD		(TIMG1_BASE + 0x1100)
#define CCPD_C0CCP1_OUTPUT	BIT(1)

#define TIM_CCLKCTL		(TIMG1_BASE + 0x1108)
#define CCLKCTL_CLKEN		BIT(0)

#define TIM_CPS			(TIMG1_BASE + 0x110c)
#define CPS_DIV_BY(n)		((n) - 1)

#define TIM_CTR			(TIMG1_BASE + 0x1800)

#define TIM_CTRCTL		(TIMG1_BASE + 0x1804)
#define CTRCTL_EN		BIT(0)
#define CTRCTL_REPEAT		(1 << 1)
#define CTRCTL_CVAE_UNCHANGED	(1 << 28)

#define TIM_LOAD		(TIMG1_BASE + 0x1808)

#define TIM_OCTL0		(TIMG1_BASE + 0x1850)
#define OCTL_CCPINV		BIT(4)

#define TIM_CCACT0		(TIMG1_BASE + 0x1870)
#define CCACT_ZACT_LOW		(2 << 0)

#define LFCLK_FREQ		32768

#define REG_CTRL	0
#define CTRL_EN0	BIT(0)
#define CTRL_EN1	BIT(1)
#define CTRL_LOCKED	BIT(2)
#define CTRL_OE0	BIT(6)
#define CTRL_OE1	BIT(7)
#define REG_TOUT	1
#define REG_KICK	2
#define REG_CNT		3
#define KICK_VALUE	0x6b

#define DEFAULT_TOUT	8

static unsigned char ctrl;

static void sl28_wdt_enable_output(void)
{
	iow(TIM_CCPD, CCPD_C0CCP1_OUTPUT);
}

static void sl28_wdt_disable_output(void)
{
	iow(TIM_CCPD, 0);
}

static void sl28_wdt_enable(void)
{
	iow(TIM_CTRCTL, CTRCTL_CVAE_UNCHANGED | CTRCTL_EN);
}

static void sl28_wdt_disable(void)
{
	iow(TIM_CTRCTL, CTRCTL_CVAE_UNCHANGED);
}

void timg1_irq(void)
{
	printf("%s\n", __func__);
	if (ctrl & CTRL_EN1)
		/* bootmode_enable_failsafe()*/;
	if (ctrl & CTRL_OE0)
		/* issue cpu reset */;
}

unsigned char sl28wdt_read(unsigned char offset)
{
	switch (offset) {
	case REG_CTRL:
		return ctrl;
	case REG_TOUT:
		return ior(TIM_LOAD) >> 8;
	case REG_CNT:
		return ior(TIM_CTR) >> 8;
	default:
		return 0;
	}
}

void sl28wdt_reset(void)
{
	/* bootmode_disable_failsafe() */;

	/* default value for the counter register */
	iow(TIM_LOAD, DEFAULT_TOUT << 8);
	iow(TIM_CTR, DEFAULT_TOUT << 8);
}

void sl28wdt_write(unsigned char offset, unsigned char value)
{
	switch (offset) {
	case REG_CTRL:
		/* disallow further writes */
		if (ctrl & CTRL_LOCKED)
			return;

		if (value & (CTRL_EN0 | CTRL_EN1))
			sl28_wdt_enable();
		else
			sl28_wdt_disable();

		if (value & CTRL_OE1)
			sl28_wdt_enable_output();
		else
			sl28_wdt_disable_output();

		ctrl = value;
		break;
	case REG_TOUT:
		iow(TIM_LOAD, value << 8);
		break;
	case REG_KICK:
		if (value == KICK_VALUE && ior(TIM_CTRCTL) & CTRCTL_EN)
			iow(TIM_CTR, ior(TIM_LOAD));
		break;
	}
}

void sl28wdt_init(void)
{
	iow(TIM_PWREN, PWREN_KEY | PWREN_ENABLE);

	/* use 32kHz low frequency clock */
	iow(TIM_CLKSEL, CLKSEL_LFCLK);

	/* (first) prescale by /8 */
	iow(TIM_CLKDIV, CLKDIV_DIV_BY(8));

	/* (second) prescale by 16 */
	iow(TIM_CPS, CPS_DIV_BY(16));

	/* ungate the clock */
	iow(TIM_CCLKCTL, CCLKCTL_CLKEN);

	/* set output low on zero */
	iow(TIM_CCACT0, CCACT_ZACT_LOW);

	/* default value for the output is high */
	iow(TIM_OCTL0, OCTL_CCPINV);

	sl28wdt_reset();
}
