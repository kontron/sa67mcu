// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2025 Kontron Europe GmbH
 */

#include "misc.h"
#include "iomux.h"

#define IOMUX_BASE		0x40428000
#define IOMUX_PINCM(n)		(IOMUX_BASE + 4 * (n))

#define PINCM_PC		BIT(7)
#define PINCM_PF_NC		0x0
#define PINCM_PF_GPIO		0x1
#define PINCM17_PF_TIMG0_C0	0x5
#define PINCM24_PF_UART0_TX	0x2

void iomux_init(void)
{
	iow(IOMUX_PINCM(17), PINCM17_PF_TIMG0_C0 | PINCM_PC);	/* PA16 AIN/TIMG0_C0 (AIN_VDD_RTC) */
	iow(IOMUX_PINCM(19), PINCM_PF_GPIO | PINCM_PC);	/* PA18 GPO (LED_OUT) */
	iow(IOMUX_PINCM(24), PINCM24_PF_UART0_TX | PINCM_PC); /* PA23 UART0_TX */
}
