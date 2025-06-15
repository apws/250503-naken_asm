/**
 *  naken_asm assembler.
 *  Author: Michael Kohn
 *   Email: mike@mikekohn.net
 *     Web: https://www.mikekohn.net/
 * License: GPLv3
 *
 * Copyright 2010-2024 by Michael Kohn
 *
 */

#include "table/rv32em.h"

struct _table_rv32em table_rv32em[] =
{
  // Alias instructions.
  { "nop",        0x00000013, 0xffffffff, OP_NONE,       0 },
  // mv rd, rs           : addi rd, rs, 0
  // not rd, rs          : xori rd, rs, -1
  // neg rd, rs          : sub rd, x0, rs
  // negw rd, rs         : subw rd, x0, rs
  // sext.w rd, rs       : addiw rd, rs, 0
  // seqz rd, rs         : sltiu rd, rs, 1
  // snez rd, rs         : sltu rd, x0, rs
  // sltz rd, rs         : slt rd, rs, x0
  // sgtz rd, rs         : slt rd, x0, rs
  { "mv",         0x00000013, 0xfff0707f, OP_ALIAS_RD_RS1,       0 },
  { "not",        0xfff04013, 0xfff0707f, OP_ALIAS_RD_RS1,       0 },
  { "neg",        0x40000033, 0xfe0ff07f, OP_ALIAS_RD_RS2,       0 },
  { "seqz",       0x00103013, 0x0000707f, OP_ALIAS_RD_RS1,       0 },
  { "snez",       0x00003033, 0xfe0ff07f, OP_ALIAS_RD_RS2,       0 },
  { "sltz",       0x00002033, 0xfff0707f, OP_ALIAS_RD_RS1,       0 },
  { "sgtz",       0x00002033, 0xfe0ff07f, OP_ALIAS_RD_RS2,       0 },
  // beqz rs, offset     : beq rs, x0, offset
  // bnez rs, offset     : bne rs, x0, offset
  // blez rs, offset     : bge x0, rs, offset
  // bgez rs, offset     : bge rs, x0, offset
  // bltz rs, offset     : blt rs, x0, offset
  // bgtz rs, offset     : blt x0, rs, offset
  // bgt rs, rt, offset  : blt rt, rs, offset
  // ble rs, rt, offset  : bge rt, rs, offset
  // bgtu rs, rt, offset : bltu rt, rs, offset
  // bleu rs, rt, offset : bgeu rt, rs, offset
  { "beqz",       0x00000063, 0x01f0707f, OP_ALIAS_BR_RS_X0,    0 },
  { "bnez",       0x00001063, 0x01f0707f, OP_ALIAS_BR_RS_X0,    0 },
  { "blez",       0x00005063, 0x01f0707f, OP_ALIAS_BR_X0_RS,    0 },
  { "bgez",       0x00005063, 0x000ff07f, OP_ALIAS_BR_RS_X0,    0 },
  { "bltz",       0x00004063, 0x000ff07f, OP_ALIAS_BR_RS_X0,    0 },
  { "bgtz",       0x00004063, 0x01f0707f, OP_ALIAS_BR_X0_RS,    0 },
  { "bgt",        0x00004063, 0x0000707f, OP_ALIAS_BR_RS_RT,    0 },
  { "ble",        0x00005063, 0x0000707f, OP_ALIAS_BR_RS_RT,    0 },
  { "bgtu",       0x00006063, 0x0000707f, OP_ALIAS_BR_RS_RT,    0 },
  { "bleu",       0x00007063, 0x0000707f, OP_ALIAS_BR_RS_RT,    0 },
  // j offset            : jal x0, offset
  // jal offset          : jal x1, offset
  // jr rs               : jalr x0, rs, 0
  // jalr rs             : jalr x1, rs, 0
  // ret                 : jalr x0, x1, 0
  { "ret",        0x00008067, 0xffffffff, OP_NONE,              0 },
  { "j",          0x0000006f, 0x00000fff, OP_ALIAS_JAL,         0 },
  { "jal",        0x000000ef, 0x00000fff, OP_ALIAS_JAL,         0 },
  { "jr",         0x00000067, 0xfff07fff, OP_ALIAS_JALR,        0 },
  { "jalr",       0x000000e7, 0xfff07fff, OP_ALIAS_JALR,        0 },
  // call offset         : auipc x6, offset[31:12]
  //                       jalr x1, x6, offset[11:0]
  // tail offset         : auipc x6, offset[31:12]
  //                       jalr x0, x6, offset[11:0]
  //{ "call",       0x00000017, 0x0000007f, OP_U_TYPE,            0 },
  //{ "tail",       0x00000017, 0x0000007f, OP_U_TYPE,            0 },
  // fence               : fence iorw, iorw
  { "fence",      0x0000000f, 0xffffffff, OP_NONE,              0 },
  // Regular instructions.
  { "lui",        0x00000037, 0x0000007f, OP_U_TYPE,     0 },
  { "auipc",      0x00000017, 0x0000007f, OP_U_TYPE,     0 },
  { "jal",        0x0000006f, 0x0000007f, OP_UJ_TYPE,    0 },
  { "jalr",       0x00000067, 0x0000707f, OP_I_TYPE,     0 },
  { "beq",        0x00000063, 0x0000707f, OP_SB_TYPE,    0 },
  { "bne",        0x00001063, 0x0000707f, OP_SB_TYPE,    0 },
  { "blt",        0x00004063, 0x0000707f, OP_SB_TYPE,    0 },
  { "bge",        0x00005063, 0x0000707f, OP_SB_TYPE,    0 },
  { "bltu",       0x00006063, 0x0000707f, OP_SB_TYPE,    0 },
  { "bgeu",       0x00007063, 0x0000707f, OP_SB_TYPE,    0 },
  { "lb",         0x00000003, 0x0000707f, OP_RD_INDEX_R, 0 },
  { "lh",         0x00001003, 0x0000707f, OP_RD_INDEX_R, 0 },
  { "lw",         0x00002003, 0x0000707f, OP_RD_INDEX_R, 0 },
  { "lbu",        0x00004003, 0x0000707f, OP_RD_INDEX_R, 0 },
  { "lhu",        0x00005003, 0x0000707f, OP_RD_INDEX_R, 0 },
  { "sb",         0x00000023, 0x0000707f, OP_RS_INDEX_R, 0 },
  { "sh",         0x00001023, 0x0000707f, OP_RS_INDEX_R, 0 },
  { "sw",         0x00002023, 0x0000707f, OP_RS_INDEX_R, 0 },
  { "addi",       0x00000013, 0x0000707f, OP_I_TYPE,     0 },
  { "slti",       0x00002013, 0x0000707f, OP_I_TYPE,     0 },
  { "sltiu",      0x00003013, 0x0000707f, OP_I_TYPE,     0 },
  { "xori",       0x00004013, 0x0000707f, OP_I_TYPE,     0 },
  { "ori",        0x00006013, 0x0000707f, OP_I_TYPE,     0 },
  { "andi",       0x00007013, 0x0000707f, OP_I_TYPE,     0 },
  { "slli",       0x00001013, 0xfe00707f, OP_SHIFT,      0 },
  { "srli",       0x00005013, 0xfe00707f, OP_SHIFT,      0 },
  { "srai",       0x40005013, 0xfe00707f, OP_SHIFT,      0 },
  { "add",        0x00000033, 0xfe00707f, OP_R_TYPE,     0 },
  { "sub",        0x40000033, 0xfe00707f, OP_R_TYPE,     0 },
  { "sll",        0x00001033, 0xfe00707f, OP_R_TYPE,     0 },
  { "slt",        0x00002033, 0xfe00707f, OP_R_TYPE,     0 },
  { "sltu",       0x00003033, 0xfe00707f, OP_R_TYPE,     0 },
  { "xor",        0x00004033, 0xfe00707f, OP_R_TYPE,     0 },
  { "srl",        0x00005033, 0xfe00707f, OP_R_TYPE,     0 },
  { "sra",        0x40005033, 0xfe00707f, OP_R_TYPE,     0 },
  { "or",         0x00006033, 0xfe00707f, OP_R_TYPE,     0 },
  { "and",        0x00007033, 0xfe00707f, OP_R_TYPE,     0 },
  { "fence",      0x0000000f, 0xf00fffff, OP_FENCE,      0 },
  { "fence.i",    0x0000100f, 0xffffffff, OP_FFFF,       0 },
  { "ecall",      0x00000073, 0xffffffff, OP_FFFF,       0 },
  { "ebreak",     0x00100073, 0xffffffff, OP_FFFF,       0 },
  { "mul",        0x02000033, 0xfe00707f, OP_R_TYPE,     0 },
  { "mulh",       0x02001033, 0xfe00707f, OP_R_TYPE,     0 },
  { "mulhsu",     0x02002033, 0xfe00707f, OP_R_TYPE,     0 },
  { "mulhu",      0x02003033, 0xfe00707f, OP_R_TYPE,     0 },
  { "div",        0x02004033, 0xfe00707f, OP_R_TYPE,     0 },
  { "divu",       0x02005033, 0xfe00707f, OP_R_TYPE,     0 },
  { "rem",        0x02006033, 0xfe00707f, OP_R_TYPE,     0 },
  { "remu",       0x02007033, 0xfe00707f, OP_R_TYPE,     0 },
  { "lr.w",       0x1000202f, 0xf800707f, OP_LR,         0 },
  { "sc.w",       0x1800202f, 0xf800707f, OP_STD_EXT,    0 },
  { "lr.d",       0x1000302f, 0xf800707f, OP_LR,         0 },
  { "sc.d",       0x1800302f, 0xf800707f, OP_STD_EXT,    0 },
  // Privileged instructions?
  { "uret",       0x00200073, 0xffffffff, OP_NONE,        0 },
  { "sret",       0x10200073, 0xffffffff, OP_NONE,        0 },
  { "hret",       0x20200073, 0xffffffff, OP_NONE,        0 },
  { "mret",       0x30200073, 0xffffffff, OP_NONE,        0 },
  { "wfi",        0x10500073, 0xffffffff, OP_NONE,        0 },
  { "sfence.vm",  0x10400073, 0xfff07fff, OP_RS1,         0 },
  { NULL,                  0,          0, 0,              0 }
};

