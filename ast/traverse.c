#include "traverse.h"

#include "../include/stb_ds.h"
#include "../bl.h"


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
  // @TODO - how do we check if the variable was never defined?
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
