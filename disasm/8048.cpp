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

#include "disasm/8048.h"
#include "table/8048.h"

int disasm_8048(
  Memory *memory,
  uint32_t address,
  char *instruction,
  int length,
  int flags,
  int *cycles_min,
  int *cycles_max)
{
  int opcode;
  char temp[32];
  int value, byte_count;
  int n, r;

  strcpy(instruction, "???");

  opcode = memory->read8(address);

  for (n = 0; table_8048[n].name != NULL; n++)
  {
    if (table_8048[n].flags != 0)
    {
      if ((table_8048[n].flags & flags) == 0) { continue; }
    }

    if (table_8048[n].opcode == (opcode & table_8048[n].mask))
    {
      snprintf(instruction, length, "%s", table_8048[n].name);

      *cycles_min = table_8048[n].cycles;
      *cycles_max = table_8048[n].cycles;

      if (table_8048[n].operand_count == 0)
      {
        return 1;
      }

      byte_count = 1;

      strcat(instruction, " ");

      for (r = 0; r < table_8048[n].operand_count; r++)
      {
        if (r == 1) { strcat(instruction, ", "); }

        int type = (r == 0) ? table_8048[n].operand_1 :
                              table_8048[n].operand_2;

        switch (type)
        {
          case OP_A:
            strcat(instruction, "A");
            break;
          case OP_C:
            strcat(instruction, "C");
            break;
          case OP_I:
            strcat(instruction, "I");
            break;
          case OP_T:
            strcat(instruction, "T");
            break;
          case OP_F0:
            strcat(instruction, "F0");
            break;
          case OP_F1:
            strcat(instruction, "F1");
            break;
          case OP_BUS:
            strcat(instruction, "BUS");
            break;
          case OP_CLK:
            strcat(instruction, "CLK");
            break;
          case OP_CNT:
            strcat(instruction, "CNT");
            break;
          case OP_MB0:
            strcat(instruction, "MB0");
            break;
          case OP_MB1:
            strcat(instruction, "MB1");
            break;
          case OP_RB0:
            strcat(instruction, "RB0");
            break;
          case OP_RB1:
            strcat(instruction, "RB1");
            break;
          case OP_PSW:
            strcat(instruction, "PSW");
            break;
          case OP_TCNT:
            strcat(instruction, "TCNT");
            break;
          case OP_TCNTI:
            strcat(instruction, "TCNTI");
            break;
          case OP_AT_A:
            strcat(instruction, "@A");
            break;
          case OP_PP:
          case OP_P03:
          case OP_P12:
            snprintf(temp, sizeof(temp), "p%d", opcode & 0x3);
            strcat(instruction, temp);
            break;
          case OP_P0:
            strcat(instruction, "p0");
            break;
          case OP_RR:
            snprintf(temp, sizeof(temp), "r%d", opcode & 0x7);
            strcat(instruction, temp);
            break;
          case OP_AT_R:
            snprintf(temp, sizeof(temp), "@r%d", opcode & 0x1);
            strcat(instruction, temp);
            break;
          case OP_NUM:
            value = memory->read8(address + 1);
            snprintf(temp, sizeof(temp), "#0x%02x", value);
            strcat(instruction, temp);
            byte_count = 2;
            break;
          case OP_ADDR:
            value = memory->read8(address + 1);
            snprintf(temp, sizeof(temp), "#0x%02x", ((opcode & 0xe000) >> 5) | value);
            strcat(instruction, temp);
            byte_count = 2;
            break;
          case OP_PADDR:
            value = memory->read8(address + 1);
            snprintf(temp, sizeof(temp), "#0x%02x", (address & 0xff00) | value);
            strcat(instruction, temp);
            byte_count = 2;
            break;
          case OP_DMA:
            strcat(instruction, "DMA");
            break;
          case OP_FLAGS:
            strcat(instruction, "FLAGS");
            break;
          case OP_STS:
            strcat(instruction, "STS");
            break;
          case OP_DBB:
            strcat(instruction, "DBB");
            break;
        }
      }

      return byte_count;
    }
  }

  return 1;
}

void list_output_8048(
  AsmContext *asm_context,
  uint32_t start,
  uint32_t end)
{
  int cycles_min = -1, cycles_max = -1, count;
  char instruction[128];
  char temp[32];
  char temp2[4];
  int n;

  Memory *memory = &asm_context->memory;

  while (start < end)
  {
    count = disasm_8048(
      memory,
      start,
      instruction,
      sizeof(instruction),
      asm_context->flags,
      &cycles_min,
      &cycles_max);

    temp[0] = 0;

    for (n = 0; n < count; n++)
    {
      snprintf(temp2, sizeof(temp2), "%02x ", memory->read8(start + n));
      strcat(temp, temp2);
    }

    fprintf(asm_context->list, "0x%04x: %-10s %-40s cycles: %d\n",
      start, temp, instruction, cycles_min);

    start += count;
  }
}

void disasm_range_8048(
  Memory *memory,
  uint32_t flags,
  uint32_t start,
  uint32_t end)
{
  char instruction[128];
  char temp[32];
  char temp2[4];
  int cycles_min = 0, cycles_max = 0;
  int count;
  int n;

  printf("\n");

  printf("%-7s %-5s %-40s Cycles\n", "Addr", "Opcode", "Instruction");
  printf("------- ------ ----------------------------------       ------\n");

  while (start <= end)
  {
    count = disasm_8048(
      memory,
      start,
      instruction,
      sizeof(instruction),
      flags,
      &cycles_min,
      &cycles_max);

    temp[0] = 0;
    for (n = 0; n < count; n++)
    {
      snprintf(temp2, sizeof(temp2), "%02x ", memory->read8(start + n));
      strcat(temp, temp2);
    }

    printf("0x%04x: %-10s %-40s %d\n", start, temp, instruction, cycles_min);

    start += count;
  }
}

