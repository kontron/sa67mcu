// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2025 Kontron Europe GmbH
 */

#include <stdbool.h>
#include <stddef.h>
#include "adc.h"
#include "board.h"
#include "bootmode.h"
#include "config.h"
#include "cp.h"
#include "gpio.h"
#include "i2c.h"
#include "iomux.h"
#include "led.h"
#include "misc.h"
#include "nvic.h"
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

#define INT_GROUP1_IRQ 1

#define MCU_SPI_FLASH_WP_DISn_PIN 13
#define POR_REQn_PIN 15
#define FORCE_GBE_RST_PIN 17

const struct iomux_config iomux_default_config[] = {
	/* PA0 (SCL) */
	{ 1, PINCM_INENA | PINCM_HIZ1 | PINCM_PC | PINCM1_PF_I2C0_SCL },
	/* PA1 (SDA) */
	{ 2, PINCM_INENA | PINCM_HIZ1 | PINCM_PC | PINCM2_PF_I2C0_SDA },
	/* PA14 (WDT_TIME_OUT#) */
	{ 15, PINCM_PC | PINCM15_PF_TIMG1_C0 },
	/* PA16 (analog RTC voltage input, both AIN and TIMG0_C0) */
	{ 17, PINCM_PC | PINCM17_PF_TIMG0_C0 },
	/* PA18 (healthy LED) */
	{ 19, PINCM_PC | PINCM19_PF_TIMG4_C1 },
	/* PA13 (MCU_SPI_FLASH_WP_DIS#) */
	{ 14, PINCM_INENA | PINCM_PC | PINCM_HIZ1 | PINCM_PF_GPIO },
	/* PA15 (POR_REQ#) */
	{ 16, PINCM_INENA | PINCM_PC | PINCM_HIZ1 | PINCM_PF_GPIO },
	/* PA17 (FORCE_GBE_RST) */
	{ 18, PINCM_PC | PINCM_PF_GPIO },
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


static bool failsafe_mode;
static bool failsafe_mode_latch;

static volatile bool do_disable_failsafe;

void board_disable_failsafe(void)
{
	do_disable_failsafe = true;
}

void group1_irq(void)
{
	bool por = !gpio_get(POR_REQn_PIN);

	printf("Board reset detected (POR %d)\n", por);

	failsafe_mode_latch = failsafe_mode;

	if (por)
		sl28wdt_stop();
	else
		sl28wdt_reset(false);
}

static volatile bool do_sys_reset;

void board_sys_reset(bool failsafe)
{
	failsafe_mode = failsafe;
	failsafe_mode_latch = failsafe;
	do_sys_reset = true;
}

static void board_reset_blocking(void)
{
	gpio_set(POR_REQn_PIN, 0);
	udelay(1000);
	gpio_set(POR_REQn_PIN, 1);
}

static bool do_eth_reset;

void board_eth_reset(void)
{
	do_eth_reset = true;
}

static void eth_reset_blocking(void)
{
	gpio_set(FORCE_GBE_RST_PIN, 1);
	udelay(1000);
	gpio_set(FORCE_GBE_RST_PIN, 0);
}

static volatile bool do_invoke_bsl;

void board_invoke_bsl(void)
{
	do_invoke_bsl = true;
}

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

static bool board_powered(void)
{
	/*
	 * XXX we don't use the WP disable pin for now. Thus infer the board
	 * power state by the external pull-up.
	 */
	return gpio_get(MCU_SPI_FLASH_WP_DISn_PIN);
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

	/* init GPIOs */
	gpio_set(POR_REQn_PIN, 1);
	gpio_out(POR_REQn_PIN);
	gpio_set(FORCE_GBE_RST_PIN, 0);
	gpio_out(FORCE_GBE_RST_PIN);

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
		eth_reset_blocking();
	}

	i2c_enable_target_mode();

	if (!gpio_get(POR_REQn_PIN))
		sl28wdt_stop();
	else
		sl28wdt_reset(true);

	gpio_conf_irq(POR_REQn_PIN,
		      GPIO_IRQ_FALLING_EDGE |GPIO_IRQ_RISING_EDGE);
	gpio_irq_ack(POR_REQn_PIN);
	gpio_irq_unmask(POR_REQn_PIN);

	nvic_enable_irq(INT_GROUP1_IRQ);

	while (true) {
		wdt_kick();
		config_loop();

		/* update the LED state */
		if (!board_powered())
			led_set_period(0);
		else if (failsafe_mode_latch)
			led_set_period(200);
		else
			led_set_period(1000);

		if (do_invoke_bsl) {
			printf("Rebooting into BSL..\n");
			sysctl_invoke_bsl();
		}

		if (do_eth_reset) {
			printf("Resetting ethernet PHYs..\n");
			eth_reset_blocking();
			do_eth_reset = false;
		}

		if (do_disable_failsafe) {
			printf("Disable failsafe mode..\n");
			failsafe_mode = false;
			bootmode_init();
			do_disable_failsafe = false;
		}

		if (do_sys_reset) {
			if (failsafe_mode) {
				printf("Enable failsafe mode..\n");
				bootmode_enable(BOOTMODE_FAILSAFE);
			}
			printf("Resetting board..\n");
			board_reset_blocking();
			do_sys_reset = false;
		}
	}
}
