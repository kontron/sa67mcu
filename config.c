// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2025 Kontron Europe GmbH
 *
 */

#include "config.h"
#include "misc.h"
#include "nvm.h"

static struct configuration nvm_config __attribute__((section(".config")));
static struct configuration running_config;
const struct configuration *config = &running_config;

void config_init(void)
{
	_Static_assert(sizeof(struct configuration) == 8);

	memcpy(&config, &nvm_config, sizeof(config));
}

void config_save(void)
{
	nvm_erase_sector(&nvm_config);
	nvm_program_word(&nvm_config, &config);
}
