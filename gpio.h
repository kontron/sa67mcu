// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2025 Kontron Europe GmbH
 */

#include <stdbool.h>

#ifndef __GPIO_H
#define __GPIO_H

void gpio_init(void);
void gpio_out(int pin);
void gpio_in(int pin);
bool gpio_get(int pin);
void gpio_set(int pin, bool value);
void gpio_toggle(int pin);
void gpio_mask_set(unsigned int mask);
void gpio_mask_clr(unsigned int mask);
void gpio_mask_toggle(unsigned int mask);

#endif /* __GPIO_H */
