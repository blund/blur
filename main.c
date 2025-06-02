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

#include "cps/cps.h"
#include "cps/print.h"
#include "cps/transform.h"

#include "copy_and_patch.h"
#include "stencil.h"

Block *example_ast();

// Functions used for testing cps functionlaity
void print_result(uintptr_t stack, int result) {
  printf("From continuation passing: %d\n", result);
}

int main() {
  CompileContext cc;
  cc.mem = make_executable_memory();
  cc.add_stencil         = read_stencil("generated/stencils/add_const.bin");
  cc.if_stencil          = read_stencil("generated/stencils/if_test.bin");
  cc.stack_write_stencil = read_stencil("generated/stencils/stack_write.bin");
  cc.print_result        = &print_result;

  // Build example AST to compile
  dprintf("\n [ Build AST ] \n");
  Block *b = example_ast();
  traverse_block(b, print_ast, &(TraverseCtx){.traversal=pre_order, .data=0});

  // Construct continuation passing style graph of our ast
  dprintf("\n [ Construct CPS graph ] \n");
  CpsNode *n = transform_ast(b);
  print_cps_graph(n);

  // Do the magic
  dprintf("\n [ Compile ]\n");
  copy_and_patch(n, &cc);

  // Execute the compiled code
  dprintf(" [ Run ]\n");
  int stack_[1024];
  uintptr_t stack = (uintptr_t)&stack;
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
