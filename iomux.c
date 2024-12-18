// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2025 Kontron Europe GmbH
 */

#include "misc.h"
#include "iomux.h"

#define IOMUX_BASE		0x40428000

#define IOMUX_PINCM(n)		(IOMUX_BASE + 4 * (n))
#define PINCM_HIZ1		BIT(25)
#define PINCM_INENA		BIT(18)
#define PINCM_PC		BIT(7)
#define PINCM_PF_NC		0x0
#define PINCM_PF_GPIO		0x1
#define PINCM1_PF_I2C0_SCL	0x3
#define PINCM2_PF_I2C0_SDA	0x3
#define PINCM17_PF_TIMG0_C0	0x5
#define PINCM24_PF_UART0_TX	0x2

void iomux_init(void)
{
	/* PA0 I2C0_SCL */
	iow(IOMUX_PINCM(1), PINCM_INENA | PINCM_HIZ1 | PINCM_PC | PINCM1_PF_I2C0_SCL);
	/* PA0 I2C0_SDA */
	iow(IOMUX_PINCM(2), PINCM_INENA | PINCM_HIZ1 | PINCM_PC | PINCM1_PF_I2C0_SCL);
	/* PA16 AIN/TIMG0_C0 (AIN_VDD_RTC) */
	iow(IOMUX_PINCM(17), PINCM_PC | PINCM17_PF_TIMG0_C0 );
	/* PA18 GPO (LED_OUT) */
	iow(IOMUX_PINCM(19), PINCM_PC | PINCM_PF_GPIO );
	/* PA23 UART0_TX */
	iow(IOMUX_PINCM(24), PINCM_PC | PINCM24_PF_UART0_TX);
}
