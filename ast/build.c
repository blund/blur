#include "malloc.h"

#include "../include/stb_ds.h"
#include "../bl.h"

#include "ast.h"
#include "build.h"

#include <stdarg.h>

Block *new_block_multi(int count, ...) {
  static int counter = 0;
  Block *b = calloc(1, sizeof(Block));
  b->node_type = block_node;
    b->statements = NULL;
    b->count = count;

    va_list args;
    va_start(args, count);
    for (int i = 0; i < count; ++i) {
        Statement *s = va_arg(args, Statement *);
        arrput(b->statements, s); // stb_ds dynamic array
    }
    va_end(args);
    return b;
}

Statement *new_if(Expression* condition, Block *s1, Block* s2) {
  Statement *s = malloc(sizeof(Statement));
  s->node_type = statement_node;

  s->kind = if_statement;
  If *i = &s->if_block;
  i->condition = condition;
  i->then_branch = s1;
  i->else_branch = s2;
  return s;
}

Statement *new_assign(char* name, Expression* e) {
  Statement *s = malloc(sizeof(Statement));
  s->node_type = statement_node;
  s->kind = assign_statement;
  Assign* a = &s->assign;
  a->name = name;
  a->expr = e;
  return s;
}

Statement *new_call(char* name, Arguments args) {
  Statement *s = malloc(sizeof(Statement));
  s->node_type = statement_node;
  s->kind = call_statement;
  s->call.name = name;
  s->call.args = args;
  return s;
}

Expression *new_identifier(char* name) {
  Expression* e = malloc(sizeof(Expression));
  e->node_type = expression_node;
  e->kind = lit_expr;
  e->lit = (Literal){.node_type = literal_node, .kind = identifier_lit, .identifier = name};
  return e;
}

Expression *new_integer(int n) {
  Expression* e = malloc(sizeof(Expression));
  e->node_type = expression_node;
  e->kind = lit_expr;
  e->lit = (Literal){.node_type = literal_node, .kind = integer_lit, .integer = n};
  return e;
}
