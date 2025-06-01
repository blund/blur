#ifndef BLUR_CPS_PRINT_H
#define BLUR_CPS_PRINT_H

#include "../ast/ast.h"
#include "../ast/build.h"

#include "cps.h"


void print_literal(CpsLiteral *lit);
void print_cps_graph(CpsNode *head);

#endif
