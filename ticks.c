// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2025 Kontron Europe GmbH
 *
 * The ticks counter uses TIMG2 with a period of 1ms.
 */

#include "miniprintf.h"
#include "misc.h"
#include "led.h"

#define TIMG2_BASE		0x40088000
#define TIMG2_IRQ		18

#define TIM_PWREN		(TIMG2_BASE + 0x0800)
#define PWREN_KEY		0x26000000
#define PWREN_ENABLE		BIT(0)

#define TIM_CLKSEL		(TIMG2_BASE + 0x1008)
#define CLKSEL_MFCLK		BIT(2)

#define TIM_IMASK		(TIMG2_BASE + 0x1028)
#define INT_Z			BIT(0)

#define TIM_CCLKCTL		(TIMG2_BASE + 0x1108)
#define CCLKCTL_CLKEN		BIT(0)

#define TIM_CTRCTL		(TIMG2_BASE + 0x1804)
#define CTRCTL_EN		BIT(0)
#define CTRCTL_REPEAT		(1 << 1)

#define TIM_LOAD		(TIMG2_BASE + 0x1808)

#define MFCLK_FREQ		4000000
#define TICKS_PERIOD_MS		1

#include "misc.h"
#include "nvic.h"
#include "ticks.h"

static volatile ticks_t __ticks;
ticks_t ticks;

void timg2_irq(void)
{
	__ticks++;
}

void ticks_loop(void)
{
	nvic_disable_irq(TIMG2_IRQ);
	ticks = __ticks;
	nvic_enable_irq(TIMG2_IRQ);
}

/*
 * Just for debugging, because we don't have a printf with
 * long long support. Will overflow. Also not thread-safe.
 */
const char *ticks_str(void)
{
	static char buf[sizeof("4294967.296")];

	mini_snprintf(buf, sizeof(buf), "%d.%03d",
		      (unsigned int)ticks / 1000,
		      (unsigned int)ticks % 1000);
	buf[sizeof(buf) - 1] = '\0';

	return buf;
}

void ticks_init(void)
{
	iow(TIM_PWREN, PWREN_KEY | PWREN_ENABLE);

	/* use 4MHz mid frequency clock */
	iow(TIM_CLKSEL, CLKSEL_MFCLK);

	/* ungate the clock */
	iow(TIM_CCLKCTL, CCLKCTL_CLKEN);

	/* set period */
	iow(TIM_LOAD, MFCLK_FREQ * TICKS_PERIOD_MS / 1000);

	/* unmask and enable interrupts */
	iow(TIM_IMASK, INT_Z);
	nvic_enable_irq(TIMG2_IRQ);

	/* enable timer */
	iow(TIM_CTRCTL, CTRCTL_REPEAT | CTRCTL_EN);
}
