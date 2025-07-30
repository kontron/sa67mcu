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
#define SYSCTL_RESETLEVEL	(SYSCTL_BASE + 0x1300)
#define RESETLEVEL_SYSRST_BSL	2
#define RESETLEVEL_POR		3
#define SYSCTL_RESETCMD		(SYSCTL_BASE + 0x1304)
#define RESETCMD_KEY		(0xe4000000)
#define RESETCMD_GO		BIT(0)

unsigned int reset_cause;

void sysctl_por(void)
{
	iow(SYSCTL_RESETLEVEL, RESETLEVEL_POR);
	iow(SYSCTL_RESETCMD, RESETCMD_KEY |RESETCMD_GO);
}

void sysctl_invoke_bsl(void)
{
	iow(SYSCTL_RESETLEVEL, RESETLEVEL_SYSRST_BSL);
	iow(SYSCTL_RESETCMD, RESETCMD_KEY |RESETCMD_GO);
}

void sysctl_init(void)
{
	reset_cause = ior(SYSCTL_RSTCAUSE);

	/* enable MFCLK (4MHz) */
	iow(SYSCTL_MCLKCFG, MCLKCFG_USEMFTICK);
}
