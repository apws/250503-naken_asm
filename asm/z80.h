/**
 *  naken_asm assembler.
 *  Author: Michael Kohn
 *   Email: mike@mikekohn.net
 *     Web: https://www.mikekohn.net/
 * License: GPLv3
 *
 * Copyright 2010-2024 by Michael Kohn
 *
 */

#ifndef NAKEN_ASM_ASM_Z80_H
#define NAKEN_ASM_ASM_Z80_H

#include "common/assembler.h"

int parse_instruction_z80(AsmContext *asm_context, char *instr);

#endif

