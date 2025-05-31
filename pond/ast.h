#ifndef AST_H
#define AST_H

#define THIS p->code[p->index]
#define NEXT p->code[p->index+1]
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
  char *ptr; // Index of the first char of the Unit
  int start;
  int end; // Index of the char following the Unit
} Unit;

typedef struct Value {
  Unit      data;
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
  statement_pointer_call_kind,
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

typedef struct Parameters {
  int  arg_count;
  Unit names[8];
  Type types[8];
} Parameters;

typedef struct FuncDecl {
  Unit    name;
  Type    ret;
  Parameters parameters;
  Block*  body;
} FuncDecl;

typedef struct PointerCall {
  // Type
  Type return_type;

  // Call
  Unit operand; // @TODO - this could be more general, we assume this to be a hex for now

  Parameters parameters;
} PointerCall;

typedef enum ExprKind {
  expr_call_kind,
  expr_value_kind,
} ExprKind;

typedef struct Expr {
  ExprKind kind;
  union {
    // @TODO - unify PoiterCall and Call
    PointerCall  pointer_call;
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
    Call call;
    PointerCall pointer_call;
    IfBlock if_block;
  };
} Statement;

#endif
