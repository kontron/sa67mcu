// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2025 Kontron Europe GmbH
 */

#ifndef __BOARD_H
#define __BOARD_H

#include <stdbool.h>

void board_eth_reset(void);
void board_sys_reset(bool failsafe_mode);

#endif /* __BOARD_H */
