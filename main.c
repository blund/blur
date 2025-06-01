#include <string.h>
#include <sys/mman.h>

#define BL_STRINGBUILDER_IMPL
#define BL_IMPL
#include "bl.h"

#define STB_DS_ASSERT assert
#define STB_DS_IMPLEMENTATION
#include "include/stb_ds.h"

#include "ast/ast.h"
#include "ast/build.h"
#include "ast/traverse.h"
#include "ast/traversers.h"

#include "copy_and_patch.h"
#include "stencil.h"


Block *example_ast();

// Functions used for testing cps functionlaity
void print_result(uintptr_t stack, int result) {
  printf("From continuation passing: %d\n", result);
}

int main() {
  dprintf(" Running copy-patch compiler...\n");
  CompileContext cc;
  cc.mem = make_executable_memory();
  cc.add_stencil = read_stencil("generated/stencils/add_const.bin");
  cc.if_stencil = read_stencil("generated/stencils/if_test.bin");
  cc.print_result = &print_result;
  cc.loc_stack = NULL;

  Block *b = example_ast();

  // Traverse block to gather aliveness
  TraverseCtx ctx;

  UsedVarSet **set_stack = NULL;
  UsedVarSet *initial_set = NULL;
  arrput(set_stack, initial_set);

  // Find all used vars in blocks
  ctx = (TraverseCtx){.traversal=post_order, .data = &set_stack};
  traverse_block(b, collect_used_vars, &ctx);

  // Print AST
  dprintf("Printing ast...\n");
  ctx = (TraverseCtx){.traversal= pre_order, .data = 0};
  traverse_block(b, print_ast, &ctx);

  // Build executable
  ctx = (TraverseCtx){.traversal= pre_order, .data = &cc};
  traverse_block(b, copy_and_patch, &ctx);

  int stack_[1024];
  uintptr_t stack = (uintptr_t)&stack;

  // Call the compiled code
  void (*func)(uintptr_t, int, int) = (void (*)(uintptr_t, int, int))cc.mem.code;
  func(stack, 0, 2);
  
  return 0;
}

Block* example_ast() {
  return new_block(
	     new_assign("test", new_integer(3)),
             new_if(new_integer(1),
                    new_block(new_call(
                        "add", (Arguments){.count = 2,
                                           .entries = {new_identifier("test"),
                                                       new_integer(4)}})),
                    new_block(new_call(
                        "add", (Arguments){.count = 2,
                                           .entries = {new_identifier("test"),
                                                       new_integer(7)}}))));
}
