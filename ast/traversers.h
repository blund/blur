#ifndef BLUR_TRAVERSERS_H
#define BLUR_TRAVERSERS_H

#include <ast/ast.h>
#include <ast/traverse.h>

void collect_used_vars(NodeType type, void *node, TraverseCtx *ctx, TraversalType traversal);
void print_ast(NodeType type, void *node, TraverseCtx *ctx, TraversalType traversal);

#endif
