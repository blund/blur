#include <string.h>
#include <sys/mman.h>

#define BL_STRINGBUILDER_IMPL
#define BL_IMPL
#include <bl.h>

#define STB_DS_ASSERT assert
#define STB_DS_IMPLEMENTATION
#include <include/stb_ds.h>

#include <ast/ast.h>
#include <ast/build.h>
#include <ast/traverse.h>
#include <ast/traversers.h>

#include <ir/ir.h>
#include <ir/print.h>
#include <ir/transform.h>

#include <copy_and_patch.h>
#include <stencil.h>

Block *example_ast();

// Functions used for testing cps functionlaity
void print_result(uintptr_t stack, int result) {
  printf("From continuation passing: %d\n", result);
}

int main() {
  dprintf("\n [ Running Compiler ]\n");
  CompileContext cc;
  cc.mem = make_executable_memory();
  cc.print_result = &print_result;
  cc.stencils = NULL;

  CallSignature add_cs = {"add", {ARG_REG, ARG_IMM}};
  hmput(cc.stencils, add_cs, read_stencil("generated/stencils/add_const.bin"));
  CallSignature if_cs = {"if", {ARG_REG, ARG_REG}};
  hmput(cc.stencils, if_cs,  read_stencil("generated/stencils/if_test.bin"));

  // Build example AST to compile
  dprintf("\n -- Building AST\n");
  Block *b = example_ast();
  traverse_block(b, print_ast, &(TraverseCtx){.traversal=pre_order, .data=0});

  // Construct continuation passing style graph of our ast
  dprintf("\n -- Constructng IR \n");
  IrNode *n = transform_ast(b);
  print_ir(n);

  // Do the magic
  dprintf("\n -- Compiling\n");
  copy_and_patch(n, &cc);

  // Execute the compiled code
  dprintf(" -- Running code\n");
  int stack_[1024];
  uintptr_t stack = (uintptr_t)&stack;
  void (*func)(uintptr_t, int, int) = (void (*)(uintptr_t, int, int))cc.mem.code;
  func(stack, 0, 2);
  
  return 0;
}

Block* example_ast() {
  return block(let("test", type("int"), integer(3)),
               if_test(integer(1),
                       block(call("add", args(identifier("test"), integer(4)))),
                       block(call("add", args(identifier("test"), integer(7))))));
}

