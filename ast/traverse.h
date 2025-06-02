#ifndef BLUR_AST_TRAVERSE_H
#define BLUR_AST_TRAVERSE_H

#include <ast/ast.h>

typedef enum {
  pre_order,
  post_order,
} TraversalType;

typedef struct {
  TraversalType traversal;
  void* data;
} TraverseCtx;

void merge_sets(UsedVarSet **dst, UsedVarSet *src);

typedef void (*Visit)(NodeType nt, void* data, TraverseCtx* ctx, TraversalType traversal);

void traverse_lit(Literal *l, Visit v, TraverseCtx* ctx);
void traverse_expr(Expression *e, Visit v, TraverseCtx* ctx);
void traverse_call(Call *c, Visit v, TraverseCtx* ctx);
void traverse_assign(Assign *a, Visit v, TraverseCtx* ctx);
void traverse_block(Block *b, Visit v, TraverseCtx* ctx);
void traverse_if(If *i, Visit v, TraverseCtx* ctx);
void traverse_statement(Statement *s, Visit v, TraverseCtx* ctx);
void traverse_block(Block *b, Visit v, TraverseCtx* ctx);

#endif
