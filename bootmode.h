// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2025 Kontron Europe GmbH
 */

#ifndef __BOOTMODE_H
#define __BOOTMODE_H

#define BOOTMODE_FAILSAFE	0x00000018

void bootmode_write(unsigned char offset, unsigned char value);
unsigned char bootmode_read(unsigned char offset);

void bootmode_enable(unsigned short mode);
void bootmode_write(unsigned char offset, unsigned char value);
unsigned char bootmode_read(unsigned char offset);
void bootmode_disable(void);
void bootmode_init(void);

#endif /* __BOOTMODE_H */
