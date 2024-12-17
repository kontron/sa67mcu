// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2025 Kontron Europe GmbH
 *
 */

#include "config.h"
#include "misc.h"
#include "nvm.h"

static struct configuration nvm_config __attribute__((section(".config")));
struct configuration config;

void config_init(void)
{
	_Static_assert(sizeof(config) == 8);
	_Static_assert(sizeof(nvm_config) == 8);

	memcpy(&config, &nvm_config, sizeof(config));
}

void config_save(void)
{
	nvm_erase_sector(&nvm_config);
	nvm_program_word(&nvm_config, &config);
}
