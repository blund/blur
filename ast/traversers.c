
#include "stdio.h"

#include "../include/stb_ds.h"

#include "ast.h"
#include "traverse.h"


UsedVarSet *clone_set(UsedVarSet *src) {
  UsedVarSet *copy = NULL;
  for (int i = 0; i < hmlen(src); ++i) {
    hmput(copy, src[i].key, 1);
  }
  return copy;
}

void push_scope(UsedVarSet ***stack) {
  UsedVarSet *empty = NULL;
  arrput(*stack, empty);
}

UsedVarSet *pop_scope(UsedVarSet ***stack) { return arrpop(*stack); }

void collect_used_vars(NodeType type, void *node, TraverseCtx *ctx, TraversalType traversal) {
  UsedVarSet ***set_stack = ctx->data;
  UsedVarSet **set = &arrlast(*set_stack);

  if (traversal == pre_order) {
    switch (type) {
    case block_node: {
      Block *b = node;
      push_scope(set_stack);
    } break;

    default: break;
    }
    return;
  }

  
  if (traversal == post_order) {
    switch (type) {
    case block_node: {
      Block *b = node;
      UsedVarSet *popped = arrpop(*set_stack); // pop

      // Merge children’s used_vars
      for (int i = 0; i < b->count; ++i) {
	Statement *stmt = b->statements[i];

	// If the statement contains a nested block (e.g., if-statement),
	// you'll want to look inside that and merge its used_vars.
        if (stmt->kind == if_statement) {
	  If *ifstmt = &stmt->if_block;
          if (ifstmt->then_branch) {
            Block *branch = ifstmt->then_branch;
	    merge_sets(&popped, ifstmt->then_branch->used_vars);
	  }
	  if (ifstmt->else_branch) {
	    Block *branch = ifstmt->else_branch;
	    merge_sets(&popped, ifstmt->else_branch->used_vars);
	  }
	}
      }

      b->used_vars = clone_set(popped);
    } break;

    case literal_node: {
      Literal *lit = node;
      if (lit->kind == identifier_lit) {
	hmput(*set, lit->identifier, 1);
	//printf("  literal '%s'\n", lit->identifier);
      }
    } break;

    case assign_node: {
      Assign *a = node;
      //printf("  assign '%s'\n", a->name);
      //if (!hmget(*set, a->name)) {
	//printf("DEAD: %s is never used\n", a->name);
      //}
    } break;

    default:
      break;
    }
  }
}

void indent_(int depth) {
  for (int i = 0; i < depth; ++i) {
    printf("  "); // 2 spaces per level
  }
}

void print_ast(NodeType type, void *node, TraverseCtx *ctx, TraversalType traversal) {
  int* depth = (int*)&ctx->data;

  if (traversal == post_order) {
    if (type == block_node) {
      *depth -= 1;
    }
    if (type == assign_node) {
      *depth -= 1;
    }
    if (type == if_node) {
      *depth -= 1;
    }
    if (type == call_node) {
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
      printf("(Literal var '%s')\n", lit->identifier);
      break;

    case integer_lit:
      printf("(Literal int %d)\n", lit->integer);
      break;

    default: break;
    }
  } break;

  case assign_node: {
    indent_(*depth);
    *depth += 1;
    Assign *a = node;
    printf("(Assign '%s')\n", a->name);
  } break;

  case call_node: {
    indent_(*depth);
    *depth += 1;
    Call *c = node;
    printf("(Call %s)\n", c->name);
  } break;

  case statement_node: {
  } break;

  case block_node: {
    indent_(*depth);
    *depth += 1;
    Block *b = node;
    printf("(Block)\n");
	   
  } break;

  case if_node: {
    indent_(*depth);
    *depth += 1;
    printf("(If block)\n");
  } break;

  case expression_node: {
  } break;

  default:
    break;
  }
  }
}
