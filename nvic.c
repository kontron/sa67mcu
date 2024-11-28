// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2025 Kontron Europe GmbH
 */

#include "misc.h"
#include "nvic.h"

#define NVIC_BASE		0xe000e000
#define NVIC_ISER		(NVIC_BASE + 0x0100)
#define NVIC_ICER		(NVIC_BASE + 0x0180)

void nvic_enable_irq(unsigned int irq)
{
	iow(NVIC_ISER, 1 << irq);
}

void nvic_disable_irq(unsigned int irq)
{
	iow(NVIC_ICER, 1 << irq);
}
