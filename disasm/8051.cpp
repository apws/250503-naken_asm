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

#include "disasm/8051.h"
#include "table/8051.h"

#define READ_RAM(a) memory->read8(a)

int disasm_8051(
  Memory *memory,
  uint32_t address,
  char *instruction,
  int length,
  int flags,
  int *cycles_min,
  int *cycles_max)
{
  int count = 1;
  int opcode;
  char temp[32];
  int value;
  int n;

  opcode = READ_RAM(address);

  // The mov iram_addr, iram_addr instruction didn't fit the format of
  // the table and how the rest of the instructions were done.
  if (opcode == 0x85)
  {
    snprintf(instruction, length, "%s 0x%02x, 0x%02x",
      table_8051[opcode].name,
      READ_RAM(address + 1),
      READ_RAM(address + 2));
    return 3;
  }

  strcpy(instruction, table_8051[opcode].name);

  for (n = 0; n < 3; n++)
  {
    if (table_8051[opcode].op[n] == OP_NONE) break;

    if (n == 0)
    {
      strcat(instruction, " ");
    }
      else
    {
      strcat(instruction, ", ");
    }

    switch (table_8051[opcode].op[n])
    {
      case OP_REG:
        snprintf(temp, sizeof(temp), "R%d", table_8051[opcode].range);
        strcat(instruction, temp);
        break;
      case OP_AT_REG:
        snprintf(temp, sizeof(temp), "@R%d", table_8051[opcode].range);
        strcat(instruction, temp);
        break;
      case OP_A:
        strcat(instruction, "A");
        break;
      case OP_C:
        strcat(instruction, "C");
        break;
      case OP_AB:
        strcat(instruction, "AB");
        break;
      case OP_DPTR:
        strcat(instruction, "DPTR");
        break;
      case OP_AT_A_PLUS_DPTR:
        strcat(instruction, "@A+DPTR");
        break;
      case OP_AT_A_PLUS_PC:
        strcat(instruction, "@A+PC");
        break;
      case OP_AT_DPTR:
        strcat(instruction, "@DPTR");
        break;
      case OP_DATA:
        snprintf(temp, sizeof(temp), "#0x%02x", READ_RAM(address + count));
        strcat(instruction, temp);
        count++;
        break;
      case OP_DATA_16:
        snprintf(temp, sizeof(temp), "#0x%04x", READ_RAM(address + count + 1) | (READ_RAM(address + count) << 8));
        strcat(instruction, temp);
        count = 3;
        break;
      case OP_CODE_ADDR:
        snprintf(temp, sizeof(temp), "0x%04x", READ_RAM(address + count + 1) | (READ_RAM(address + count) << 8));
        strcat(instruction, temp);
        count = 3;
        break;
      case OP_RELADDR:
        value = READ_RAM(address + count);
        snprintf(temp, sizeof(temp), "0x%04x", (address + count + 1) + ((char)value));
        strcat(instruction, temp);
        count++;
        break;
      case OP_SLASH_BIT_ADDR:
        value = READ_RAM(address + count);
        if ((value & 0x80) != 0)
        {
          snprintf(temp, sizeof(temp), "/0x%02x.%d [0x%02x]",
            value & 0xf8, value & 0x7, value);
        }
          else
        {
          snprintf(temp, sizeof(temp), "/0x%02x.%d [0x%02x]",
            ((value & 0xf8) >> 3) + 0x20, value & 0x7, value);
        }
        strcat(instruction, temp);
        count++;
        break;
      case OP_BIT_ADDR:
        value = READ_RAM(address + count);
        if ((value & 0x80) != 0)
        {
          snprintf(temp, sizeof(temp), "0x%02x.%d [0x%02x]",
            value & 0xf8, value & 0x07, value);
        }
          else
        {
          snprintf(temp, sizeof(temp), "0x%02x.%d [0x%02x]",
            ((value & 0xf8) >> 3) + 0x20, value & 0x07, value);
        }
        strcat(instruction, temp);
        count++;
        break;
      case OP_PAGE:
        snprintf(temp, sizeof(temp), "0x%04x",
          (address & 0xf800) |
          READ_RAM(address + count) |
          (table_8051[opcode].range << 8));
        strcat(instruction, temp);
        count++;
        break;
      case OP_IRAM_ADDR:
        snprintf(temp, sizeof(temp), "0x%02x", READ_RAM(address + count));
        strcat(instruction, temp);
        count++;
        break;
    }
  }

  return count;
}

void list_output_8051(
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

  fprintf(asm_context->list, "\n");

  while (start < end)
  {
    count = disasm_8051(
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

    fprintf(asm_context->list, "0x%04x: %-10s %-40s cycles:", start, temp, instruction);

    start += count;
  }
}

void disasm_range_8051(
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
    count = disasm_8051(
      memory,
      start,
      instruction,
      sizeof(instruction),
      0,
      &cycles_min,
      &cycles_max);

    temp[0] = 0;

    for (n = 0; n < count; n++)
    {
      snprintf(temp2, sizeof(temp2), "%02x ", memory->read8(start + n));
      strcat(temp, temp2);
    }

    if (cycles_min < 1)
    {
      printf("0x%04x: %-10s %-40s ?\n", start, temp, instruction);
    }
      else
    if (cycles_min == cycles_max)
    {
      printf("0x%04x: %-10s %-40s %d\n", start, temp, instruction, cycles_min);
    }
      else
    {
      printf("0x%04x: %-10s %-40s %d-%d\n", start, temp, instruction, cycles_min, cycles_max);
    }

    start += count;
  }
}

