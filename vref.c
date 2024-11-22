// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2025 Kontron Europe GmbH
 */

#include "vref.h"
#include "misc.h"

#define VREF_BASE		0x40030000
#define VREF_PWREN		(VREF_BASE + 0x0800)
#define PWREN_KEY		0x26000000
#define PWREN_ENABLE		BIT(0)
#define VREF_CTL0		(VREF_BASE + 0x1100)
#define CTL0_BUFCONFIG_V1P4	BIT(7)
#define CTL0_ENABLE		BIT(0)

void vref_init(void)
{
	iow(VREF_PWREN, PWREN_KEY | PWREN_ENABLE);
	iow(VREF_CTL0, CTL0_ENABLE | CTL0_BUFCONFIG_V1P4);
}
