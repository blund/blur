#ifndef BLUR_AST_TRAVERSE_H
#define BLUR_AST_TRAVERSE_H

#include "ast.h"

void merge_sets(UsedVarSet **dst, UsedVarSet *src);

void traverse_lit(Literal *l, UsedVarSet **set);
void traverse_expr(Expression *e, UsedVarSet **set);
void traverse_call(Call *c, UsedVarSet **set);
void traverse_assign(Assign *a, UsedVarSet **set);
void traverse_block(Block *b);
void traverse_if(If *i, UsedVarSet **set);
void traverse_statement(Statement *s, UsedVarSet **set);
void traverse_block(Block *b);

#endif
