#include <stdio.h>

// For blob;
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>


#define STB_DS_IMPLEMENTATION
#include <include/stb_ds.h>

#define BL_STRINGBUILDER_IMPL
#define BL_IMPL
#include <bl.h>

#include "../generated/stencils.h"
#include <copy_and_patch.h>

#include <ast/ast.h>
#include <ast/traversers.h>
#include <ast/build.h>

#include <ir/ir.h>
#include <ir/print.h>
#include <ir/transform.h>

void final(uintptr_t stack, int result) {
  printf("From continuation passing: %d\n", result);
}

typedef int (*Fn)(uintptr_t);
typedef int (*FnInt)(uintptr_t, int);
typedef int (*FnIntInt)(uintptr_t, int, int);

Block *example_ast();

int main() {
  CopyPatchContext ctx =
      make_context("../generated/index.bin", "../generated/code_blob.bin");

  ctx.final = final;

  Block *b = example_ast();
  traverse_block(b, print_ast, &(TraverseCtx){.traversal=pre_order, .data=0});

  IrNode *n = transform_ast(b);
  print_ir(n);

  copy_and_patch2(n, &ctx);

  uintptr_t stack[32] = {0};
  Fn fn = (Fn)ctx.mem.code;

  fn((uintptr_t)stack);

}

Block* example_ast() {
  return block(
	       let("test", type("int"), integer(3)),
               if_test(integer(1),
                       block(call("add", args(identifier("test"), integer(4)))),
                       block(call("add", args(identifier("test"), integer(8))))));
}

