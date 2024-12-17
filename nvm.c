// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2025 Kontron Europe GmbH
 */

#include "misc.h"
#include "nvm.h"

#define FLASHCTL_BASE			0x400cd000

#define FLASHCTL_CMDEXEC		(FLASHCTL_BASE + 0x1100)

#define FLASHCTL_CMDTYPE		(FLASHCTL_BASE + 0x1104)
#define CMDTYPE_CMD_NOOP		(0x0 << 0)
#define CMDTYPE_CMD_PROGRAM		(0x1 << 0)
#define CMDTYPE_CMD_ERASE		(0x2 << 0)
#define CMDTYPE_CMD_READ_VERIFY		(0x3 << 0)
#define CMDTYPE_CMD_BLANK_VERIFY	(0x6 << 0)
#define CMDTYPE_SIZE_1_WORD		(0x0 << 4)
#define CMDTYPE_SIZE_2_WORD		(0x1 << 4)
#define CMDTYPE_SIZE_4_WORD		(0x2 << 4)
#define CMDTYPE_SIZE_8_WORD		(0x3 << 4)
#define CMDTYPE_SIZE_SECTOR		(0x4 << 4)
#define CMDTYPE_SIZE_BANK		(0x5 << 4)

#define FLASHCTL_CMDCTL			(FLASHCTL_BASE + 0x1108)
#define CMDCTL_REGIONSEL_MAIN		(0x1 << 9)

#define FLASHCTL_CMDADDR		(FLASHCTL_BASE + 0x1120)
#define FLASHCTL_CMDBYTEN		(FLASHCTL_BASE + 0x1124)
#define FLASHCTL_CMDDATAINDEX		(FLASHCTL_BASE + 0x112c)
#define FLASHCTL_CMDDATA(n)		(FLASHCTL_BASE + 0x1130 + (n) * 4)
#define FLASHCTL_WEPROTA		(FLASHCTL_BASE + 0x11d0)

#define FLASHCTL_STATCMD		(FLASHCTL_BASE + 0x13d0)
#define STATCMD_CMDINPROGRESS		BIT(2)
#define STATCMD_CMDPASS			BIT(1)
#define STATCMD_CMDDONE			BIT(0)

#define FLASH_SECTOR_WORD 8
#define FLASH_SECTOR_SIZE 1024

static void nvm_unprotect_sector(const void *addr)
{
	unsigned int sector = 1 << ((unsigned int)addr >> 10);

	iow(FLASHCTL_WEPROTA, ior(FLASHCTL_WEPROTA) & ~sector);
}

void nvm_erase_sector(const void *addr)
{
	nvm_unprotect_sector(addr);

	iow(FLASHCTL_CMDTYPE, CMDTYPE_SIZE_SECTOR | CMDTYPE_CMD_ERASE);
	iow(FLASHCTL_CMDADDR, (unsigned int)addr & ~(FLASH_SECTOR_SIZE - 1));
	iow(FLASHCTL_CMDEXEC, 1);

	while (ior(FLASHCTL_STATCMD) & STATCMD_CMDINPROGRESS);

	iow(FLASHCTL_CMDTYPE, CMDTYPE_CMD_NOOP);
}

void nvm_program_word(const void *addr, const void *buf)
{
	const unsigned char *data = buf;
	unsigned int word0, word1;

	word0 = data[3] << 24 | data[2] << 16 | data[1] << 8 | data[0];
	word1 = data[7] << 24 | data[6] << 16 | data[5] << 8 | data[4];

	nvm_unprotect_sector(addr);

	iow(FLASHCTL_CMDTYPE, CMDTYPE_SIZE_1_WORD | CMDTYPE_CMD_PROGRAM);
	iow(FLASHCTL_CMDCTL, CMDCTL_REGIONSEL_MAIN);
	iow(FLASHCTL_CMDADDR, (unsigned int)addr & ~(FLASH_SECTOR_WORD - 1));
	iow(FLASHCTL_CMDBYTEN, 0xff);
	iow(FLASHCTL_CMDDATA(0), word0);
	iow(FLASHCTL_CMDDATA(1), word1);

	iow(FLASHCTL_CMDEXEC, 1);

	while (ior(FLASHCTL_STATCMD) & STATCMD_CMDINPROGRESS);

	iow(FLASHCTL_CMDTYPE, CMDTYPE_CMD_NOOP);
}
