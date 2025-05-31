#include "stdlib.h"
#include "string.h"

#include "ast.h"
#include "build.h"

Unit make_unit(char *str) {
  Unit u;
  u.ptr = str;
  u.start = 0;
  u.end = strlen(str);

  return u;
}

Type make_type(char *str, int ptr) {
  Type t;
  Unit u;
  u.ptr = str;
  u.start = 0;
  u.end = strlen(str);

  t.name = u;
  t.ptr = ptr;
  return t;
}

Parameter new_parameter(char *type, char *name) {
  Parameter p;
  p.name = make_unit(name);
  p.type = make_type(type, 0);
  return p;
}

FuncDecl *new_func_decl(char *ret_type, char *name, Parameters params) {
  FuncDecl *fd = malloc(sizeof(FuncDecl));
  fd->name = make_unit(name);
  fd->ret = make_type(ret_type, 0);
  fd->parameters = params;
  fd->body = new_block();

  return fd;
}


Statement *new_statement() {
  Statement *b = malloc(sizeof(Statement));
  return b;
}


Block *new_block() {
  Block *b = malloc(sizeof(Block));
  b->statement = new_statement();
  b->next = 0;
  return b;
}

Block *next_block(Block *b) {
  b->next = new_block();

  return b->next;
}


IfBlock *new_if_block(Block *b, char* condition) {
  Statement *s = b->statement;

  s->kind = statement_if_kind;
  IfBlock *ib = &s->if_block;

  ib->condition = make_unit(condition);

  ib->if_block = new_block();
  ib->if_block->statement = new_statement();

  ib->else_block = new_block();
  ib->else_block->statement = new_statement();
  return ib;
}

Assign *new_assign(Block *b, char* type, char* name) {
  Statement *s = b->statement;
  s->kind = statement_assign_kind;
  Assign *assign = &s->assign;
  assign->name = make_unit(name);
  assign->type = make_type(type, 0);
  assign->expr = malloc(sizeof(Expr));

  return assign;
}

Return *new_return(Block *b) {
  Statement *s = b->statement;
  s->kind = statement_return_kind;
  Return *r = &s->return_block;

  return r;
}

BinOp* new_binop(Expr *expr, char *lhs, char *op, char *rhs) {
  expr->kind = expr_binop_kind;
  expr->binop.lhs = lhs;
  expr->binop.op = op;
  expr->binop.rhs = rhs;

  return &expr->binop;
}

PointerCall *new_pointer_call(Block *b, char* ret, char* name, Parameters params) {
  b->statement->kind = statement_pointer_call_kind;
  PointerCall *ptr_call = &b->statement->pointer_call;
  ptr_call->operand = make_unit(name);
  ptr_call->return_type = make_type(ret, 0);
  ptr_call->parameters = params;
  return ptr_call;
}

