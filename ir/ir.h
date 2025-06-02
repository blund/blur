#ifndef BLUR_CPS_H
#define BLUR_CPS_H

#include <ast/ast.h>

typedef enum {
  IR_LITERAL,
  IR_LET,
  IR_IF,
  IR_CALL,
  IR_RETURN,
} IrKind;

typedef struct IrNode IrNode;

typedef struct {
  const char *name;
  int index;
} IrVar;

typedef enum {
  IR_LITERAL_INTEGER,
  IR_LITERAL_VAR,
} IrLiteralKind;

typedef struct {
  IrLiteralKind kind;
  union {
    IrVar var;
    int integer;
  };
} IrLiteral;


typedef struct {
  IrVar var;
  IrLiteral value;
  int cont;
} IrLet;

typedef struct {
  IrVar cond;
  int then_label;
  int else_label;
} IrIf;

typedef struct {
  const char *value;
} IrReturn;

typedef struct {
  char *func;
  IrLiteral* args;
  int arg_count;
  // int result_index;
  int cont;
} IrCall;

struct IrNode {
  int label;
  IrKind kind;
  union {
    IrLet let_node;
    IrCall call_node;
    IrIf if_node;
    IrLiteral literal;
    IrReturn return_node;
  };

  IrNode *next;
};

#endif
