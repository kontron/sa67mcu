// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2025 Kontron Europe GmbH
 */

#ifndef __CONFIG_H
#define __CONFIG_H

struct configuration {
	unsigned int val32;
	unsigned char val8;
	unsigned char pad[3];
} __attribute__((packed));

extern struct configuration config;

void config_init(void);
void config_save(void);

#endif /* __CONFIG_H */
