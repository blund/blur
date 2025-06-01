#include "traverse.h"

#include "../include/stb_ds.h"
#include "../bl.h"
void merge_sets(UsedVarSet **dst, UsedVarSet *src) {
  for (int i = 0; i < hmlen(src); ++i) {
    hmput(*dst, src[i].key, 1);
  }
}

void traverse_lit(Literal *l, Visit v, void *ctx) {
  switch (l->kind) {
  case integer_lit: break;
  case floating_lit: break;
  case string_lit: break;
  case identifier_lit: break;
  }
  v(l->node_type, l, ctx);
}


void traverse_expr(Expression *e, Visit v, void* ctx);
void traverse_call(Call *c, Visit v, void* ctx) {
  fori(c->args.count) {
    traverse_expr(c->args.entries[i], v, ctx);
  }
  v(c->node_type, c, ctx);
}

void traverse_expr(Expression *e, Visit v, void* ctx) {
  switch (e->kind) {
  case call_expr: traverse_call(&e->call, v, ctx); break;
  case lit_expr: traverse_lit(&e->lit, v, ctx); break;
  }
  v(e->node_type, e, ctx);
}

void traverse_assign(Assign *a, Visit v, void* ctx) {
  traverse_expr(a->expr, v, ctx);
  v(a->node_type, a, ctx);
}


void traverse_block(Block *b, Visit v, void* ctx);

void traverse_if(If *i, Visit v, void* ctx) {
  traverse_expr(i->condition, v, ctx);
  traverse_block(i->then_branch, v, ctx);
  if (i->else_branch) {
    traverse_block(i->else_branch, v, ctx);
  }
  v(i->node_type, i, ctx);
}

void traverse_statement(Statement *s, Visit v, void* ctx) {
  switch (s->kind) {
  case assign_statement: traverse_assign(&s->assign, v, ctx); break;
  case call_statement: traverse_call(&s->call, v, ctx); break;
  case if_statement: traverse_if(&s->if_block, v, ctx); break;
  }
  v(s->node_type, s, ctx);
}

void traverse_block(Block *b, Visit v, void* ctx) {
  fori(b->count) {
    traverse_statement(b->statements[i], v, ctx);
  }
  v(b->node_type, b, ctx);
}
