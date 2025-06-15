#include <stdarg.h>

#include <include/stb_ds.h>

#include <ast/ast.h>
#include <ast/build.h>

Block *block_empty() {
  Block *b = malloc(sizeof(Block));
  b->node_type = block_node;
  b->statements = NULL;

  return b;
}

Block *block_multi(int count, ...) {
  static int counter = 0;
  Block* b = block_empty();

  va_list args;
  va_start(args, count);
  for (int i = 0; i < count; ++i) {
    Statement *s = va_arg(args, Statement*);
    arrput(b->statements, s); // stb_ds dynamic array
  }
  va_end(args);
  return b;
}

Parameters *params_multi(int count, ...) {
  static int counter = 0;
  Parameters *p = malloc(sizeof(Block));
  p->node_type = params_node;
  p->entries = NULL;

  va_list args;
  va_start(args, count);
  for (int i = 0; i < count; ++i) {
    Var s = va_arg(args, Var);
    arrput(p->entries, s); // stb_ds dynamic array
  }
  va_end(args);
  return p;
}

Arguments *args_empty() {
  Arguments *a = malloc(sizeof(Block));
  a->node_type = args_node;
  a->entries = NULL;

  return a;
}

Arguments *args_multi(int count, ...) {
  static int counter = 0;
  Arguments *p = args_empty();

  va_list args;
  va_start(args, count);
  for (int i = 0; i < count; ++i) {
    Expression* s = va_arg(args, Expression*);
    arrput(p->entries, s); // stb_ds dynamic array
  }
  va_end(args);
  return p;
}


Type type(char *name) {
  Type v;
  v.node_type = type_node;
  v.name = name;
  return v;
}

Var var(char *name, Type type) {
  Var v;
  v.node_type = var_node;
  v.name = name;
  v.type = type;
  return v;
}

Statement *if_test(Expression* condition, Block *s1, Block* s2) {
  Statement *s = malloc(sizeof(Statement));
  s->node_type = statement_node;
  s->kind = if_statement;

  If *i = &s->if_block;
  i->node_type = if_node;
  i->condition = condition;
  i->then_branch = s1;
  i->else_branch = s2;
  return s;
}

Statement *let(char* name, Type type ,Expression* e) {
  Statement *s = malloc(sizeof(Statement));
  s->node_type = statement_node;
  s->kind = let_statement;

  Let* l = &s->let;
  l->node_type = let_node;
  l->name = name;
  l->type = type;
  l->expr = e;
  return s;
}

Statement *call(char* name, Arguments *args) {
  Statement *s = malloc(sizeof(Statement));
  s->node_type = statement_node;
  s->kind = call_statement;

  Call* c = &s->call;
  c->node_type = call_node;
  c->name = name;
  c->args = args;
  return s;
}

Expression *call_e(char* name, Arguments *args) {
  Expression *e = malloc(sizeof(Expression));
  e->node_type = expression_node;
  e->kind = call_expr;

  Call* c = &e->call;
  c->node_type = call_node;
  c->name = name;
  c->args = args;
  return e;
}


Expression *identifier(char* name) {
  Expression* e = malloc(sizeof(Expression));
  e->node_type = expression_node;
  e->kind = lit_expr;

  Literal *l = &e->lit;
  l->node_type = literal_node;
  l->kind = identifier_lit;
  l->identifier = name;
  return e;
}

Expression *integer(int n) {
  Expression* e = malloc(sizeof(Expression));
  e->node_type = expression_node;
  e->kind = lit_expr;

  Literal *l = &e->lit;
  l->node_type = literal_node;
  l->kind = integer_lit;
  l->integer = n;
  return e;
}

FuncDecl *func_decl(Type ret, char* name, Parameters *params, Block* body) {
  FuncDecl *fd = malloc(sizeof(FuncDecl));
  fd->node_type = func_decl_node;
  fd->ret = ret;
  fd->name = name;
  fd->params = params;
  fd->body = body;

  return fd;
}

