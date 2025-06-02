#ifndef BLUR_IR_PRINT_H
#define BLUR_IR_PRINT_H

#include <ast/ast.h>
#include <ast/build.h>

#include <ir/ir.h>

void print_literal(IrLiteral *lit);
void print_ir(IrNode *head);
void compile(IrNode *head);

#endif
