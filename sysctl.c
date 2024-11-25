// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2025 Kontron Europe GmbH
 */

#include "misc.h"
#include "sysctl.h"

#define SYSCTL_BASE		0x400af000
#define SYSCTL_MCLKCFG		(SYSCTL_BASE + 0x1104)
#define MCLKCFG_USEMFTICK	BIT(12)
#define MCLKCFG_MDIV_DIV_BY(n)	((n)-1)

void clocks_init(void)
{
	/* enable MFCLK (4MHz) */
	iow(SYSCTL_MCLKCFG, MCLKCFG_USEMFTICK);
}
