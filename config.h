// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2025 Kontron Europe GmbH
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include "misc.h"

#define CFG_F_INITIAL_PWR_OFF			BIT(0)
#define CFG_F_DRIVE_BOOTMODE			BIT(1)
#define CFG_F_ENABLE_WATCHDOG			BIT(2)
#define CFG_F_DISABLE_FAILSAFE_WATCHDOG		BIT(3)
#define CFG_F_DISABLE_WATCHDOG			BIT(4)
#define CFG_F_DEBUG				BIT(15)

struct configuration {
	unsigned char version;
	unsigned short flags;
	unsigned short bootmode;
	unsigned char reserved1[3];
} __attribute__((packed));

extern const struct configuration *config;

void config_init(void);
void config_loop(void);
void config_write(unsigned char offset, unsigned char value);
unsigned char config_read(unsigned char offset);

#endif /* __CONFIG_H */
