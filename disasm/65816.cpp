/**
 *  naken_asm assembler.
 *  Author: Michael Kohn
 *   Email: mike@mikekohn.net
 *     Web: https://www.mikekohn.net/
 * License: GPLv3
 *
 * Copyright 2010-2023 by Michael Kohn, Joe Davisson
 *
 * 65816 by Joe Davisson
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "disasm/65816.h"
#include "table/65816.h"

#define READ_RAM(a) (memory->read8(a) & 0xff)

// bytes for each addressing mode
static int op_bytes[] =
{
  1, 2, 3, 2, 3, 4, 2, 2, 3, 3, 4, 2,
  2, 3, 3, 2, 3, 2, 2, 3, 2, 3, 2, 2
};

int disasm_65816(
  Memory *memory,
  uint32_t address,
  char *instruction,
  int length,
  int flags,
  int *cycles_min,
  int *cycles_max)
{
  char temp[128];
  char num[64];
  uint8_t opcode = memory->read8(address);

  int op = 0;
  int lo = 0, hi = 0, bank = 0;
  int branch_address = 0;

  // FIXME: This used to be a parameter on listings calculated by
  // end - start + 1.
  int bytes = 0;

  *cycles_min = -1;
  *cycles_max = -1;
  opcode = READ_RAM(address);

  snprintf(temp, sizeof(temp), " ");

  strcpy(instruction, table_65816[table_65816_opcodes[opcode].instr].name);
  op = table_65816_opcodes[opcode].op;

  if (bytes == 0)
  {
    bytes = op_bytes[op];
  }
    else
  {
    bytes = bytes - 1;
  }

  if (bytes > 1)
  {
    if (bytes == 2)
    {
      lo = READ_RAM(address + 1);

      // special case for branches
      if (op == OP_RELATIVE)
      {
        int8_t offset = (int8_t)lo;

        branch_address = (address + 2) + offset;
        snprintf(num, sizeof(num), "0x%04x (offset=%d)", branch_address, offset);
      }
        else
      {
        snprintf(num, sizeof(num), "0x%02x", lo);
      }
    }
      else
    if (bytes == 3)
    {
      lo = READ_RAM(address + 1);
      hi = READ_RAM(address + 2);
      // special case for long branch (BRL)
      if (op == OP_RELATIVE_LONG)
      {
        int16_t offset = (int16_t)((hi << 8) | lo);

        branch_address = (address + 3) + offset;
        snprintf(num, sizeof(num), "0x%04x (offset=%d)", branch_address, offset);
      }
        else
      {
        snprintf(num, sizeof(num), "0x%04x", (hi << 8) | lo);
      }
    }
      else
    if (bytes == 4)
    {
      lo = READ_RAM(address + 1);
      hi = READ_RAM(address + 2);
      bank = READ_RAM(address + 3);
      snprintf(num, sizeof(num), "0x%06x", (bank << 16) | (hi << 8) | lo);
    }

    switch (op)
    {
      case OP_NONE:
        snprintf(temp, sizeof(temp), " ");
        break;
      case OP_IMMEDIATE8:
      case OP_IMMEDIATE16:
        snprintf(temp, sizeof(temp), " #%s", num);
        break;
      case OP_ADDRESS8:
      case OP_ADDRESS16:
      case OP_ADDRESS24:
        snprintf(temp, sizeof(temp), " %s", num);
        break;
      case OP_INDEXED8_X:
      case OP_INDEXED16_X:
      case OP_INDEXED24_X:
        snprintf(temp, sizeof(temp), " %s,x", num);
        break;
      case OP_INDEXED8_Y:
      case OP_INDEXED16_Y:
        snprintf(temp, sizeof(temp), " %s,y", num);
        break;
      case OP_INDIRECT8:
      case OP_INDIRECT16:
        snprintf(temp, sizeof(temp), " (%s)", num);
        break;
      case OP_INDIRECT8_LONG:
      case OP_INDIRECT16_LONG:
        snprintf(temp, sizeof(temp), " [%s]", num);
        break;
      case OP_X_INDIRECT8:
      case OP_X_INDIRECT16:
        snprintf(temp, sizeof(temp), " (%s,x)", num);
        break;
      case OP_INDIRECT8_Y:
        snprintf(temp, sizeof(temp), " (%s),y", num);
        break;
      case OP_INDIRECT8_Y_LONG:
        snprintf(temp, sizeof(temp), " [%s],y", num);
        break;
      case OP_BLOCK_MOVE:
        snprintf(temp, sizeof(temp), " %0x02x,%0x02x", lo, hi);
        break;
      case OP_RELATIVE:
      case OP_RELATIVE_LONG:
        snprintf(temp, sizeof(temp), " %s", num);
        break;
      case OP_SP_RELATIVE:
        snprintf(temp, sizeof(temp), " %s,s", num);
        break;
      case OP_SP_INDIRECT_Y:
        snprintf(temp, sizeof(temp), " (%s,s),y", num);
        break;
      default:
        strcat(instruction, " ???");
        break;
    }

    strcat(instruction, temp);
  }

  // set this to the number of bytes the operation took up
  return bytes;

    // get cycle mode
/*
    int min = table_65xx_opcodes[opcode].cycles_min;
    int max = table_65xx_opcodes[opcode].cycles_max;

    if (op == OP_RELATIVE)
    {
      // branch, see if we're in the same page
      int page1 = (address + 2) / 256;
      int page2 = branch_address / 256;
      if (page1 != page2)
        max += 2;
      else
        max += 1;
    }

    *cycles_min = min;
    *cycles_max = max;
*/
}

