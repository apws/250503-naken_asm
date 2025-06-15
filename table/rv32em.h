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

#ifndef NAKEN_ASM_TABLE_RV32EM_H
#define NAKEN_ASM_TABLE_RV32EM_H

#include "common/assembler.h"

enum
{
  OP_NONE,
  OP_R_TYPE,
  OP_I_TYPE,
  OP_UI_TYPE,
  OP_SB_TYPE,
  OP_U_TYPE,
  OP_UJ_TYPE,
  OP_SHIFT,
  OP_FENCE,
  OP_FFFF,
  OP_RD_INDEX_R,
  OP_RS_INDEX_R,
  OP_LR,
  OP_STD_EXT,
  OP_ALIAS_RD_RS1,
  OP_ALIAS_RD_RS2,
  OP_ALIAS_BR_RS_X0,
  OP_ALIAS_BR_X0_RS,
  OP_ALIAS_BR_RS_RT,
  OP_ALIAS_JAL,
  OP_ALIAS_JALR,
  // Privileged instructions.
  OP_RS1,
};

struct _table_rv32em
{
  const char *instr;
  uint32_t opcode;
  uint32_t mask;
  uint8_t type;
  uint8_t flags;
};

extern struct _table_rv32em table_rv32em[];

#endif

