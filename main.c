// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2025 Kontron Europe GmbH
 */

#include <stdbool.h>
#include <stddef.h>
#include "adc.h"
#include "bootmode.h"
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

#define PMIC_I2C_ADDR 0x44

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
	/* PA2 (BOOTMODE03) */
	{ 3, PINCM_PC | PINCM_PF_GPIO },
	/* PA3 (BOOTMODE04) */
	{ 4, PINCM_PC | PINCM_PF_GPIO },
	/* PA4 (BOOTMODE05) */
	{ 5, PINCM_PC | PINCM_PF_GPIO },
	/* PA5 (BOOTMODE06) */
	{ 6, PINCM_PC | PINCM_PF_GPIO },
	/* PA6 (BOOTMODE07) */
	{ 7, PINCM_PC | PINCM_PF_GPIO },
	/* PA7 (BOOTMODE08) */
	{ 8, PINCM_PC | PINCM_PF_GPIO },
	/* PA8 (BOOTMODE09) */
	{ 9, PINCM_PC | PINCM_PF_GPIO },
	/* PA23 (BOOTMODE10) */
	{ 24, PINCM_PC | PINCM_PF_GPIO },
	/* PA22 (BOOTMODE11) */
	{ 23, PINCM_PC | PINCM_PF_GPIO },
	/* PA9 (BOOTMODE12) */
	{ 10, PINCM_PC | PINCM_PF_GPIO },
	/* PA10 (BOOTMODE13) */
	{ 11, PINCM_PC | PINCM_PF_GPIO },
	{ 0 }
};

#define PMIC_MASK_STARTUP 0x52

static void pmic_abort_power_up(void)
{
	unsigned char buf[2];
	int ret;

	buf[0] = PMIC_MASK_STARTUP;
	buf[1] = 0x33;

	ret = i2c_xfer_blocking(PMIC_I2C_ADDR, buf, sizeof(buf), NULL, 0);
	if (ret)
		printf("ERROR: Couldn't write to PMIC\n");
}

static void pmic_configure(void)
{
	if (config->flags & CFG_F_INITIAL_PWR_OFF)
		pmic_abort_power_up();
}

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
	bootmode_init();

	i2c_bus_reset();

	printf("sa67mcu v%d (%s)\n", version, gitversion);
	printf("reset cause: %02Xh\n", reset_cause);
	printf("Configuration:\n"
	       "  version:%d\n"
	       "  flags:%04x\n"
	       "  bootmode:%04x\n",
	       config->version, config->flags, config->bootmode);

	/* when powering-up for the first time, configure the PMIC and reset the PHYs */
	if (reset_cause == RSTCAUSE_POR) {
		pmic_configure();
	}

	i2c_enable_target_mode();

	if (reset_cause == RSTCAUSE_WWDT0)
		led_set_period(200);
	else
		led_set_period(1000);


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
