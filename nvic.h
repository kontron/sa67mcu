// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2025 Kontron Europe GmbH
 */

#ifndef __NVIC_H
#define __NVIC_H

void nvic_enable_irq(unsigned int irq);
void nvic_disable_irq(unsigned int irq);

#endif /* __NVIC_H */
