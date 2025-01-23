// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2025 Kontron Europe GmbH
 *
 * The LED blinker uses TIMG4 with a frequency of 1Hz/2Hz and a duty cycle of
 * 50%.
 */

#include "misc.h"
#include "led.h"

#define TIMG4_BASE		0x4008c000

#define TIM_PWREN		(TIMG4_BASE + 0x0800)
#define PWREN_KEY		0x26000000
#define PWREN_ENABLE		BIT(0)

#define TIM_CLKDIV		(TIMG4_BASE + 0x1000)
#define CLKDIV_DIV_BY(n)	((n) - 1)

#define TIM_CLKSEL		(TIMG4_BASE + 0x1008)
#define CLKSEL_LFCLK		BIT(1)

#define TIM_CCPD		(TIMG4_BASE + 0x1100)
#define CCPD_C0CCP1_OUTPUT	BIT(1)

#define TIM_CTRCTL		(TIMG4_BASE + 0x1804)
#define CTRCTL_EN		BIT(0)
#define CTRCTL_REPEAT		(1 << 1)

#define TIM_LOAD		(TIMG4_BASE + 0x1808)

#define TIM_CCACT1		(TIMG4_BASE + 0x1874)
#define CCACT_ZACT_TOGGLE	(3 << 0)

#define LFCLK_FREQ		32768

void led_init(void)
{
	iow(TIM_PWREN, PWREN_KEY | PWREN_ENABLE);

	/* use 32kHz low frequency clock */
	iow(TIM_CLKSEL, CLKSEL_LFCLK);

	/* prescale by /4 */
	iow(TIM_CLKDIV, CLKDIV_DIV_BY(4));

	/* toggle output on zero */
	iow(TIM_CCACT1, CCACT_ZACT_TOGGLE);

	/* enable output */
	iow(TIM_CCPD, CCPD_C0CCP1_OUTPUT);

	/* enable timer */
	iow(TIM_CTRCTL, CTRCTL_REPEAT | CTRCTL_EN);
}

void led_set_mode(enum led_mode mode)
{
	unsigned int period_ms;

	switch (mode) {
	case LED_MODE_NORMAL:
		period_ms = 1000;
		break;
	case LED_MODE_WDOG_RESET:
		period_ms = 200;
		break;
	default:
		return;
	};

	/*
	 * set frequency, because we just toggle the output on zero condition we
	 * have to be twice as fast as the configured frequency
	 */
	iow(TIM_LOAD, LFCLK_FREQ * period_ms / 2 / 1000 / 4);
}
