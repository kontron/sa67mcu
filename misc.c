// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2025 Kontron Europe GmbH
 */

#include "miniprintf.h"
#include "uart.h"
#include "misc.h"

unsigned int strlen(const char *s)
{
	unsigned int len = 0;

	while (*s++) len++;

	return len;
}

void *memcpy(void *dst, const void *src, unsigned int n)
{
	unsigned char *d = dst;
	const unsigned char *s = src;

	while (n--)
		*(d++) = *(s++);

	return dst;
}

void *memset(void *s, int c, unsigned int n)
{
	while (n--)
		((unsigned char*)s)[n] = c;
	return s;
}

int printf(const char *format,...)
{
	va_list args;
	int rc;

	va_start(args,format);
	rc = mini_vprintf_cooked(uart_putc, format, args);
	va_end(args);
	return rc;
}
