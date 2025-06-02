#ifndef BLUR_CPS_H
#define BLUR_CPS_H

#include "../ast/ast.h"

typedef enum {
  CPS_LITERAL,
  CPS_LET,
  CPS_IF,
  CPS_CALL,
  CPS_RETURN,
} CpsKind;

typedef struct CpsNode CpsNode;

typedef struct {
  const char *name;
  int index;
} CpsVar;

typedef enum {
  CPS_LITERAL_INTEGER,
  CPS_LITERAL_VAR,
} CpsLiteralKind;

typedef struct {
  CpsLiteralKind kind;
  union {
    CpsVar var;
    int integer;
  };
} CpsLiteral;


typedef struct {
  CpsVar var;
  CpsLiteral value;
  int cont;
} CpsLet;

typedef struct {
  CpsVar cond;
  int then_label;
  int else_label;
} CpsIf;

typedef struct {
  const char *value;
} CpsReturn;

typedef struct {
  char *func;
  CpsLiteral* args;
  int arg_count;
  // int result_index;
  int cont;
} CpsCall;

struct CpsNode {
  int label;
  CpsKind kind;
  union {
    CpsLet let_node;
    CpsCall call_node;
    CpsIf if_node;
    Literal literal;
    CpsReturn return_node;
  };

  CpsNode *next;
};

#endif
