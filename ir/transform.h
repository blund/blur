#ifndef BLUR_CPS_TRANSFORM_H
#define BLUR_CPS_TRANSFORM_H

#include "ast/ast.h"
#include "ast/build.h"

#include "ir.h"

typedef struct {
  const char* key;
  int value;
} VarSlot;

CpsNode *transform_ast(Block* b);

#endif
