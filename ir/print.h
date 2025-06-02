#ifndef BLUR_CPS_PRINT_H
#define BLUR_CPS_PRINT_H

#include "../ast/ast.h"
#include "../ast/build.h"

#include "ir.h"


void print_literal(CpsLiteral *lit);
void print_cps_graph(CpsNode *head);
void compile(CpsNode *head);

#endif
