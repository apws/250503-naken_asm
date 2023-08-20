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

#include "disasm/ps2_ee_vu.h"

int disasm_ps2_ee_vu(
  Memory *memory,
  uint32_t address,
  char *instruction,
  int length,
  int flags,
  int *cycles_min,
  int *cycles_max)
{
  struct _table_ps2_ee_vu *table_ps2_ee_vu;
  uint32_t opcode;
  int n, r;
  char temp[32];
  int ft, fs, fd, dest;
  int16_t offset;
  int immediate;
  const char *scalar[] = { "x", "y", "z", "w" };
  char bits[8];

  bits[0] = 0;

  *cycles_min = -1;
  *cycles_max = -1;
  opcode = memory->read32(address);

  bool is_lower = (address % 8) == 0;

  if (is_lower == false)
  {
    table_ps2_ee_vu = table_ps2_ee_vu_upper;

    int temp = opcode >> 27;

    if (temp != 0)
    {
      snprintf(bits, sizeof(bits), "[%s%s%s%s%s]",
        (temp & 0x10) != 0 ? "I" : "",
        (temp & 0x08) != 0 ? "E" : "",
        (temp & 0x04) != 0 ? "M" : "",
        (temp & 0x02) != 0 ? "D" : "",
        (temp & 0x01) != 0 ? "T" : "");
    }
  }
    else
  {
    table_ps2_ee_vu = table_ps2_ee_vu_lower;
  }

  instruction[0] = 0;

  n = 0;
  while (table_ps2_ee_vu[n].instr != NULL)
  {
    if (flags == PS2_EE_VU0 && (table_ps2_ee_vu[n].flags & FLAG_VU1_ONLY))
    {
      n++;
      continue;
    }

    if (table_ps2_ee_vu[n].opcode == (opcode & table_ps2_ee_vu[n].mask))
    {
      strcpy(instruction, table_ps2_ee_vu[n].instr);

      if (bits[0] != 0)
      {
        strcat(instruction, bits);
      }

      dest = (opcode >> 21) & 0xf;
      ft = (opcode >> 16) & 0x1f;
      fs = (opcode >> 11) & 0x1f;
      fd = (opcode >> 6) & 0x1f;

      if ((table_ps2_ee_vu[n].flags & FLAG_DEST) != 0)
      {
        strcat(instruction, ".");
        if ((dest & 8) != 0) { strcat(instruction, "x"); }
        if ((dest & 4) != 0) { strcat(instruction, "y"); }
        if ((dest & 2) != 0) { strcat(instruction, "z"); }
        if ((dest & 1) != 0) { strcat(instruction, "w"); }
      }

      for (r = 0; r < table_ps2_ee_vu[n].operand_count; r++)
      {
        if (r != 0) { strcat(instruction, ","); }

        switch (table_ps2_ee_vu[n].operand[r])
        {
          case EE_VU_OP_FT:
            snprintf(temp, sizeof(temp), " vf%02d", ft);
            if ((table_ps2_ee_vu[n].flags & FLAG_TE) != 0)
            {
              strcat(temp, scalar[(dest >> 2) & 0x3]);
            }
            break;
          case EE_VU_OP_FS:
            snprintf(temp, sizeof(temp), " vf%02d", fs);
            if ((table_ps2_ee_vu[n].flags & FLAG_SE) != 0)
            {
              strcat(temp, scalar[dest & 0x3]);
            }
            break;
          case EE_VU_OP_FD:
            snprintf(temp, sizeof(temp), " vf%02d", fd);
            break;
          case EE_VU_OP_VIT:
            snprintf(temp, sizeof(temp), " vi%02d", ft);
            break;
          case EE_VU_OP_VIS:
            snprintf(temp, sizeof(temp), " vi%02d", fs);
            break;
          case EE_VU_OP_VID:
            snprintf(temp, sizeof(temp), " vi%02d", fd);
            break;
          case EE_VU_OP_VI01:
            snprintf(temp, sizeof(temp), " vi01");
            break;
          case EE_VU_OP_I:
            strcpy(temp, " I");
            break;
          case EE_VU_OP_Q:
            strcpy(temp, " Q");
            break;
          case EE_VU_OP_P:
            strcpy(temp, " P");
            break;
          case EE_VU_OP_R:
            strcpy(temp, " R");
            break;
          case EE_VU_OP_ACC:
            strcpy(temp, " ACC");
            break;
          case EE_VU_OP_OFFSET:
            offset = (opcode & 0x7ff) << 3;
            if ((offset & 0x400) != 0) { offset |= 0xf800; }
            snprintf(temp, sizeof(temp), " 0x%x (offset=%d)", address + 8 + offset, offset);
            break;
          case EE_VU_OP_OFFSET_BASE:
          case EE_VU_OP_OFFSET_BASE_1:
            offset = opcode & 0x7ff;
            if ((offset & 0x400) != 0) { offset |= 0xf800; }
            snprintf(temp, sizeof(temp), " %d(vi%02d)", offset, (opcode >> 11) & 0x1f);
            break;
          case EE_VU_OP_BASE:
          case EE_VU_OP_BASE_1:
            snprintf(temp, sizeof(temp), " (vi%02d)", fs);
            break;
          case EE_VU_OP_BASE_DEC:
            if (table_ps2_ee_vu[n].operand[0] == EE_VU_OP_FS)
            {
              snprintf(temp, sizeof(temp), " (--vi%02d)", ft);
            }
              else
            {
              snprintf(temp, sizeof(temp), " (--vi%02d)", fs);
            }
            break;
          case EE_VU_OP_BASE_INC:
            if (table_ps2_ee_vu[n].operand[0] == EE_VU_OP_FS)
            {
              snprintf(temp, sizeof(temp), " (vi%02d++)", ft);
            }
              else
            {
              snprintf(temp, sizeof(temp), " (vi%02d++)", fs);
            }
            break;
          case EE_VU_OP_IMMEDIATE24:
            snprintf(temp, sizeof(temp), " 0x%06x", opcode & 0xffffff);
            break;
          case EE_VU_OP_IMMEDIATE15:
            immediate = (opcode & (0xf << 21)) >> 10;
            immediate |= opcode & 0x7ff;
            snprintf(temp, sizeof(temp), " 0x%04x", immediate);
            break;
          case EE_VU_OP_IMMEDIATE12:
            immediate = (opcode & (1 << 21)) >> 10;
            immediate |= opcode & 0x7ff;
            snprintf(temp, sizeof(temp), " 0x%03x", immediate);
            break;
          case EE_VU_OP_IMMEDIATE5:
            immediate = (opcode >> 6) & 0x1f;
            if ((immediate & 0x10) != 0) { immediate |= 0xfffffff0; }
            snprintf(temp, sizeof(temp), " %d", immediate);
            break;
          default:
            strcpy(temp, " ?");
            break;
        }

        strcat(instruction, temp);
      }

      return 4;
    }

    n++;
  }

  strcpy(instruction, "???");

  return 4;
}

