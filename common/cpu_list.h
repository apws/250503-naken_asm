/**
 *  naken_asm assembler.
 *  Author: Michael Kohn
 *   Email: mike@mikekohn.net
 *     Web: https://www.mikekohn.net/
 * License: GPLv3
 *
 * Copyright 2010-2025 by Michael Kohn
 *
 */

#ifndef NAKEN_ASM_CPU_LIST_H
#define NAKEN_ASM_CPU_LIST_H

#include "common/Linker.h"
#include "simulate/Simulate.h"

typedef Simulate *(*simulate_init_t)(Memory *);

typedef int (*parse_instruction_t)(AsmContext *, char *);
typedef int (*parse_directive_t)(AsmContext *, const char *);

typedef int (*link_function_t)(
  AsmContext *,
  Imports *,
  const uint8_t *,
  uint32_t function_offset,
  int size,
  uint8_t *obj_file,
  uint32_t obj_size);

typedef void (*list_output_t)(AsmContext *, uint32_t, uint32_t);
typedef void (*disasm_range_t)(Memory *, uint32_t, uint32_t, uint32_t);

enum
{
  CPU_TYPE_MSP430 = 0,
  CPU_TYPE_MSP430X,
  CPU_TYPE_1802,
  CPU_TYPE_4004,
  CPU_TYPE_6502,
  CPU_TYPE_65816,
  CPU_TYPE_6800,
  CPU_TYPE_6809,
  CPU_TYPE_68HC08,
  CPU_TYPE_68000,
  CPU_TYPE_8008,
  CPU_TYPE_8041,
  CPU_TYPE_8048,
  CPU_TYPE_8051,
  CPU_TYPE_86000,
  CPU_TYPE_AGC,
  CPU_TYPE_ARC,
  CPU_TYPE_ARM,
  CPU_TYPE_ARM64,
  CPU_TYPE_AVR8,
  CPU_TYPE_CELL,
  CPU_TYPE_COPPER,
  CPU_TYPE_CP1610,
  CPU_TYPE_DOTNET,
  CPU_TYPE_DSPIC,
  CPU_TYPE_EBPF,
  CPU_TYPE_EMOTION_ENGINE,
  CPU_TYPE_EPIPHANY,
  CPU_TYPE_F100_L,
  CPU_TYPE_F8,
  CPU_TYPE_JAVA,
  CPU_TYPE_LC3,
  CPU_TYPE_M8C,
  CPU_TYPE_MIPS32,
  CPU_TYPE_PDP8,
  CPU_TYPE_PDP11,
  CPU_TYPE_PDK13,
  CPU_TYPE_PDK14,
  CPU_TYPE_PDK15,
  CPU_TYPE_PDK16,
  CPU_TYPE_PIC14,
  CPU_TYPE_PIC18,
  CPU_TYPE_PIC24,
  CPU_TYPE_POWERPC,
  CPU_TYPE_PROPELLER,
  CPU_TYPE_PROPELLER2,
  CPU_TYPE_PS2_EE_VU,
  CPU_TYPE_RV32EM,
  CPU_TYPE_RISCV,
  CPU_TYPE_SH4,
  CPU_TYPE_SPARC,
  CPU_TYPE_STM8,
  CPU_TYPE_SUPER_FX,
  CPU_TYPE_SWEET16,
  CPU_TYPE_THUMB,
  CPU_TYPE_TMS340,
  CPU_TYPE_TMS1000,
  CPU_TYPE_TMS1100,
  CPU_TYPE_TMS9900,
  CPU_TYPE_UNSP,
  CPU_TYPE_WEBASM,
  CPU_TYPE_XTENSA,
  CPU_TYPE_Z80,
  CPU_TYPE_IGNORE
};

#define ALIGN_1 1
#define ALIGN_2 2
#define ALIGN_4 4
#define ALIGN_8 8
#define ALIGN_16 16

#define SREC_16 0
#define SREC_24 1
#define SREC_32 2

// The options in this structure:
// name: If this is set to "mycpu", the assembler will use a .mycpu directive
// default_endian: ENDIAN_BIG or ENDIAN_LITTLE
// bytes_per_address: Some odd CPU's (AVR8 and PIC24) aren't 1 byte per address
// alignment: How many bytes to align on.  MIPS for example is 4 byte per instr
// is_dollar_hex: Some old CPU's have assemblers that represent hex as $100 ..
//                even if this is set, naken_asm will still allow 0x100
// can_tick_end_string: Some wierd z80 syntax
// srec_size: size of data in an srec file
// parse_instruction: function name to assemble the next instruction.
// parse_directive: extra function for special directives
// link_function: function used for doing ELF linking with .o's.
// list_output: function that write to the -l option listing file
// disasm_range: function that disassembles code and writes to stdout
// simulate_init: function that inializes the simulator.
// flags: extra flags the assembler can use.

typedef struct _cpu_list
{
  const char *name;
  uint8_t type;
  uint8_t default_endian;
  uint8_t bytes_per_address;
  uint8_t alignment;
  uint8_t is_dollar_hex          : 1;
  uint8_t can_tick_end_string    : 1;
  uint8_t pass_1_write_disable   : 1;
  uint8_t strings_have_dots      : 1;
  uint8_t strings_have_slashes   : 1;
  uint8_t ignore_number_postfix  : 1;
  uint8_t numbers_dont_have_dots : 1;
  uint8_t srec_size              : 2;
  parse_instruction_t parse_instruction;
  parse_directive_t parse_directive;
  link_function_t link_function;
  list_output_t list_output;
  disasm_range_t disasm_range;
  simulate_init_t simulate_init;
  uint32_t flags;
} CpuList;

extern CpuList cpu_list[];

#endif

