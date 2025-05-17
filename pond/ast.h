#ifndef AST_H
#define AST_H

#define THIS p->code[p->index]
#define DBG printf("DBG:\n%s\n\n", &THIS)
#define OK = p->ok

typedef enum ValueType {
  string_type,
  integer_type,
} ValueType;

typedef struct Parser {
  char* code;
  int   len;
  int   index;
  int   ok;

  int indent;
} Parser;

typedef struct statement Statement;

typedef struct Unit {
  int start; // Index of the first char of the Unit
  int end;   // Index of the char following the Unit
} Unit;

typedef struct Value {
  Unit       data;
  ValueType type;
} Value;

typedef struct Type {
  Unit name;
  int  ptr; // 1 or 0
} Type;

typedef struct Call {
  Unit name;
} Call;

typedef enum StatementKind {
  statement_assign_kind,
  statement_call_kind,
  statement_if_kind,
} StatementKind;

typedef struct Block {
  Statement* statement;
  struct Block* next; // Linked list, assume 0 == end
} Block;

typedef struct IfBlock {
  Unit condition;
  Block* body;
} IfBlock;

typedef struct ArgList {
  int  arg_count;
  Unit names[8];
  Type types[8];
} ArgList;

typedef struct FuncDecl {
  Unit    name;
  Type    ret;
  ArgList arg_list;
  Block*  body;
} FuncDecl;

typedef enum ExprKind {
  expr_call_kind,
  expr_value_kind,
} ExprKind;

typedef struct Expr {
  ExprKind kind;
  union {
    Call  call;
    Value value;
  };
} Expr;

typedef struct Assign {
  Type  type;
  Unit  name;
  Expr* expr;
} Assign;

typedef struct statement {
  StatementKind kind;
  union {
    Assign   assign;
    Call     call;
    IfBlock if_block;
  };
} Statement;

#endif
