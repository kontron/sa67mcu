// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2025 Kontron Europe GmbH
 *
 * The charge pump uses TIMG0 with a frequency of 100kHz and a duty cycle of 50%.
 */

#include "cp.h"
#include "misc.h"

#define TIMG0_BASE		0x40084000

#define TIM_PWREN		(TIMG0_BASE + 0x0800)
#define PWREN_KEY		0x26000000
#define PWREN_ENABLE		BIT(0)

#define TIM_CLKSEL		(TIMG0_BASE + 0x1008)
#define CLKSEL_MFCLK		BIT(2)

#define TIM_CCPD		(TIMG0_BASE + 0x1100)
#define CCPD_C0CCP0_OUTPUT	BIT(0)

#define TIM_CTRCTL		(TIMG0_BASE + 0x1804)
#define CTRCTL_EN		BIT(0)
#define CTRCTL_REPEAT		(1 << 1)

#define TIM_LOAD		(TIMG0_BASE + 0x1808)

#define MCLK_FREQ		4000000
#ifndef CFG_CP_FREQ
#define CFG_CP_FREQ		100000
#endif

#define TIM_CCACT0		(TIMG0_BASE + 0x1870)
#define CCACT_ZACT_TOGGLE	(3 << 0)

void cp_init(void)
{
	iow(TIM_PWREN, PWREN_KEY | PWREN_ENABLE);

	/* use 4 MHz mid frequency clock */
	iow(TIM_CLKSEL, CLKSEL_MFCLK);

	/*
	 * set frequency, because we just toggle the output on zero condition we
	 * have to be twice as fast as the configured frequency
	 */
	iow(TIM_LOAD, MCLK_FREQ / CFG_CP_FREQ / 2);

	/* toggle output on zero */
	iow(TIM_CCACT0, CCACT_ZACT_TOGGLE);

	/* enable timer */
	iow(TIM_CTRCTL, CTRCTL_REPEAT | CTRCTL_EN);
}

void cp_enable(void)
{
	/* enable output */
	iow(TIM_CCPD, CCPD_C0CCP0_OUTPUT);
}

void cp_disable(void)
{
	/* disable output */
	iow(TIM_CCPD, 0);
}
