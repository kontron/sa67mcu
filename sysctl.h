// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2025 Kontron Europe GmbH
 */

#ifndef __SYSCTL_H
#define __SYSCTL_H

#define RSTCAUSE_NO_RESET		0x0
#define RSTCAUSE_POR			0x1
#define RSTCAUSE_POR_BY_NRST		0x2
#define RSTCAUSE_POR_BY_SW		0x3
#define RSTCAUSE_BOOTRST_BY_NRST	0xc
#define RSTCAUSE_BOOTRST_BY_SW		0xd
#define RSTCAUSE_WWDT0			0xe

extern unsigned int reset_cause;
void sysctl_init(void);
void sysctl_invoke_bsl(void);
void sysctl_por(void);

#endif /* __SYSCTL_H */
