#include "print.h"
#include "stdio.h"

#include "../bl.h"

int indent_level = 0;
void indent() {
    for (int i = 0; i < indent_level; ++i) printf("  ");
}

void print_lit(Literal *l) {
  switch (l->kind) {
  case integer_lit: printf("%d", l->integer); break;
  case floating_lit: printf("%f", l->floating); break;
  case string_lit: printf("%s", l->string); break;
  case identifier_lit: printf("'%s'", l->identifier); break;
  }
}


void print_expr(Expression *e);
void print_call(Call *c) {
  printf("(Call '%s'", c->name);
  fori(c->args.count) {
    printf(" ");
    print_expr(c->args.entries[i]);
  }
  printf(")\n");
}

void print_expr(Expression *e) {
  switch (e->kind) {
  case call_expr: print_call(&e->call);
  case lit_expr: print_lit(&e->lit);
  }
}

void print_assign(Assign *a) {
  printf("(Assign %s ", a->name);
  print_expr(a->expr);
  puts(")");
}


void print_block(Block *b);

void print_if(If *i) {
  printf("(If ");
  print_expr(i->condition); printf("\n");
  print_block(i->then_branch);
  if (i->else_branch)
    print_block(i->else_branch);
  indent_level--;
  indent();
  printf(")\n");
}

void print_statement(Statement *s) {
  indent_level++;
  switch (s->kind) {
  case assign_statement: print_assign(&s->assign); break;
  case call_statement: print_call(&s->call); break;
  case if_statement: print_if(&s->if_block); break;
  }
  indent_level--;
}

void print_block(Block *b) {
  indent();
  printf("(Block,\n");
  indent_level++;
  fori(b->count) {
    indent();
    print_statement(b->statements[i]);
  } 
  indent_level--;
  indent();
  printf(")\n");
}

