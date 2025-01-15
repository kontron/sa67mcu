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
#include "misc.h"
#include "sysctl.h"
#include "systick.h"
#include "uart.h"
#include "vref.h"

#define LED_PIN 18

const struct iomux_config iomux_default_config[] = {
	/* PA0 I2C0_SCL */
	{ 1, PINCM_INENA | PINCM_HIZ1 | PINCM_PC | PINCM1_PF_I2C0_SCL },
	/* PA1 I2C0_SDA */
	{ 2, PINCM_INENA | PINCM_HIZ1 | PINCM_PC | PINCM2_PF_I2C0_SDA },
	/* PA16 AIN/TIMG0_C0 (AIN_VDD_RTC) */
	{ 17, PINCM_PC | PINCM17_PF_TIMG0_C0 },
	/* PA18 GPO (LED_OUT) */
	{ 19, PINCM_PC | PINCM_PF_GPIO },
	/* PA23 UART0_TX */
	{ 24, PINCM_PC | PINCM24_PF_UART0_TX },
	{ 0 }
};

int main(void)
{
	clocks_init();
	iomux_conf(iomux_default_config);
	systick_init();
	uart_init();
	vref_init();
	adc_init();
	gpio_init();
	cp_init();
	i2c_init();
	config_init();

	i2c_enable_target_mode();

	gpio_out(LED_PIN);

	printf("Hello World\n");

	if (config.val8)
		config.val8 <<= 1;
	else
		config.val8 = 0xff;
	printf("val8=%x\n", config.val8);
	config_save();

	while (true) {
		udelay(500000);
		cp_enable();
		udelay(2000);
		cp_disable();
		udelay(1000);
		printf("AIN0 %d\n", adc_value(0));
		printf("AIN8 %d\n", adc_value(1));
		printf("AT %d\n", adc_value(2));
		unsigned int temp = adc_temperature();
		printf("T %d.%03d\n", temp / 1000, temp % 1000);
		gpio_toggle(LED_PIN);
	}
}
