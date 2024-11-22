// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2025 Kontron Europe GmbH
 */

#include "adc.h"
#include "misc.h"

#define ADC_BASE		0x40004000
#define ADC_PWREN		(ADC_BASE + 0x0800)
#define PWREN_KEY		0x26000000
#define PWREN_ENABLE		BIT(0)

#define ADC_CTL0		(ADC_BASE + 0x1100)
#define CTL0_SCLKDIV_DIV_BY_8	(3 << 24)
#define CTL0_ENC		BIT(0)

#define ADC_CTL1		(ADC_BASE + 0x1104)
#define CTL1_CONSEQ_SEQ_REPEAT	(3 << 16)
#define CTL1_SC			BIT(8)

#define ADC_CTL2		(ADC_BASE + 0x1108)
#define CTL2_START(n)		((n) << 16)
#define CTL2_END(n)		((n) << 24)

#define ADC_SCOMP0		(ADC_BASE + 0x1114)
#define ADC_MEMCTL(n)		(ADC_BASE + 0x1180 + (n) * 4)
#define MEMCTL_VRSEL_VREF	(2 << 8)
#define MEMCTL_CHAN(n)		((n) << 0)
#define ADC_MEMRES(n)		(ADC_BASE + 0x1280 + (n) * 4)

void adc_init(void)
{
	iow(ADC_PWREN, PWREN_KEY | PWREN_ENABLE);

	/* sample three channels */
	iow(ADC_CTL2, CTL2_START(0) | CTL2_END(2));
	iow(ADC_SCOMP0, 32);
	iow(ADC_MEMCTL(0), MEMCTL_CHAN(0) | MEMCTL_VRSEL_VREF);
	iow(ADC_MEMCTL(1), MEMCTL_CHAN(8) | MEMCTL_VRSEL_VREF);
	iow(ADC_MEMCTL(2), MEMCTL_CHAN(11) | MEMCTL_VRSEL_VREF);
	iow(ADC_CTL1, CTL1_CONSEQ_SEQ_REPEAT | CTL1_SC);
	iow(ADC_CTL0, CTL0_SCLKDIV_DIV_BY_8 | CTL0_ENC);
}

unsigned int adc_value(unsigned int chan)
{
	switch (chan) {
	case 0:
		return ior(ADC_MEMRES(0));
	case 1:
		return ior(ADC_MEMRES(1));
	case 2:
		return ior(ADC_MEMRES(2));
	default:
		return 0;
	}
}

#define TEMP_SENSE0	0x41c4003c
#define TS_TRIM		30000
#define TS_C		((int)(1 / -0.00175))

int adc_temperature(void)
{
	int diff = ior(ADC_MEMRES(2)) - ior(TEMP_SENSE0);

	return (((int)ADC_VREF * diff * TS_C) >> 12) + TS_TRIM;
}
