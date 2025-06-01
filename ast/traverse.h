#ifndef BLUR_AST_TRAVERSE_H
#define BLUR_AST_TRAVERSE_H

#include "ast.h"

void merge_sets(UsedVarSet **dst, UsedVarSet *src);

typedef void (*Visit)(NodeType nt, void* data, void* ctx);

void traverse_lit(Literal *l, Visit v, void* ctx);
void traverse_expr(Expression *e, Visit v, void* ctx);
void traverse_call(Call *c, Visit v, void* ctx);
void traverse_assign(Assign *a, Visit v, void* ctx);
void traverse_block(Block *b, Visit v, void* ctx);
void traverse_if(If *i, Visit v, void* ctx);
void traverse_statement(Statement *s, Visit v, void* ctx);
void traverse_block(Block *b, Visit v, void* ctx);

#endif
