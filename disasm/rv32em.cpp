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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "disasm/rv32em.h"
#include "table/rv32em.h"

//mw mod for rv32em !!!
const char *rv32em_reg_names[32] =
{
  "zero", "ra",  "sp",  "gp", "tp", "t0", "t1", "t2",
    "fp", "s1",  "a0",  "a1", "a2", "a3", "a4", "a5",
    "a6", "a7",  "s2",  "s3", "s4", "s5", "s6", "s7",
    "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

static int32_t permutate_branch(uint32_t opcode)
{
  int32_t immediate;

  immediate = ((opcode >> 31) & 0x1) << 12;
  immediate |= ((opcode >> 8) & 0xf) << 1;
  immediate |= ((opcode >> 7) & 0x1) << 11;
  immediate |= ((opcode >> 25) & 0x3f) << 5;
  if ((immediate & 0x1000) != 0) { immediate |= 0xffffe000; }

  return immediate;
}

static int32_t permutate_jal(uint32_t opcode)
{
  int32_t offset;

  offset = ((opcode >> 31) & 0x1) << 20;
  offset |= ((opcode >> 12) & 0xff) << 12;
  offset |= ((opcode >> 20) & 0x1) << 11;
  offset |= ((opcode >> 21) & 0x3ff) << 1;
  if ((offset & 0x100000) != 0) { offset |= 0xfff00000; }

  return offset;
}

int disasm_rv32em(
  Memory *memory,
  uint32_t address,
  char *instruction,
  int length,
  int flags,
  int *cycles_min,
  int *cycles_max)
{
  uint32_t opcode;
  uint32_t immediate;
  int32_t offset;
  int32_t simmediate;
  int n;
  char temp[16];
  int count, i;

  *cycles_min = -1;
  *cycles_max = -1;

  opcode = memory->read32(address);

  for (n = 0; table_rv32em[n].instr != NULL; n++)
  {
    if ((opcode & table_rv32em[n].mask) == table_rv32em[n].opcode)
    {
      uint32_t rd = (opcode >> 7) & 0x1f;
      uint32_t rs1 = (opcode >> 15) & 0x1f;
      uint32_t rs2 = (opcode >> 20) & 0x1f;
      uint32_t rs3 = (opcode >> 27) & 0x1f;
      uint32_t rm = (opcode >> 12) & 0x7;
      const char *instr = table_rv32em[n].instr;

      switch (table_rv32em[n].type)
      {
        case OP_NONE:
          snprintf(instruction, length, "%s", instr);
          break;
        case OP_R_TYPE:
          snprintf(instruction, length, "%s %s, %s, %s",
            instr,
            rv32em_reg_names[rd],
            rv32em_reg_names[rs1],
            rv32em_reg_names[rs2]);
          break;
        case OP_I_TYPE:
          immediate = opcode >> 20;
          simmediate = immediate;
          if ((simmediate & 0x800) != 0) { simmediate |= 0xfffff000; }
          snprintf(instruction, length, "%s %s, %s, %d (0x%06x)",
            instr,
            rv32em_reg_names[rd],
            rv32em_reg_names[rs1],
            simmediate,
            immediate);
          break;
        case OP_UI_TYPE:
          immediate = opcode >> 20;
          snprintf(instruction, length, "%s %s, 0x%x",
            instr,
            rv32em_reg_names[rd],
            immediate);
          break;
        case OP_SB_TYPE:
          immediate = permutate_branch(opcode);
          snprintf(instruction, length, "%s %s, %s, 0x%x (%d)",
            instr,
            rv32em_reg_names[rs1],
            rv32em_reg_names[rs2],
            address + immediate,
            immediate);
          break;
        case OP_U_TYPE:
          immediate = opcode >> 12;
          snprintf(instruction, length, "%s %s, 0x%06x",
            instr,
            rv32em_reg_names[rd],
            immediate);
          break;
        case OP_UJ_TYPE:
          offset = permutate_jal(opcode);
          snprintf(instruction, length, "%s %s, 0x%x (offset=%d)",
            instr,
            rv32em_reg_names[rd],
            address + offset,
            offset);
          break;
        case OP_SHIFT:
          immediate = (opcode >> 20) & 0x1f;
          snprintf(instruction, length, "%s %s, %s, %d",
            instr,
            rv32em_reg_names[rd],
            rv32em_reg_names[rs1],
            immediate);
          break;
        case OP_FFFF:
          snprintf(instruction, length, "%s", instr);
          break;
        case OP_RD_INDEX_R:
          immediate = opcode >> 20;
          simmediate = immediate;
          if ((simmediate & 0x800) != 0) { simmediate |= 0xfffff000; }
          snprintf(instruction, length, "%s %s, %d(%s)",
            instr,
            rv32em_reg_names[rd],
            simmediate,
            rv32em_reg_names[rs1]);
          break;
        case OP_RS_INDEX_R:
          immediate = ((opcode >> 25) & 0x7f) << 5;
          immediate |= ((opcode >> 7) & 0x1f);
          if ((immediate & 0x800) != 0) { immediate |= 0xfffff000; }
          snprintf(instruction, length, "%s %s, %d(%s)",
            instr,
            rv32em_reg_names[rs2],
            immediate,
            rv32em_reg_names[rs1]);
          break;
        case OP_LR:
          temp[0] = 0;
          if ((opcode & (1 << 26)) != 0) { strcat(temp, ".aq"); }
          if ((opcode & (1 << 25)) != 0) { strcat(temp, ".rl"); }
          snprintf(instruction, length, "%s%s %s, (%s)",
            instr,
            temp,
            rv32em_reg_names[rd],
            rv32em_reg_names[rs1]);
          break;
        case OP_STD_EXT:
          temp[0] = 0;
          if ((opcode & (1 << 26)) != 0) { strcat(temp, ".aq"); }
          if ((opcode & (1 << 25)) != 0) { strcat(temp, ".rl"); }
          // FIXME - The docs say rs2 and rs1 are reversed. gnu-as is like this.
          snprintf(instruction, length, "%s%s %s, %s, (%s)",
            instr,
            temp,
            rv32em_reg_names[rd],
            rv32em_reg_names[rs2],
            rv32em_reg_names[rs1]);
          break;
        case OP_ALIAS_RD_RS1:
          snprintf(instruction, length, "%s %s, %s",
            instr,
            rv32em_reg_names[rd],
            rv32em_reg_names[rs1]);
          break;
        case OP_ALIAS_RD_RS2:
          snprintf(instruction, length, "%s %s, %s",
            instr,
            rv32em_reg_names[rd],
            rv32em_reg_names[rs2]);
          break;
        case OP_ALIAS_BR_RS_X0:
          immediate = permutate_branch(opcode);
          snprintf(instruction, length, "%s %s, 0x%x (%d)",
            instr,
            rv32em_reg_names[rs1],
            address + immediate,
            immediate);
          break;
        case OP_ALIAS_BR_X0_RS:
          immediate = permutate_branch(opcode);
          snprintf(instruction, length, "%s %s, 0x%x (%d)",
            instr,
            rv32em_reg_names[rs2],
            address + immediate,
            immediate);
          break;
        case OP_ALIAS_BR_RS_RT:
          immediate = permutate_branch(opcode);
          snprintf(instruction, length, "%s %s, %s, 0x%x (%d)",
            instr,
            rv32em_reg_names[rs2],
            rv32em_reg_names[rs1],
            address + immediate,
            immediate);
          break;
        case OP_ALIAS_JAL:
          offset = permutate_jal(opcode);
          snprintf(instruction, length, "%s 0x%x (offset=%d)",
            instr,
            address + offset,
            offset);
          break;
        case OP_ALIAS_JALR:
          immediate = opcode >> 20;
          simmediate = immediate;
          if ((simmediate & 0x800) != 0) { simmediate |= 0xfffff000; }
          snprintf(instruction, length, "%s %s, %d (0x%06x)",
            instr,
            rv32em_reg_names[rs1],
            simmediate,
            immediate);
          break;
        case OP_RS1:
          snprintf(instruction, length, "%s %s", instr, rv32em_reg_names[rs1]);
          break;
        default:
          strcpy(instruction, "???");
          break;
      }

      return 4;
    }
  }

  strcpy(instruction, "???");

  return -1;
}

void list_output_rv32em(
  AsmContext *asm_context,
  uint32_t start,
  uint32_t end)
{
  int cycles_min,cycles_max;
  char instruction[128];
  uint32_t opcode;
  int count;

  Memory *memory = &asm_context->memory;

  fprintf(asm_context->list, "\n");

  while (start < end)
  {
    opcode = memory->read32(start);

    count = disasm_rv32em(
      memory,
      start,
      instruction,
      sizeof(instruction),
      asm_context->flags,
      &cycles_min,
      &cycles_max);

    if (count == 2)
    {
      fprintf(asm_context->list, "0x%08x: 0x%04x     %s\n",
        start,
        opcode,
        instruction);
    }
      else
    {
      fprintf(asm_context->list, "0x%08x: 0x%08x %s\n",
        start,
        opcode,
        instruction);
    }

#if 0
    if (count == 2)
    {
      fprintf(asm_context->list, "0x%08x: 0x%04x     %-40s%s",
        start,
        opcode,
        instruction,
        cycles_min != -1 ? " cycles: " : "");
    }
      else
    {
      fprintf(asm_context->list, "0x%08x: 0x%08x %-40s%s",
        start,
        opcode,
        instruction,
        cycles_min != -1 ? " cycles: " : "");
    }

    if (cycles_min == -1)
    {
      fprintf(asm_context->list, "\n");
    }
      else
    if (cycles_min == cycles_max)
    {
      fprintf(asm_context->list, "%d\n", cycles_min);
    }
      else
    {
      fprintf(asm_context->list, "%d-%d\n", cycles_min, cycles_max);
    }
#endif

    start += 4;
  }
}

void disasm_range_rv32em(
  Memory *memory,
  uint32_t flags,
  uint32_t start,
  uint32_t end)
{
  char instruction[128];
  uint32_t opcode;
  int cycles_min = 0, cycles_max = 0;
  int count;

  printf("\n");

  printf("%-7s %-5s %-40s Cycles\n", "Addr", "Opcode", "Instruction");
  printf("------- ------ ----------------------------------       ------\n");

  while (start <= end)
  {
    opcode = memory->read32(start);

    count = disasm_rv32em(
      memory,
      start,
      instruction,
      sizeof(instruction),
      0,
      &cycles_min,
      &cycles_max);

    if (count == 2)
    {
      printf("0x%08x: 0x%04x     %-40s cycles: ", start, opcode, instruction);
    }
      else
    {
      printf("0x%08x: 0x%08x %-40s cycles: ", start, opcode, instruction);
    }

    if (cycles_min == -1)
    {
      printf("\n");
    }
      else
    if (cycles_min == cycles_max)
    {
      printf("%d\n", cycles_min);
    }
      else
    {
      printf("%d-%d\n", cycles_min, cycles_max);
    }

    start = start + count;
  }
}

