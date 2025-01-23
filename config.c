// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2025 Kontron Europe GmbH
 *
 */

#include <stdbool.h>
#include "config.h"
#include "misc.h"
#include "nvm.h"

#define REG_CTRL	0
#define CTRL_LOAD	BIT(0)
#define CTRL_SAVE	BIT(1)
#define CTRL_ERASE	BIT(2)
#define CTRL_BUSY	BIT(7)
#define REG_OFS		1
#define REG_DATA	2

static bool do_save_config;
static bool do_erase_config;
static unsigned char ofs;
static unsigned char shadow_config[sizeof(struct configuration)];

static struct configuration nvm_config __attribute__((section(".config")));
static struct configuration running_config;
const struct configuration *config = &running_config;

void config_init(void)
{
	unsigned char *src = (void*)&nvm_config;
	unsigned char *dst = (void*)&running_config;
	int i;

	for (i = 0; i < sizeof(running_config); i++)
		dst[i] = src[i] ^ 0xff;

	memset(shadow_config, 0xff, sizeof(shadow_config));
}

void config_loop(void)
{
	_Static_assert(sizeof(struct configuration) == 8);
	if (do_erase_config) {
		nvm_erase_sector(&nvm_config);
		do_erase_config = false;
	}
	if (do_save_config) {
		nvm_erase_sector(&nvm_config);
		nvm_program_word(&nvm_config, shadow_config);
		do_save_config = false;
	}
}

void config_write(unsigned char offset, unsigned char value)
{
	switch (offset) {
	case REG_CTRL:
		if (value & CTRL_LOAD)
			memcpy(&shadow_config, &nvm_config, sizeof(nvm_config));
		else if (value & CTRL_ERASE)
			do_erase_config = true;
		else if (value & CTRL_SAVE)
			do_save_config = true;
		break;
	case REG_OFS:
		ofs = value & 0x7;
		break;
	case REG_DATA:
		shadow_config[ofs] = value ^ 0xff;
		break;
	}
}

unsigned char config_read(unsigned char offset)
{
	switch (offset) {
	case REG_CTRL:
		return (do_save_config || do_erase_config) ? CTRL_BUSY : 0;
	case REG_OFS:
		return ofs;
	case REG_DATA:
		return shadow_config[ofs] ^ 0xff;
	default:
		return 0;
	}
}
