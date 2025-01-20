// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2025 Kontron Europe GmbH
 */

#ifndef __LED_H
#define __LED_H

enum led_mode {
	LED_MODE_NORMAL,
	LED_MODE_WDOG_RESET,
};

void led_init(void);
void led_set_mode(enum led_mode mode);

#endif /* __LED_H */