void list_output_ps2_ee_vu(
  AsmContext *asm_context,
  uint32_t start,
  uint32_t end)
{
  int cycles_min,cycles_max;
  char instruction_upper[128];
  char instruction_lower[128];
  uint32_t opcode_upper;
  uint32_t opcode_lower;

  Memory *memory = &asm_context->memory;

  fprintf(asm_context->list, "\n");

  while (start < end)
  {
    opcode_upper = memory->read32(start + 4);
    opcode_lower = memory->read32(start + 0);

    disasm_ps2_ee_vu(
      memory,
      start + 4,
      instruction_upper,
      sizeof(instruction_upper),
      asm_context->flags,
      &cycles_min,
      &cycles_max);

    disasm_ps2_ee_vu(
      memory,
      start,
      instruction_lower,
      sizeof(instruction_lower),
      asm_context->flags,
      &cycles_min,
      &cycles_max);

    fprintf(asm_context->list, "0x%08x: 0x%08x 0x%08x %-20s %s\n", start, opcode_upper, opcode_lower, instruction_upper, instruction_lower);

#if 0
    fprintf(asm_context->list, "0x%08x: 0x%08x %-40s cycles: ", start, opcode, instruction);

    if (cycles_min == cycles_max)
    { fprintf(asm_context->list, "%d\n", cycles_min); }
      else
    { fprintf(asm_context->list, "%d-%d\n", cycles_min, cycles_max); }

#endif
    start += 8;
  }
}

void disasm_range_ps2_ee_vu(
  Memory *memory,
  uint32_t flags,
  uint32_t start,
  uint32_t end)
{
  char instruction_upper[128];
  char instruction_lower[128];
  uint32_t opcode_upper;
  uint32_t opcode_lower;
  int cycles_min = 0,cycles_max = 0;

  printf("\n");

  printf("%-7s %-5s %-40s Cycles\n", "Addr", "Opcode", "Instruction");
  printf("------- ------ ----------------------------------       ------\n");

  while (start < end)
  {
    opcode_upper = memory->read32(start + 4);
    opcode_lower = memory->read32(start);

    disasm_ps2_ee_vu(
      memory,
      start + 4,
      instruction_upper,
      sizeof(instruction_upper),
      flags,
      &cycles_min,
      &cycles_max);

    disasm_ps2_ee_vu(
      memory,
      start,
      instruction_lower,
      sizeof(instruction_lower),
      flags,
      &cycles_min,
      &cycles_max);

    printf("0x%08x: 0x%08x 0x%08x %-20s %s\n", start, opcode_upper, opcode_lower, instruction_upper, instruction_lower);

#if 0
    if (cycles_min < 1)
    {
      printf("0x%04x: 0x%08x %-40s ?\n", start, num, instruction);
    }
      else
    if (cycles_min == cycles_max)
    {
      printf("0x%04x: 0x%08x %-40s %d\n", start, num, instruction, cycles_min);
    }
      else
    {
      printf("0x%04x: 0x%08x %-40s %d-%d\n", start, num, instruction, cycles_min, cycles_max);
    }
#endif

    start += 8;
  }
}

