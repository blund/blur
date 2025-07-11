
#include "stdio.h"

#include <include/stb_ds.h>
#include <bl.h>

#include <ast/ast.h>
#include <ast/traverse.h>

void indent_(int depth) {
  for (int i = 0; i < depth; ++i) {
    dprintf("  "); // 2 spaces per level
  }
}

void print_ast(NodeType type, void *node, TraverseCtx *ctx, TraversalType traversal) {
  int* depth = (int*)&ctx->data;

  if (traversal == post_order) {
    if (type == block_node) {
      *depth -= 1;
    }
    if (type == let_node) {
      *depth -= 1;
    }
    if (type == if_node) {
      *depth -= 1;
    }
    if (type == call_node) {
      *depth -= 1;
    }
    if (type == var_node) {
      *depth -= 1;
    }
  }


  if (traversal == pre_order) {
  switch (type) {
  case literal_node: {
    Literal *lit = node;
    indent_(*depth);

    switch(lit->kind) {
    case identifier_lit:
      dprintf("(Literal var '%s')\n", lit->identifier);
      break;

    case integer_lit:
      dprintf("(Literal int %d)\n", lit->integer);
      break;

    default: break;
    }
  } break;

  case let_node: {
    indent_(*depth);
    *depth += 1;
    Let *a = node;
    dprintf("(Let '%s')\n", a->name);
  } break;

  case call_node: {
    indent_(*depth);
    *depth += 1;
    Call *c = node;
    dprintf("(Call %s)\n", c->name);
  } break;

  case block_node: {
    indent_(*depth);
    *depth += 1;
    Block *b = node;
    dprintf("(Block)\n");
	   
  } break;

  case if_node: {
    indent_(*depth);
    *depth += 1;
    dprintf("(If block)\n");
  } break;

  case func_decl_node: {
    indent_(*depth);
    *depth += 1;
    FuncDecl *fd = node;
    dprintf("(FuncDecl %s) \n", fd->name);

  } break;

  case type_node: {
    indent_(*depth);
    Type *t = node;
    dprintf("(Type %s) \n", t->name);
  } break;

  case var_node: {
    indent_(*depth);
    *depth += 1;
    Var *t = node;
    dprintf("(Var %s) \n", t->name);
  } break;

  default: break;
  }
  }
}
