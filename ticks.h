// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2025 Kontron Europe GmbH
 */

#ifndef __TICKS_H
#define __TICKS_H

typedef unsigned long long ticks_t;
extern ticks_t ticks;

void ticks_init(void);
void ticks_loop(void);
const char *ticks_str(void);

#endif /* __TICKS_H */
