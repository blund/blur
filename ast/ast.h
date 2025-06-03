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
  args_node,
  params_node,
  call_node,
  expression_node,
  if_node,
  let_node,
  statement_node,
  func_decl_node,
  type_node,
  var_node,
} NodeType;

typedef struct {
  NodeType node_type;
  char* name;
} Type;

typedef struct {
  NodeType node_type;
  char *name;
  Type type;
} Var;

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
  struct expression **entries;
} Arguments;

typedef struct Call {
  NodeType node_type;
  char *name;
  Arguments *args;
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
  Type type;
  char *name;
  Expression *expr;
} Let;

typedef enum StatementKind {
  let_statement,
  call_statement,
  if_statement,
} StatementKind;

typedef struct statement {
  NodeType node_type;
  StatementKind kind;
  union {
    Let let;
    Call call;
    If if_block;
  };
} Statement;

typedef struct {
  NodeType node_type;
  int count;
  Var *entries;
} Parameters;

typedef struct FuncDecl {
  NodeType node_type;
  Type ret;
  char *name;
  Parameters *params;
  Block *body;
} FuncDecl;

#endif
