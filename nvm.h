// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2025 Kontron Europe GmbH
 */

#ifndef __NVM_H
#define __NVM_H

void nvm_erase_sector(const void *addr);
void nvm_program_word(const void *addr, const void *buf);

#endif /* __NVM_H */
