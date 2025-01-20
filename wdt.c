// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2025 Kontron Europe GmbH
 */

#include "misc.h"
#include "wdt.h"

#define WWDT_BASE		0x40080000

#define WWDT_PWREN		(WWDT_BASE + 0x0800)
#define PWREN_KEY		0x26000000
#define PWREN_ENABLE		BIT(0)

#define WWDT_WWDTCTL0		(WWDT_BASE + 0x1100)
#define WWDTCTL0_KEY		0xc9000000
#define WWDTCTL0_CLKDIV(n)	((n) << 0)
#define WWDTCTL0_PER(n)		((n) << 4)

#define WWDT_WWDTCNTRST		(WWDT_BASE + 0x1108)
#define WWDTCNTRST_RESTART	0xa7

void wdt_init(void)
{
	iow(WWDT_PWREN, PWREN_KEY | PWREN_ENABLE);
	
	/* set watchdog timeout to 1s */
	iow(WWDT_WWDTCTL0, WWDTCTL0_KEY | WWDTCTL0_CLKDIV(0) | WWDTCTL0_PER(3));
}

void wdt_kick(void)
{
	iow(WWDT_WWDTCNTRST, WWDTCNTRST_RESTART);
}
