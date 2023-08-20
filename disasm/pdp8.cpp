/**
 *  naken_asm assembler.
 *  Author: Michael Kohn
 *   Email: mike@mikekohn.net
 *     Web: https://www.mikekohn.net/
 * License: GPLv3
 *
 * Copyright 2010-2023 by Michael Kohn, Lars Brinkhoff
 *
 * PDP-8 by Lars Brinkhoff
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "disasm/pdp8.h"
#include "table/pdp8.h"

static void disasm_pdp8_opr(char *instruction, int length, int opcode)
{
  int mask = 0;
  int n, x, y;

  x = opcode;

  switch (opcode & 0411)
  {
    case 0000:
    case 0001:
    case 0010:
    case 0011:
      mask = 0377;
      break;
    case 0400:
    case 0410:
      mask = 0366;
      break;
    case 0401:
    case 0411:
      mask = 0376;
      break;
  }

  while (1)
  {
    y = x;

    for (n = 0; table_pdp8[n].instr != NULL; n++)
    {
      if (table_pdp8[n].type != OP_NONE ||
          table_pdp8[n].opcode == 07000)
      {
        continue;
      }

      if ((table_pdp8[n].opcode & ~mask) == (opcode & ~mask) &&
          (table_pdp8[n].opcode & x) == table_pdp8[n].opcode)
        {
          int n = strlen(instruction);
          snprintf(instruction + n, length - n,
                  "%s ", table_pdp8[n].instr);
          x &= ~(table_pdp8[n].opcode & mask);
          break;
        }
    }

    if (y == x)
    {
      x &= mask;

      if ((x & 0777) != 0)
      {
        int n = strlen(instruction);
        snprintf(instruction + n, length - n, "%o", x & 0777);
      }

      return;
    }
  }
}

int disasm_pdp8(
  Memory *memory,
  uint32_t address,
  char *instruction,
  int length,
  int flags,
  int *cycles_min,
  int *cycles_max)
{
  int opcode;
  int n;
  int i, z, a;

  opcode = memory->read16(address);

  n = 0;

  while (table_pdp8[n].instr != NULL)
  {
    if ((opcode & table_pdp8[n].mask) == table_pdp8[n].opcode)
    {
      switch (table_pdp8[n].type)
      {
        case OP_NONE:
        {
          strcpy(instruction, table_pdp8[n].instr);
          return 2;
        }
        case OP_M:
        {
          a = opcode & 0177;
          z = opcode & 0200;
          i = opcode & 0400;

          if (z)
          {
            a |= (address / 2) & 07600;
          }

          snprintf(instruction, length, "%s %s%s%o", table_pdp8[n].instr,
                  z ? "z " : "", i ? "i ": "", a);

          if (i)
          {
            int n = strlen(instruction);
            snprintf(instruction + n, length - n,
                    " ; (%04o)", memory->read16(a << 1));
          }

          return 2;
        }
        case OP_IOT:
        {
          a = opcode & 0777;
          snprintf(instruction, length, "%s %o", table_pdp8[n].instr, a);
          return 2;
        }
        case OP_OPR:
        {
          instruction[0] = 0;
          disasm_pdp8_opr (instruction, length, opcode);
          return 2;
        }
        default:
        {
          //print_error_internal(asm_context, __FILE__, __LINE__);
          break;
        }
      }
    }

    n++;
  }

  strcpy(instruction, "???");

  return 2;
}

void list_output_pdp8(
  AsmContext *asm_context,
  uint32_t start,
  uint32_t end)
{
  uint32_t opcode;
  char instruction[128];
  int cycles_min = 0, cycles_max = 0;
  int count;

  Memory *memory = &asm_context->memory;

  fprintf(asm_context->list, "\n");

  while (start < end)
  {
    count = disasm_pdp8(
      memory,
      start,
      instruction,
      sizeof(instruction),
      asm_context->flags,
      &cycles_min,
      &cycles_max);

    opcode = memory->read16(start);

    fprintf(asm_context->list, "0%04o: %04o %s\n", start / 2, opcode, instruction);

    start += count;
  }
}

void disasm_range_pdp8(
  Memory *memory,
  uint32_t flags,
  uint32_t start,
  uint32_t end)
{
  char instruction[128];
  int cycles_min = 0, cycles_max = 0;
  uint16_t opcode;

  printf("\n");

  printf("%-5s  %-5s %s\n", "Addr", "Opcode", "Instruction");
  printf("-----  ------ ----------------------------------\n");

  while (start <= end)
  {
    disasm_pdp8(
      memory,
      start,
      instruction,
      sizeof(instruction),
      0,
      &cycles_min,
      &cycles_max);

    opcode = memory->read16(start);

    printf("0%04o: %04o   %s\n", start / 2, opcode, instruction);

    start = start + 2;
  }
}

