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

#include "disasm/dotnet.h"
#include "table/dotnet.h"

int disasm_dotnet(
  Memory *memory,
  uint32_t address,
  char *instruction,
  int length,
  int flags,
  int *cycles_min,
  int *cycles_max)
{
  uint8_t opcode;
  int n;

  instruction[0] = 0;

  *cycles_min = 0;
  *cycles_max = 0;

  opcode = memory->read8(address);

  if (opcode == 0xfe)
  {
    opcode = memory->read8(address + 1);

    n = 0;
    while (table_dotnet_fe[n].instr != NULL)
    {
      if (opcode != table_dotnet_fe[n].opcode)
      {
        n++;
        continue;
      }

      switch (table_dotnet_fe[n].type)
      {
        case DOTNET_OP_NONE:
          snprintf(instruction, length, "%s", table_dotnet_fe[n].instr);
          return 2;
        default:
          return 2;
      }
    }
  }

  n = 0;
  while (table_dotnet[n].instr != NULL)
  {
    if (opcode != table_dotnet[n].opcode)
    {
      n++;
      continue;
    }

    switch (table_dotnet[n].type)
    {
      case DOTNET_OP_NONE:
        snprintf(instruction, length, "%s", table_dotnet[n].instr);
        return 1;
      default:
        return 1;
    }
  }

  return 1;
}

void list_output_dotnet(
  AsmContext *asm_context,
  uint32_t start,
  uint32_t end)
{
  int cycles_min, cycles_max;
  char instruction[128];
  int opcode, count;
  char hex[80];
  char temp[16];
  int n;

  Memory *memory = &asm_context->memory;

  count = disasm_dotnet(
    memory,
    start,
    instruction,
    sizeof(instruction),
    asm_context->flags,
    &cycles_min,
    &cycles_max);

  hex[0] = 0;

  for (n = 0; n < count; n++)
  {
    opcode = memory->read8(start + n);

    snprintf(temp, sizeof(temp), "%02x ", opcode);
    strcat(hex, temp);
  }

  fprintf(asm_context->list, "0x%04x: %-20s %-40s\n", start, hex, instruction);
}

void disasm_range_dotnet(
  Memory *memory,
  uint32_t flags,
  uint32_t start,
  uint32_t end)
{
  char instruction[128];
  int cycles_min = 0, cycles_max = 0;
  int opcode, count;
  char hex[80];
  char temp[16];
  int n;

  printf("\n");

  printf("%-7s %-5s %-40s Cycles\n", "Addr", "Opcode", "Instruction");
  printf("------- ------ ----------------------------------       ------\n");

  while (start <= end)
  {
    count = disasm_dotnet(
      memory,
      start,
      instruction,
      sizeof(instruction),
      0,
      &cycles_min,
      &cycles_max);

     hex[0] = 0;

     for (n = 0; n < count; n++)
     {
       opcode = memory->read8(start + n);

       snprintf(temp, sizeof(temp), "%02x ", opcode);
       strcat(hex, temp);
     }

    printf("0x%04x: %-20s %-40s\n", start, hex, instruction);

    start += count;
  }
}

