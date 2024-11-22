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

int printf(const char *format,...)
{
	va_list args;
	int rc;

	va_start(args,format);
	rc = mini_vprintf_cooked(uart_putc, format, args);
	va_end(args);
	return rc;
}
