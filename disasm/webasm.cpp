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
#include <stdint.h>
#include <string.h>
#include <inttypes.h>

#include "disasm/webasm.h"
#include "table/webasm.h"

static uint64_t get_varuint(
  Memory *memory,
  uint32_t address,
  int *length)
{
  uint8_t ch;
  int shift = 0;
  int done = 0;
  uint64_t num = 0;

  *length = 0;

  while (done == 0)
  {
    ch = memory->read8(address++);

    *length += 1;

    if ((ch & 0x80) == 0)
    {
      done = 1;
    }

    ch = ch & 0x7f;

    num |= ch << shift;
    shift += 7;
  }

  return num;
}

static int64_t get_varint(
  Memory *memory,
  uint32_t address,
  int *byte_count)
{
  uint8_t ch;
  int shift = 0;
  int done = 0;
  int64_t num = 0;

  *byte_count = 0;

  while (done == 0)
  {
    ch = memory->read8(address++);

    *byte_count += 1;

    if ((ch & 0x80) == 0)
    {
      done = 1;
    }

    ch = ch & 0x7f;

    num |= ch << shift;
    shift += 7;
  }

  if ((num & (1ULL << (shift - 1))) != 0)
  {
    num |= ~((1ULL << shift) - 1);
  }

  return num;
}

static const char *get_type(int type)
{
  int n = 0;

  while (webasm_types[n].name != NULL)
  {
    if (webasm_types[n].type == type)
    {
      return webasm_types[n].name;
    }

    n++;
  }

  return "???";
}

static int print_table(Memory *memory, uint32_t address, FILE *out)
{
  int byte_count = 0, total_length, entry, n, count = 0;

  count = get_varint(memory, address, &total_length);

  address += total_length;
  byte_count += count;

  for (n = 0; n < count; n++)
  {
    entry = get_varint(memory, address, &byte_count);

    fprintf(out, "    %04x\n", entry);

    address += byte_count;
    total_length += byte_count;
  }

  return total_length;
}

int disasm_webasm(
  Memory *memory,
  uint32_t address,
  char *instruction,
  int length,
  int flags,
  int *cycles_min,
  int *cycles_max)
{
  uint8_t opcode;
  uint64_t i;
  int64_t v;
  uint32_t count;
  int n, byte_count;

  instruction[0] = 0;

  *cycles_min = 0;
  *cycles_max = 0;

  opcode = memory->read8(address);

  n = 0;
  while (table_webasm[n].instr != NULL)
  {
    if (opcode != table_webasm[n].opcode)
    {
      n++;
      continue;
    }

    switch (table_webasm[n].type)
    {
      case WEBASM_OP_NONE:
        snprintf(instruction, length, "%s", table_webasm[n].instr);
        return 1;
      case WEBASM_OP_UINT32:
        i = memory->read32(address + 1);
        snprintf(instruction, length, "%s 0x%04" PRIx64, table_webasm[n].instr, i);
        return 5;
      case WEBASM_OP_UINT64:
        i = memory->read32(address + 1);
        i |= ((uint64_t)memory->read32(address + 5)) << 32;
        snprintf(instruction, length, "%s 0x%04" PRIx64, table_webasm[n].instr, i);
        return 9;
      case WEBASM_OP_VARINT64:
        v = get_varint(memory, address + 1, &byte_count);
        snprintf(instruction, length, "%s 0x%04" PRIx64, table_webasm[n].instr, v);
        return byte_count + 1;
      case WEBASM_OP_VARINT32:
        v = get_varint(memory, address + 1, &byte_count);
        snprintf(instruction, length, "%s 0x%04" PRIx64, table_webasm[n].instr, v);
        return byte_count + 1;
      case WEBASM_OP_FUNCTION_INDEX:
      case WEBASM_OP_LOCAL_INDEX:
      case WEBASM_OP_GLOBAL_INDEX:
        i = get_varuint(memory, address + 1, &byte_count);
        snprintf(instruction, length, "%s 0x%04" PRIx64, table_webasm[n].instr, i);
        return byte_count + 1;
      case WEBASM_OP_BLOCK_TYPE:
        i = get_varuint(memory, address + 1, &byte_count);
        snprintf(instruction, length, "%s %s", table_webasm[n].instr, get_type(i));
        return byte_count + 1;
      case WEBASM_OP_RELATIVE_DEPTH:
        v = get_varint(memory, address + 1, &byte_count);
        snprintf(instruction, length, "%s 0x%04" PRIx64, table_webasm[n].instr, v);
        return byte_count + 1;
      case WEBASM_OP_TABLE:
        count = get_varint(memory, address + 1, &byte_count);

        snprintf(instruction, length, "%s %d ...", table_webasm[n].instr, count);

        return byte_count + 1;
      case WEBASM_OP_INDIRECT:
      case WEBASM_OP_MEMORY_IMMEDIATE:
      default:
        return 1;
    }
  }

  return 1;
}

void list_output_webasm(
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

  count = disasm_webasm(
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

  opcode = memory->read8(start);

  if (opcode == 0x0e)
  {
    start += print_table(&asm_context->memory, start + 1, asm_context->list);
  }
}

void disasm_range_webasm(
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
    count = disasm_webasm(
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

    opcode = memory->read8(start);

    if (opcode == 0x0e)
    {
      start += print_table(memory, start + 1, stdout);
    }

    start += count;
  }
}

