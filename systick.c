// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2025 Kontron Europe GmbH
 */

#include "misc.h"

#define SYS_BASE 0xe0000000

#define SYST_CSR	(SYS_BASE + 0xe010)
#define CSR_CLKSRC	BIT(2)
#define CSR_EN		BIT(0)
#define SYST_RVR	(SYS_BASE + 0xe014)
#define SYST_CVR	(SYS_BASE + 0xe018)
#define SYSTICK_MASK	0xffffff

#define SYSTICK_FREQ 32000000
#define TICKS_PER_US (SYSTICK_FREQ / 1000000)

void systick_init(void)
{
	iow(SYST_RVR, SYSTICK_MASK);

	/* use MCLK */
	iow(SYST_CSR, CSR_CLKSRC | CSR_EN);
}

/* no overflow hanlding, which means we can just sleep for about 524ms */
void udelay(unsigned int us)
{
	unsigned int start = ior(SYST_CVR);
	unsigned int delay = us * TICKS_PER_US;

	while (((start - ior(SYST_CVR)) & SYSTICK_MASK) < delay);
}
