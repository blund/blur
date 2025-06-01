
#include "malloc.h"

#define BL_IMPL
#include "bl.h"

#define STB_DS_IMPLEMENTATION
#include "include/stb_ds.h"

#include "ast.h"

#include <stdarg.h>
#define NUM_ARGS_IMPL(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,N,...) N
#define NUM_ARGS(...) NUM_ARGS_IMPL(__VA_ARGS__,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0)
#define new_block(...) new_block_multi(NUM_ARGS(__VA_ARGS__), __VA_ARGS__)

Block *new_block_multi(int count, ...) {
  static int counter = 0;
    Block *b = calloc(1, sizeof(Block));
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

/*
Block *new_block(Statement* s) {
  Block *b = malloc(sizeof(Block));
  b->statements = s;
  return b;
}

Block *next_block(Block *prev, Statement* s) {
  Block *b = malloc(sizeof(Block));
  b->statement = s;

  prev->next = b;
  return b;
}
*/

Statement *new_if(Expression* condition, Block *s1, Block* s2) {
  Statement *s = malloc(sizeof(Statement));
  s->kind = if_statement;
  If *i = &s->if_block;
  i->condition = condition;
  i->then_branch = s1;
  i->else_branch = s2;
  return s;
}

Statement *new_assign(char* name, Expression* e) {
  Statement *s = malloc(sizeof(Statement));
  s->kind = assign_statement;
  Assign* a = &s->assign;
  a->name = name;
  a->expr = e;
  return s;
}

Statement *new_call(char* name, Arguments args) {
  Statement *s = malloc(sizeof(Statement));
  s->kind = call_statement;
  s->call.name = name;
  s->call.args = args;
  return s;
}

Expression *new_identifier(char* name) {
  Expression* e = malloc(sizeof(Expression));
  e->kind = lit_expr;
  e->lit = (Literal){.kind = identifier_lit, .identifier = name};
  return e;
}

Expression *new_integer(int n) {
  Expression* e = malloc(sizeof(Expression));
  e->kind = lit_expr;
  e->lit = (Literal){.kind = integer_lit, .integer = n};
  return e;
}

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


typedef struct {
    char *key;
    int value;  // can be anything; `true` is good enough
} StringSetEntry;

StringSetEntry *set = NULL;

Block *example_ast();

void traverse_block(Block *b);
int main() {
  Block *b = example_ast();

  print_block(b);

  traverse_block(b);
  for (int i = 0; i < hmlen(b->used_vars); ++i) {
    printf("set has: %s\n", b->used_vars[i].key);
  }

}

Block* example_ast() {
  Block *b = new_block(
	     new_assign("test", new_integer(3)),
             new_if(new_integer(1),
                    new_block(new_call(
                        "add", (Arguments){.count = 2,
                                           .entries = {new_identifier("test"),
                                                       new_integer(4)}})),
                    new_block(new_call(
                        "add", (Arguments){.count = 2,
                                           .entries = {new_identifier("test"),
                                                       new_integer(7)}}))));
  return b;
}


void traverse_lit(Literal *l, UsedVarSet **set) {
  switch (l->kind) {
  case integer_lit: break;
  case floating_lit: break;
  case string_lit: break;
  case identifier_lit: {
    hmput(*set, l->identifier, 1);
    printf("read %s\n", l->identifier); 
  } break;
  }
}


void traverse_expr(Expression *e, UsedVarSet **set);
void traverse_call(Call *c, UsedVarSet **set) {
  fori(c->args.count) {
    traverse_expr(c->args.entries[i], set);
  }
}

void traverse_expr(Expression *e, UsedVarSet **set) {
  switch (e->kind) {
  case call_expr: traverse_call(&e->call, set);
  case lit_expr: traverse_lit(&e->lit, set);
  }
}

void traverse_assign(Assign *a, UsedVarSet **set) {
  printf("assign %s\n", a->name);
  traverse_expr(a->expr, set);
  if (!hmget(*set, a->name)) {
    // @TODO - handle dead code
  }
}


void traverse_block(Block *b);

void merge_sets(UsedVarSet **dst, UsedVarSet *src) {
    for (int i = 0; i < hmlen(src); ++i) {
        hmput(*dst, src[i].key, 1);
    }
}

void traverse_if(If *i, UsedVarSet **set) {
  traverse_expr(i->condition, set);
  traverse_block(i->then_branch);
  merge_sets(set, i->then_branch->used_vars);
  if (i->else_branch) {
    traverse_block(i->else_branch);
    merge_sets(set, i->else_branch->used_vars);
  }
}

void traverse_statement(Statement *s, UsedVarSet **set) {
  switch (s->kind) {
  case assign_statement: traverse_assign(&s->assign, set); break;
  case call_statement: traverse_call(&s->call, set); break;
  case if_statement: traverse_if(&s->if_block, set); break;
  }
}

void traverse_block(Block *b) {
  UsedVarSet *set = NULL;

  fori(b->count) {
    traverse_statement(b->statements[i], &set);
  }
  b->used_vars = set;
}

