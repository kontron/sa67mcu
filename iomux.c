// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2025 Kontron Europe GmbH
 */

#include "misc.h"
#include "iomux.h"

#define IOMUX_BASE		0x40428000
#define IOMUX_PINCM(n)		(IOMUX_BASE + 4 * (n))

#define PINCM24_PF_UART0_TX	0x2
#define PINCM_PC		BIT(7)

void iomux_init(void)
{
	iow(IOMUX_PINCM(24), PINCM24_PF_UART0_TX | PINCM_PC); /* PA23 UART0_TX */
}
