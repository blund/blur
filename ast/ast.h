#ifndef BLUR_AST_H
#define BLUR_AST_H

typedef struct statement Statement;

typedef struct {
  char *key;
  int value;  // can be anything; `true` is good enough
} UsedVarSet;


typedef struct Block {
  int count;
  Statement **statements;

  UsedVarSet* used_vars;
} Block;

typedef enum {
  integer_lit,
  floating_lit,
  string_lit,
  identifier_lit,
} LiteralKind;

typedef struct {
  LiteralKind kind;
  union {
    int   integer;
    float floating;
    char *string;
    char *identifier;
  };
} Literal;

typedef enum {
  call_expr,
  lit_expr,
} ExpressionKind;

typedef struct expression Expression;

typedef struct {
  int count;
  struct expression *entries[8];
} Arguments;

typedef struct Call {
  char *name;
  Arguments args;
} Call;

typedef struct expression {
  ExpressionKind kind;
  union {
    Literal lit;
    struct Call call;
  };
} Expression;

typedef struct {
  Expression *condition;
  Block *then_branch;
  Block *else_branch;
} If;

typedef struct {
  char *name;
  Expression *expr;
} Assign;

typedef enum StatementKind {
  assign_statement,
  call_statement,
  if_statement,
} StatementKind;

typedef struct statement {
  StatementKind kind;
  union {
    Assign assign;
    Call call;
    If if_block;
  };
} Statement;


Block *new_block(Statement *s);
Block *next_block(Block *prev, Statement *s);
Statement *new_if(Expression *condition, Block *s1, Block *s2);
Statement *new_assign(char *name, Expression *e);
Statement *new_call(char *name, Arguments args);
Expression *new_identifier(char *name);
Expression *new_integer(int n);
void print_block(Block *b);

#endif
