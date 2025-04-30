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
 *
 * Also, the TIMGx_Cy outputs don't really support one-shot mode because
 * once the enable bit is cleared (which is the case when the timer expires
 * in one-shot mode) the output is muxed to the static value OCTL0.CCPIV.
 * To work around that, we don't use the output generation logic and just
 * write the desired value to CCPIV which will be active once the timer
 * expires.
 */

#include <stdbool.h>
#include "board.h"
#include "config.h"
#include "misc.h"
#include "nvic.h"
#include "sl28wdt.h"

#define TIMG1_BASE		0x40086000
#define TIMG1_IRQ		2

#define TIM_PWREN		(TIMG1_BASE + 0x0800)
#define PWREN_KEY		0x26000000
#define PWREN_ENABLE		BIT(0)

#define TIM_CLKDIV		(TIMG1_BASE + 0x1000)
#define CLKDIV_DIV_BY(n)	((n) - 1)

#define TIM_CLKSEL		(TIMG1_BASE + 0x1008)
#define CLKSEL_LFCLK		BIT(1)

#define TIM_IMASK		(TIMG1_BASE + 0x1028)
#define INT_Z			BIT(0)

#define TIM_CCPD		(TIMG1_BASE + 0x1100)
#define CCPD_C0CCP0_OUTPUT	BIT(0)

#define TIM_CCLKCTL		(TIMG1_BASE + 0x1108)
#define CCLKCTL_CLKEN		BIT(0)

#define TIM_CPS			(TIMG1_BASE + 0x110c)
#define CPS_DIV_BY(n)		((n) - 1)

#define TIM_CTR			(TIMG1_BASE + 0x1800)

#define TIM_CTRCTL		(TIMG1_BASE + 0x1804)
#define CTRCTL_EN		BIT(0)

#define TIM_LOAD		(TIMG1_BASE + 0x1808)

#define TIM_OCTL0		(TIMG1_BASE + 0x1850)
#define OCTL_CCPIV		BIT(5)

#define TIM_CCACT0		(TIMG1_BASE + 0x1870)
#define CCACT_SWFRCACT_HIGH	(1 << 28)

/* emulated registers */
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

#define DEFAULT_TOUT	10 /* in seconds */

static volatile unsigned char ctrl;

static void sl28wdt_enable_output(void)
{
	iow(TIM_CCPD, CCPD_C0CCP0_OUTPUT);
}

static void sl28wdt_disable_output(void)
{
	iow(TIM_CCPD, 0);
}

static void sl28wdt_enable(void)
{
	iow(TIM_CTRCTL, CTRCTL_EN);

	/*
	 * Clear the CCPIV bit. If the timer expires, the line will be pulled
	 * low.
	 */
	iow(TIM_OCTL0, 0);
}

static void sl28wdt_disable(void)
{
	/* Set the CCPIV bit again. */
	iow(TIM_OCTL0, OCTL_CCPIV);

	iow(TIM_CTRCTL, 0);
}

static void sl28wdt_ctrl_update(void)
{
	if (ctrl & (CTRL_EN0 | CTRL_EN1))
		sl28wdt_enable();
	else
		sl28wdt_disable();

	if (!(ctrl & CTRL_EN1))
		board_disable_failsafe();

	if (ctrl & CTRL_OE1)
		sl28wdt_enable_output();
	else
		sl28wdt_disable_output();
}

void timg1_irq(void)
{
	if (ctrl & CTRL_OE0)
		board_sys_reset(ctrl & CTRL_EN1);
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

void sl28wdt_stop(void)
{
	if (ctrl & CTRL_EN1)
		return;

	sl28wdt_disable();
	sl28wdt_disable_output();
}

void sl28wdt_reset(bool force)
{
	/* if the supervisor mode is enabled, don't reset the peripheral */
	if (!force && (ctrl & CTRL_EN1))
		return;

	/* set ctrl default value */
	ctrl = CTRL_EN1 | CTRL_OE0;

	if (config->flags & CFG_F_ENABLE_WATCHDOG)
		ctrl |= CTRL_EN0;
	if (config->flags & CFG_F_DISABLE_FAILSAFE_WATCHDOG)
		ctrl &= ~CTRL_EN1;

	/* default value for the counter register */
	iow(TIM_LOAD, DEFAULT_TOUT << 8);
	iow(TIM_CTR, DEFAULT_TOUT << 8);

	sl28wdt_ctrl_update();
}

static void sl28wdt_kick(void)
{
	unsigned int load = ior(TIM_LOAD);

	/*
	 * Writing to this register races with the hardware (advance and load
	 * events) if the counter is enabled. The offical way to do it is to
	 * disable the counter, update the value and enable the counter again.
	 *
	 * Disabling the counter is considered a bad choice for a watchdog. Thus
	 * just try to write the value three times and assume that at least one
	 * write will go through.
	 */

	iow(TIM_CTR, load);
	iow(TIM_CTR, load);
	iow(TIM_CTR, load);
}

void sl28wdt_write(unsigned char offset, unsigned char value)
{
	bool need_kick = false;

	switch (offset) {
	case REG_CTRL:
		/* disallow further writes */
		if (ctrl & CTRL_LOCKED)
			return;

		/*
		 * kick the watchdog automatically if any enable bit transition
		 * from 0 -> 1.
		 */
		if (!(ctrl & (CTRL_EN0 | CTRL_EN1)) && (value & (CTRL_EN0 | CTRL_EN1)))
			need_kick = true;

		ctrl = value;
		sl28wdt_ctrl_update();
		if (need_kick)
			sl28wdt_kick();
		break;
	case REG_TOUT:
		iow(TIM_LOAD, value << 8);
		break;
	case REG_KICK:
		if (value == KICK_VALUE && ior(TIM_CTRCTL) & CTRCTL_EN)
			sl28wdt_kick();
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

	/* set output high if the counter is running */
	iow(TIM_CCACT0, CCACT_SWFRCACT_HIGH);

	/*
	 * Default value for the output is high, this will be changed once the
	 * watchdog is enabled.
	 */
	iow(TIM_OCTL0, OCTL_CCPIV);

	/* unmask and enable interrupts */
	iow(TIM_IMASK, INT_Z);
	nvic_enable_irq(TIMG1_IRQ);
}
