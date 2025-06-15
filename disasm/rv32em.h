/**
 *  naken_asm assembler.
 *  Author: Michael Kohn
 *   Email: mike@mikekohn.net
 *     Web: https://www.mikekohn.net/
 * License: GPLv3
 *
 * Copyright 2010-2023 by Michael Kohn
 *
 */

#ifndef NAKEN_ASM_DISASM_RV32EM_H
#define NAKEN_ASM_DISASM_RV32EM_H

#include "common/assembler.h"
//#include "table/rv32em.h"

extern const char *rv32em_reg_names[32];

int disasm_rv32em(
  Memory *memory,
  uint32_t address,
  char *instruction,
  int length,
  int flags,
  int *cycles_min,
  int *cycles_max);

void list_output_rv32em(
  AsmContext *asm_context,
  uint32_t start,
  uint32_t end);

void disasm_range_rv32em(
  Memory *memory,
  uint32_t flags,
  uint32_t start,
  uint32_t end);

#endif

