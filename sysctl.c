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

#define SYSCTL_RSTCAUSE		(SYSCTL_BASE + 0x1220)

unsigned int reset_cause;

void sysctl_init(void)
{
	reset_cause = ior(SYSCTL_RSTCAUSE);

	/* enable MFCLK (4MHz) */
	iow(SYSCTL_MCLKCFG, MCLKCFG_USEMFTICK);
}
