// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2025 Kontron Europe GmbH
 */

#include <stdbool.h>
#include "adc.h"
#include "config.h"
#include "cp.h"
#include "gpio.h"
#include "i2c.h"
#include "iomux.h"
#include "led.h"
#include "misc.h"
#include "sl28wdt.h"
#include "sysctl.h"
#include "systick.h"
#include "ticks.h"
#include "uart.h"
#include "vref.h"
#include "wdt.h"

#ifndef VERSION
#define VERSION 255
#endif

#ifndef GITVERSION
#define GITVERSION ""
#endif

static const char *gitversion = GITVER;
const unsigned char version = VERSION;

const struct iomux_config iomux_uart_debug_config[] = {
	/* PA23 (debug UART) */
	{ 24, PINCM_PC | PINCM24_PF_UART0_TX },
	{ 0 }
};

const struct iomux_config iomux_default_config[] = {
	/* PA0 (SCL) */
	{ 1, PINCM_INENA | PINCM_HIZ1 | PINCM_PC | PINCM1_PF_I2C0_SCL },
	/* PA1 (SDA) */
	{ 2, PINCM_INENA | PINCM_HIZ1 | PINCM_PC | PINCM2_PF_I2C0_SDA },
	/* PA16 (analog RTC voltage input, both AIN and TIMG0_C0) */
	{ 17, PINCM_PC | PINCM17_PF_TIMG0_C0 },
	/* PA18 (healthy LED) */
	{ 19, PINCM_PC | PINCM19_PF_TIMG4_C1 },
	{ 0 }
};

int main(void)
{
	config_init();
	sysctl_init();
	sl28wdt_init();
	iomux_conf(iomux_default_config);
	if (config->flags & CFG_F_DEBUG)
		iomux_conf(iomux_uart_debug_config);
	systick_init();
	ticks_init();
	uart_init();
	vref_init();
	adc_init();
	gpio_init();
	cp_init();
	i2c_init();
	wdt_init();
	led_init();

	i2c_bus_reset();
	i2c_enable_target_mode();

	printf("sa67mcu v%d (%s)\n", version, gitversion);
	printf("reset cause: %02Xh\n", reset_cause);
	printf("Configuration:\n"
	       "  version:%d\n"
	       "  flags:%04x\n"
	       "  bootmode:%04x\n",
	       config->version, config->flags, config->bootmode);

	if (reset_cause == RSTCAUSE_WWDT0)
		led_set_mode(LED_MODE_WDOG_RESET);
	else
		led_set_mode(LED_MODE_NORMAL);


	while (true) {
		wdt_kick();
		config_loop();
		udelay(500000);
		cp_enable();
		udelay(2000);
		cp_disable();
		udelay(1000);
		printf("[%s]\n", ticks_str());
		printf("AIN0 %d\n", adc_value(0));
		printf("AIN8 %d\n", adc_value(1));
		printf("AT %d\n", adc_value(2));
		unsigned int temp = adc_temperature();
		printf("T %d.%03d\n", temp / 1000, temp % 1000);
	}
}
