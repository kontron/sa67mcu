// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2025 Kontron Europe GmbH
 */

#ifndef __SL28WDT_H
#define __SL28WDT_H

void sl28wdt_init(void);
void sl28wdt_stop(void);
void sl28wdt_reset(bool force);
unsigned char sl28wdt_read(unsigned char offset);
void sl28wdt_write(unsigned char offset, unsigned char value);
void sl28wdt_failsafe_mode(bool enable);

#endif /* __SL28WDT_H */
