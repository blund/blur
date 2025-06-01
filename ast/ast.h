#ifndef BLUR_AST_H
#define BLUR_AST_H

typedef struct statement Statement;

typedef struct {
  char *key;
  int value;  // can be anything; `true` is good enough
} UsedVarSet;

typedef enum {
  block_node,
  literal_node,
  argument_node,
  call_node,
  expression_node,
  if_node,
  assign_node,
  statement_node,
} NodeType;

typedef struct Block {
  NodeType node_type;
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
  NodeType node_type;
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
  NodeType node_type;
  int count;
  struct expression *entries[8];
} Arguments;

typedef struct Call {
  NodeType node_type;
  char *name;
  Arguments args;
} Call;

typedef struct expression {
  NodeType node_type;
  ExpressionKind kind;
  union {
    Literal lit;
    struct Call call;
  };
} Expression;

typedef struct {
  NodeType node_type;
  Expression *condition;
  Block *then_branch;
  Block *else_branch;
} If;

typedef struct {
  NodeType node_type;
  char *name;
  Expression *expr;
} Assign;

typedef enum StatementKind {
  assign_statement,
  call_statement,
  if_statement,
} StatementKind;

typedef struct statement {
  NodeType node_type;
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
Expression *new_call_expr(char *name, Arguments args);
Expression *new_identifier(char *name);
Expression *new_integer(int n);
void print_block(Block *b);

#endif
