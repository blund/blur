#include <ast/traverse.h>

#include <include/stb_ds.h>
#include <bl.h>

void merge_sets(UsedVarSet **dst, UsedVarSet *src) {
  for (int i = 0; i < hmlen(src); ++i) {
    hmput(*dst, src[i].key, 1);
  }
}

void traverse_lit(Literal *l, Visit v, TraverseCtx *ctx) {
  v(l->node_type, l, ctx, pre_order);
  switch (l->kind) {
  case integer_lit: break;
  case floating_lit: break;
  case string_lit: break;
  case identifier_lit: break;
  }
  v(l->node_type, l, ctx,post_order);
}

void traverse_expr(Expression *e, Visit v, TraverseCtx* ctx);

void traverse_args(Arguments *a, Visit v, TraverseCtx* ctx) {
  v(a->node_type, a, ctx, pre_order);
  fori(a->count)
    traverse_expr(a->entries[i], v, ctx);
  v(a->node_type, a, ctx,post_order);
}

void traverse_call(Call *c, Visit v, TraverseCtx* ctx) {
  v(c->node_type, c, ctx, pre_order);
  traverse_args(c->args, v, ctx);
  v(c->node_type, c, ctx,post_order);
}

void traverse_expr(Expression *e, Visit v, TraverseCtx* ctx) {
  v(e->node_type, e, ctx, pre_order);
  switch (e->kind) {
  case call_expr: traverse_call(&e->call, v, ctx); break;
  case lit_expr: traverse_lit(&e->lit, v, ctx); break;
  }
  v(e->node_type, e, ctx,post_order);
}

void traverse_assign(Assign *a, Visit v, TraverseCtx* ctx) {
  v(a->node_type, a, ctx, pre_order);
  traverse_expr(a->expr, v, ctx);
  v(a->node_type, a, ctx,post_order);
}

void traverse_declare(Declare *a, Visit v, TraverseCtx* ctx) {
  v(a->node_type, a, ctx, pre_order);
  v(a->node_type, a, ctx,post_order);
}

void traverse_block(Block *b, Visit v, TraverseCtx* ctx);

void traverse_if(If *i, Visit v, TraverseCtx* ctx) {
  v(i->node_type, i, ctx, pre_order);
  traverse_expr(i->condition, v, ctx);
  traverse_block(i->then_branch, v, ctx);
  if (i->else_branch) {
    traverse_block(i->else_branch, v, ctx);
  }
  v(i->node_type, i, ctx,post_order);
}

void traverse_statement(Statement *s, Visit v, TraverseCtx* ctx) {
  v(s->node_type, s, ctx, pre_order);
  switch (s->kind) {
  case assign_statement: traverse_assign(&s->assign, v, ctx); break;
  case declare_statement: traverse_declare(&s->declare, v, ctx); break;
  case call_statement: traverse_call(&s->call, v, ctx); break;
  case if_statement: traverse_if(&s->if_block, v, ctx); break;
  }
  v(s->node_type, s, ctx,post_order);
}

void traverse_block(Block *b, Visit v, TraverseCtx *ctx) {
  if (!b->count) return;
  v(b->node_type, b, ctx, pre_order);

  if (ctx->traversal == pre_order) {
    for (int i = 0; i < b->count; i++) {
      traverse_statement(b->statements[i], v, ctx);
    }
  }

  if (ctx->traversal == post_order) {
    for(int i = b->count-1;  i >= 0; i--) {
      traverse_statement(b->statements[i], v, ctx);
    }
  }
  v(b->node_type, b, ctx, post_order);
}

void traverse_type(Type *t, Visit v, TraverseCtx *ctx) {
  v(t->node_type, t, ctx, pre_order);
  v(t->node_type, t, ctx,post_order);
}

void traverse_var(Var *t, Visit v, TraverseCtx *ctx) {
  v(t->node_type, t, ctx, pre_order);
  traverse_type(&t->type, v, ctx);
  v(t->node_type, t, ctx,post_order);
}

void traverse_params(Parameters *p, Visit v, TraverseCtx *ctx) {
  v(p->node_type, p, ctx, pre_order);
  fori(p->count)
    traverse_var(&p->entries[i], v, ctx);
  v(p->node_type, p, ctx, post_order);
}

void traverse_func_decl(FuncDecl *fd, Visit v, TraverseCtx *ctx) {
  v(fd->node_type, fd, ctx, pre_order);
  traverse_type(&fd->ret, v, ctx);
  traverse_params(fd->params, v, ctx);
  traverse_block(fd->body, v, ctx);
  v(fd->node_type, fd, ctx,post_order);
}