void list_output_65816(
  AsmContext *asm_context,
  uint32_t start,
  uint32_t end)
{
  char instruction[128];
  char bytes[32];
  int cycles_min,cycles_max;
  int count = end - start;
  int n;

  Memory *memory = &asm_context->memory;

  disasm_65816(
    memory,
    start,
    instruction,
    sizeof(instruction),
    asm_context->flags,
    &cycles_min,
    &cycles_max);

  bytes[0] = 0;

  for (n = 0; n < count; n++)
  {
    char temp[4];
    snprintf(temp, sizeof(temp), "%02x ", memory->read8(start + n));
    strcat(bytes, temp);
  }

  fprintf(asm_context->list, "0x%04x: %-16s %-40s cycles: ?\n", start, bytes, instruction);

#if 0
  if (cycles_min == cycles_max)
  { fprintf(asm_context->list, "%d\n", cycles_min); }
    else
  { fprintf(asm_context->list, "%d-%d\n", cycles_min, cycles_max); }
#endif
}

void disasm_range_65816(
  Memory *memory,
  uint32_t flags,
  uint32_t start,
  uint32_t end)
{
  char instruction[128];
  int cycles_min = 0, cycles_max = 0;
  int num = 0;

  printf("\n");

  printf("%-7s %-5s %-40s Cycles\n", "Addr", "Opcode", "Instruction");
  printf("------- ------ ----------------------------------       ------\n");

  while (start <= end)
  {
    num = READ_RAM(start) | (READ_RAM(start+1) << 8);

    int count = disasm_65816(
      memory,
      start,
      instruction,
      sizeof(instruction),
      0,
      &cycles_min,
      &cycles_max);

    if (cycles_min < 1)
    {
      printf("0x%04x: 0x%02x %-40s ?\n", start, num & 0xff, instruction);
    }
      else
    if (cycles_min == cycles_max)
    {
      printf("0x%04x: 0x%02x %-40s %d\n", start, num & 0xff, instruction, cycles_min);
    }
      else
    {
      printf("0x%04x: 0x%02x %-40s %d-%d\n", start, num & 0xff, instruction, cycles_min, cycles_max);
    }

    count -= 1;
    while (count > 0)
    {
      start = start + 1;
      num = READ_RAM(start) | (READ_RAM(start + 1) << 8);
      printf("0x%04x: 0x%02x\n", start, num & 0xff);
      count -= 1;
    }

    start = start + 1;
  }
}

