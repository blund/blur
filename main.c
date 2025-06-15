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

#include <parser/parser.h>

#include <ir/ir.h>
#include <ir/print.h>
#include <ir/transform.h>

#include <copy_and_patch/copy_and_patch.h>
#include <copy_and_patch/stencil.h>


// Function used for testing cps functionlaity
void final(uintptr_t stack, int result) {
  printf("From continuation passing: %d\n", result);
}

typedef int (*Fn)(uintptr_t);

int main() {
  dprintf("\n [ Running Compiler ]\n");

  CopyPatchContext ctx = make_context("generated/index.bin",
                                      "generated/code_blob.bin");

  ctx.final = final;

  char *program = "{ let test = 3; if (1) { add(2,3); } { mul(4, 5); } }";

  Block *b = parse(program);

  traverse_block(b, print_ast, &(TraverseCtx){.traversal=pre_order, .data=0});

  IrNode *n = transform_ast(b);
  print_ir(n);

  copy_and_patch(n, &ctx);

  uintptr_t stack[32] = {0};
  Fn fn = (Fn)ctx.mem.code;

  fn((uintptr_t)stack);

  return 0;
}
