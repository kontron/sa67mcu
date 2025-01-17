// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2025 Kontron Europe GmbH
 */

#ifndef __I2C_H
#define __I2C_H

void i2c_init(void);
void i2c_enable_target_mode(void);
void i2c_disable_target_mode(void);
int i2c_xfer_blocking(unsigned char addr,
		      unsigned char *txbuf, unsigned int txlen,
		      unsigned char *rxbuf, unsigned int rxlen);

#endif /* __I2C_H */
