#ifndef BLUR_IR_TRANSFORM_H
#define BLUR_IR_TRANSFORM_H

#include <ast/ast.h>
#include <ast/build.h>

#include <ir/ir.h>

typedef struct {
  const char* key;
  int value;
} VarSlot;

IrNode *transform_ast(Block* b);
IrNode *transform_ast2(Block* b);
int stack_index(const char *name);

#endif
