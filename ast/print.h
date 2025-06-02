#ifndef BLUR_AST_PRINT_H
#define BLUR_AST_PRINT_H

#include <ast/ast.h>

void indent();
void print_lit(Literal *l);
void print_expr(Expression *e);
void print_call(Call *c);
void print_assign(Assign *a);
void print_block(Block *b);
void print_if(If *i);
void print_statement(Statement *s);
void print_block(Block *b);

#endif
