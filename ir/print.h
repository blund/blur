#ifndef BLUR_CPS_PRINT_H
#define BLUR_CPS_PRINT_H

#include <ast/ast.h>
#include <ast/build.h>

#include <ir/ir.h>


void print_literal(IrLiteral *lit);
void print_cps_graph(IrNode *head);
void compile(IrNode *head);

#endif
