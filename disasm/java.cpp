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

#include "disasm/java.h"
#include "table/java.h"

const char *array_types[] =
{
  "",
  "",
  "",
  "",
  "boolean",
  "char",
  "float",
  "double",
  "byte",
  "short",
  "int",
  "long",
};

static const char *get_array_type(int index)
{
  if (index > (int)(sizeof(array_types) / sizeof(char *)))
  {
    return "";
  }

  return array_types[index];
}

int disasm_java(
  Memory *memory,
  uint32_t address,
  char *instruction,
  int length,
  int flags,
  int *cycles_min,
  int *cycles_max)
{
  uint16_t index;
  uint8_t opcode;
  int8_t const8;
  int8_t index8;
  int16_t offset16;
  int offset;
  int wide = 0;

  *cycles_min = 0;
  *cycles_max = 0;

  opcode = memory->read8(address);

  if (opcode == 0xc4)
  {
    wide = 1;
    opcode = memory->read8(address + 1);
  }

  switch (table_java[opcode].op_type)
  {
    case JAVA_OP_ILLEGAL:
    case JAVA_OP_NONE:
      // Note: These instructions should never be wide.
      snprintf(instruction, length, "%s", table_java[opcode].instr);
      return wide + 1;
    case JAVA_OP_CONSTANT_INDEX8:
      // Note: These instructions should never be wide.
      index8 = memory->read8(address + wide + 1);
      snprintf(instruction, length, "%s %d", table_java[opcode].instr, index8);
      return wide + 2;
    case JAVA_OP_CONSTANT_INDEX:
    case JAVA_OP_FIELD_INDEX:
    case JAVA_OP_INTERFACE_INDEX:
    case JAVA_OP_METHOD_INDEX:
    case JAVA_OP_CLASS_INDEX:
    case JAVA_OP_SPECIAL_INDEX:
    case JAVA_OP_STATIC_INDEX:
    case JAVA_OP_VIRTUAL_INDEX:
      // Note: These instructions should never be wide.
      index = memory->read16(address + wide + 1);
      snprintf(instruction, length, "%s %d", table_java[opcode].instr, index);
      return wide + 3;
    case JAVA_OP_LOCAL_INDEX:
      if (wide == 0)
      {
        index = memory->read8(address + wide + 1);
        snprintf(instruction, length, "%s %d", table_java[opcode].instr, index);
        return wide + 2;
      }
        else
      {
        index = memory->read16(address + wide + 1);
        snprintf(instruction, length, "%s %d", table_java[opcode].instr, index);
        return wide + 3;
      }

    case JAVA_OP_LOCAL_INDEX_CONST:
      if (wide == 0)
      {
        index = memory->read8(address + wide + 1);
        const8 = memory->read8(address + wide + 2);
        snprintf(instruction, length, "%s %d, %d", table_java[opcode].instr, index, const8);
        return wide + 3;
      }
        else
      {
        index = memory->read16(address + wide + 1);
        const8 = memory->read8(address + wide + 3);
        snprintf(instruction, length, "%s %d, %d", table_java[opcode].instr, index, const8);
        return wide + 4;
      }
    case JAVA_OP_ARRAY_TYPE:
      // Note: These instructions should never be wide.
      index = memory->read8(address + wide + 1);
      snprintf(instruction, length, "%s %d (%s)", table_java[opcode].instr, index, get_array_type(index));
      return wide + 2;
    case JAVA_OP_CONSTANT16:
      // Note: These instructions should never be wide.
      index = memory->read16(address + wide + 1);
      snprintf(instruction, length, "%s %d", table_java[opcode].instr, index);
      return wide + 3;
    case JAVA_OP_CONSTANT8:
      // Note: These instructions should never be wide.
      index = memory->read8(address + wide + 1);
      snprintf(instruction, length, "%s %d", table_java[opcode].instr, index);
      return wide + 2;
    case JAVA_OP_OFFSET16:
      // Note: These instructions should never be wide.
      offset16 = memory->read16(address + wide + 1);
      snprintf(instruction, length, "%s 0x%04x (offset=%d)", table_java[opcode].instr, address + offset16, offset16);
      return wide + 3;
    case JAVA_OP_OFFSET32:
      // Note: These instructions should never be wide.
      offset = memory->read32(address + wide + 1);
      snprintf(instruction, length, "%s 0x%04x (offset=%d)", table_java[opcode].instr, address + offset, offset);
      return wide + 5;
    case JAVA_OP_WARN:
      snprintf(instruction, length, "[%s]", table_java[opcode].instr);
      return wide + 1;
    default:
      snprintf(instruction, length, "<error>");
      return wide + 1;
  }

  return wide + 1;
}

void list_output_java(
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

  count = disasm_java(
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

void disasm_range_java(
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
    count = disasm_java(
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

