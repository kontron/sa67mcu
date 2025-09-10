// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2025 Kontron Europe GmbH
 */

#include <stdbool.h>

#ifndef __GPIO_H
#define __GPIO_H

#define GPIO_IRQ_RISING_EDGE BIT(0)
#define GPIO_IRQ_FALLING_EDGE BIT(1)

void gpio_init(void);
void gpio_out(int pin);
void gpio_out_mask(unsigned int mask);
void gpio_in(int pin);
void gpio_in_mask(unsigned int mask);
bool gpio_get(int pin);
void gpio_set(int pin, bool value);
void gpio_toggle(int pin);
void gpio_mask_set(unsigned int mask);
void gpio_mask_clr(unsigned int mask);
void gpio_mask_toggle(unsigned int mask);

void gpio_conf_irq(int pin, int mode);
void gpio_irq_ack(int pin);
void gpio_irq_unmask(int pin);
void gpio_irq_mask(int pin);
int gpio_irq_pin(void);

#endif /* __GPIO_H */
