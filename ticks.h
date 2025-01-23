// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2025 Kontron Europe GmbH
 */

#ifndef __TICKS_H
#define __TICKS_H

typedef unsigned long long ticks_t;

void ticks_init(void);
ticks_t ticks_get(void);
const char *ticks_str(void);

#endif /* __TICKS_H */
