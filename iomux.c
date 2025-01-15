// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2025 Kontron Europe GmbH
 */

#include "misc.h"
#include "iomux.h"

#define IOMUX_BASE		0x40428000

#define IOMUX_PINCM(n)		(IOMUX_BASE + 4 * (n))

void iomux_conf(const struct iomux_config *config)
{
	if (!config || !config->pin)
		return;

	do {
		iow(IOMUX_PINCM(config->pin), config->conf);
	} while ((++config)->pin);
}
