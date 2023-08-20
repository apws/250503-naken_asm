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

#include "disasm/tms340.h"
#include "table/tms340.h"

static void get_register(char *s, int length, int n, char r)
{
  if (n == 15)
  {
    strcpy(s, "sp");
  }
    else
  {
    snprintf(s, length, "%c%d", r, n);
  }
}

int disasm_tms340(
  Memory *memory,
  uint32_t address,
  char *instruction,
  int length,
  int flags,
  int *cycles_min,
  int *cycles_max)
{
  char operand[32];
  char reg[8];
  uint32_t start = address;
  uint32_t ilw;
  int16_t displacement;
  int32_t displacement32;
  int16_t mask;
  uint32_t temp;
  int opcode;
  int n, i, j, x;
  char r;
  int m;
  int rs, rd;

  *cycles_min = -1;
  *cycles_max = -1;

  opcode = memory->read16(address);

  address += 2;

  r = (opcode & 0x0010) == 0 ? 'a' : 'b';
  rs = (opcode >> 5) & 0xf;
  rd = opcode & 0xf;

  for (n = 0; table_tms340[n].instr != NULL; n++)
  {
    if ((opcode & table_tms340[n].mask) == table_tms340[n].opcode)
    {
      snprintf(instruction, length, "%s", table_tms340[n].instr);

      for (i = 0; i < table_tms340[n].operand_count; i++)
      {
        if (i == 0)
        {
          strcat(instruction, " ");
        }
          else
        {
          strcat(instruction, ", ");
        }

        switch (table_tms340[n].operand_types[i])
        {
          case OP_RS:
            get_register(reg, sizeof(reg), rs, r);
            strcat(instruction, reg);
            break;
          case OP_RD:
          case OP_RDS:
            get_register(reg, sizeof(reg), rd, r);
            strcat(instruction, reg);
            break;
          case OP_RD_R_FILE:
            m = (opcode >> 9) & 1;
            get_register(reg, sizeof(reg), rd, r);
            if (m == 1)
            {
              reg[0] = reg[0] == 'a' ? 'b' : 'a';
            }
            strcat(instruction, reg);
            break;
          case OP_P_RS:
            get_register(reg, sizeof(reg), rs, r);
            strcat(instruction, "*");
            strcat(instruction, reg);
            break;
          case OP_P_RD:
            get_register(reg, sizeof(reg), rd, r);
            strcat(instruction, "*");
            strcat(instruction, reg);
            break;
          case OP_P_RS_DISP:
            get_register(reg, sizeof(reg), rs, r);
            displacement = memory->read16(address);
            snprintf(operand, sizeof(operand), "*%s(%d)", reg, displacement);
            strcat(instruction, operand);
            address += 2;
            break;
          case OP_P_RD_DISP:
            get_register(reg, sizeof(reg), rd, r);
            displacement = memory->read16(address);
            snprintf(operand, sizeof(operand), "*%s(%d)", reg, displacement);
            strcat(instruction, operand);
            address += 2;
            break;
          case OP_P_RS_P:
            get_register(reg, sizeof(reg), rs, r);
            snprintf(operand, sizeof(operand), "*%s+", reg);
            strcat(instruction, operand);
            break;
          case OP_P_RD_P:
            get_register(reg, sizeof(reg), rd, r);
            snprintf(operand, sizeof(operand), "*%s+", reg);
            strcat(instruction, operand);
            break;
          case OP_P_RS_XY:
            get_register(reg, sizeof(reg), rs, r);
            snprintf(operand, sizeof(operand), "*%s.XY", reg);
            strcat(instruction, operand);
            break;
          case OP_P_RD_XY:
            get_register(reg, sizeof(reg), rd, r);
            snprintf(operand, sizeof(operand), "*%s.XY", reg);
            strcat(instruction, operand);
            break;
          case OP_MP_RS:
            get_register(reg, sizeof(reg), rs, r);
            snprintf(operand, sizeof(operand), "-*%s", reg);
            strcat(instruction, operand);
            break;
          case OP_MP_RD:
            get_register(reg, sizeof(reg), rd, r);
            snprintf(operand, sizeof(operand), "-*%s", reg);
            strcat(instruction, operand);
            break;
          case OP_ADDRESS:
            temp = memory->read16(address);
            temp |= memory->read16(address + 2) << 16;
            snprintf(operand, sizeof(operand), "0x%04x", temp);
            strcat(instruction, operand);
            address += 4;
            break;
          case OP_AT_ADDR:
            ilw = memory->read16(address);
            ilw |= memory->read16(address + 2) << 16;
            snprintf(operand, sizeof(operand), "@0x%08x", ilw);
            strcat(instruction, operand);
            address += 4;
            break;
          case OP_LIST:
            mask = memory->read16(address);
            x = 0;

            if (mask == 0)
            {
              strcat(instruction, "0");
              break;
            }

            if ((table_tms340[n].opcode & 0x0020) != 0)
            {
              for (j = 0; j < 16; j++)
              {
                if ((mask & (1 << j)) != 0)
                {
                  if (x != 0) { strcat(instruction, ", "); }
                  get_register(reg, sizeof(reg), j, r);
                  strcat(instruction, reg);
                  x++;
                }
              }
            }
              else
            {
              for (j = 0; j < 16; j++)
              {
                if ((mask & (1 << (15 - j))) != 0)
                {
                  if (x != 0) { strcat(instruction, ", "); }
                  get_register(reg, sizeof(reg), j, r);
                  strcat(instruction, reg);
                  x++;
                }
              }
            }

            address += 2;

            break;
          case OP_B:
            strcat(instruction, "B");
            break;
          case OP_F:
            strcat(instruction, (opcode & 0x0200) == 0 ? "0" : "1");
            break;
          case OP_K32:
            temp = (opcode >> 5) & 0x1f;
            if (temp == 0) { temp = 32; }
            snprintf(operand, sizeof(operand), "%d", temp);
            strcat(instruction, operand);
            break;
          case OP_K:
            temp = (opcode >> 5) & 0x1f;
            snprintf(operand, sizeof(operand), "%d", temp);
            strcat(instruction, operand);
            break;
          case OP_1K:
            temp = ~((opcode >> 5) & 0x1f);
            snprintf(operand, sizeof(operand), "%d", temp & 0x1f);
            strcat(instruction, operand);
            break;
          case OP_2K:
            temp = -(((opcode >> 5) & 0x1f) | 0xffffffe0);
            snprintf(operand, sizeof(operand), "%d", temp & 0x1f);
            strcat(instruction, operand);
            break;
          case OP_L:
            strcat(instruction, "L");
            break;
          case OP_N:
            temp = opcode & 0x1f;
            snprintf(operand, sizeof(operand), "%d", temp);
            strcat(instruction, operand);
            break;
          case OP_Z:
            strcat(instruction, (opcode & 0x0080) == 0 ? "0" : "1");
            break;
          case OP_FE:
            strcat(instruction, (opcode & 0x0020) == 0 ? "0" : "1");
            break;
          case OP_FS:
            temp = opcode & 0x1f;
            snprintf(operand, sizeof(operand), "%d", temp);
            strcat(instruction, operand);
            break;
          case OP_IL:
            temp = memory->read16(address);
            temp |= memory->read16(address + 2) << 16;
            snprintf(operand, sizeof(operand), "0x%04x", temp);
            strcat(instruction, operand);
            address += 4;
            break;
          case OP_IW:
            temp = memory->read16(address);
            snprintf(operand, sizeof(operand), "%d", (int16_t)temp);
            strcat(instruction, operand);
            address += 2;
            break;
          case OP_NIL:
            temp = memory->read16(address);
            temp |= memory->read16(address + 2) << 16;
            snprintf(operand, sizeof(operand), "0x%04x", ~temp);
            strcat(instruction, operand);
            address += 4;
            break;
          case OP_NIW:
            temp = memory->read16(address);
            snprintf(operand, sizeof(operand), "%d", (int16_t)~temp);
            strcat(instruction, operand);
            address += 2;
            break;
          case OP_NN:
            temp = opcode & 0x1f;
            snprintf(operand, sizeof(operand), "%d", temp);
            strcat(instruction, operand);
            break;
          case OP_XY:
            strcat(instruction, "XY");
            break;
          case OP_DISP:
            displacement = memory->read16(address);
            snprintf(operand, sizeof(operand), "0x%04x (%d)",
              address + 2 + displacement, displacement);
            strcat(instruction, operand);
            address += 2;
            break;
          case OP_SKIP:
            displacement = (opcode >> 5) & 0x1f;
            displacement *= 2;
            if ((opcode & 0x0400) != 0) { displacement = -displacement; }
            snprintf(operand, sizeof(operand), "0x%04x (%d)",
              address + displacement, displacement);
            strcat(instruction, operand);
            break;
          case OP_JUMP_REL:
            if ((opcode & 0x00ff) != 0)
            {
              displacement32 = (int8_t)(opcode & 0xff);
              displacement32 *= 2;
              snprintf(operand, sizeof(operand), "0x%04x (%d)",
                address + displacement32, displacement32);
              strcat(instruction, operand);
            }
              else
            {
              displacement32 = (int8_t)memory->read16(address);
              displacement32 *= 2;
              snprintf(operand, sizeof(operand), "0x%04x (%d)",
                address + 2 + displacement32, displacement32);
              strcat(instruction, operand);
              address += 2;
            }

            break;
          default:
            break;
        }
      }

      return address - start;
    }
  }

  snprintf(instruction, length, "???");

  return address - start;
}

