// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2025 Kontron Europe GmbH
 */

#ifndef __IOMUX_H
#define __IOMUX_H

#define PINCM_HIZ1		BIT(25)
#define PINCM_INENA		BIT(18)
#define PINCM_PC		BIT(7)
#define PINCM_PF_NC		0x0
#define PINCM_PF_GPIO		0x1
#define PINCM1_PF_I2C0_SCL	0x3
#define PINCM2_PF_I2C0_SDA	0x3
#define PINCM17_PF_TIMG0_C0	0x5
#define PINCM19_PF_TIMG4_C1	0x5
#define PINCM24_PF_UART0_TX	0x2

struct iomux_config {
	unsigned int pin;
	unsigned int conf;
};

void iomux_conf(const struct iomux_config *config);

#endif /* __IOMUX_H */
