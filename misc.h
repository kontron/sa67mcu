// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2025 Kontron Europe GmbH
 */

#ifndef __MISC_H
#define __MISC_H

#define BIT(x) (1<<(x))

static inline void iow(unsigned int addr, unsigned int data)
{
	*(volatile unsigned int *)addr = data;
}

static inline unsigned int ior(unsigned int addr)
{
	return *(volatile unsigned int*)addr;
}

unsigned int strlen(const char *s);
int printf(const char *format,...) __attribute((format(printf,1,2)));
#endif /* __MISC_H */