void list_output_tms340(AsmContext *asm_context, uint32_t start, uint32_t end)
{
  int cycles_min,cycles_max;
  char instruction[128];
  uint16_t opcode;
  int count;
  int n;

  Memory *memory = &asm_context->memory;

  while (start < end)
  {
    opcode = memory->read16(start);

    count = disasm_tms340(
      memory,
      start,
      instruction,
      sizeof(instruction),
      asm_context->flags,
      &cycles_min,
      &cycles_max);

    fprintf(asm_context->list, "0x%04x: %04x %-40s\n", start, opcode, instruction);

    for (n = 2; n < count; n += 2)
    {
      opcode = memory->read16(start + n);

      fprintf(asm_context->list, "0x%04x: %04x\n", start + n, opcode);
    }

    start += count;
  }

  fprintf(asm_context->list, "\n");
}

void disasm_range_tms340(
  Memory *memory,
  uint32_t flags,
  uint32_t start,
  uint32_t end)
{
  char instruction[128];
  uint16_t opcode;
  int cycles_min = 0, cycles_max = 0;
  int count;
  int n;

  printf("\n");

  printf("%-7s %-5s %-40s Cycles\n", "Addr", "Opcode", "Instruction");
  printf("------- ------ ----------------------------------       ------\n");

  while (start <= end)
  {
    count = disasm_tms340(
      memory,
      start,
      instruction,
      sizeof(instruction),
      0,
      &cycles_min,
      &cycles_max);

    opcode = memory->read16(start);

    printf("0x%04x: %04x  %-40s\n", start, opcode, instruction);

    for (n = 2; n < count; n += 2)
    {
      opcode = memory->read16(start + n);

      printf("0x%04x: %04x\n", start + n, opcode);
    }

    start = start + count;
  }
}

