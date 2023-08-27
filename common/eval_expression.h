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

#ifndef NAKEN_ASM_EVAL_EXPRESSION_H
#define NAKEN_ASM_EVAL_EXPRESSION_H

#include "common/assembler.h"
#include "common/eval_expression.h"
#include "common/Var.h"

class EvalExpression
{
public:
  struct Operator
  {
    Operator() :
      operation  (OPER_UNSET),
      precedence (PREC_UNSET)
    {
    }

    int operation;
    int precedence;
  };

  static int eval_expression_go(
    AsmContext *asm_context,
    Var &var,
    Operator *last_operator);

private:
  EvalExpression()  { }
  ~EvalExpression() { }

  enum
  {
    PREC_NOT,
    PREC_MUL,
    PREC_ADD,
    PREC_SHIFT,
    PREC_AND,
    PREC_XOR,
    PREC_OR,
    PREC_UNSET
  };

  enum
  {
    OPER_UNSET,
    OPER_NOT,
    OPER_MUL,
    OPER_DIV,
    OPER_MOD,
    OPER_PLUS,
    OPER_MINUS,
    OPER_LEFT_SHIFT,
    OPER_RIGHT_SHIFT,
    OPER_AND,
    OPER_XOR,
    OPER_OR
  };

  static int get_operator(char *token, Operator *oper);
  static int operate(Var &var_d, Var &var_s, Operator *oper);
  static int parse_unary(AsmContext *asm_context, int64_t *num, int operation);

};

int eval_expression_ex(AsmContext *asm_context, Var &var);
int eval_expression(AsmContext *asm_context, int *num);

#endif

