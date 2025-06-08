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

void final(uintptr_t stack, int result) {
  printf("From continuation passing: %d\n", result);
}

typedef int (*AddType)(uintptr_t, int);

int main() {
  CopyPatchContext ctx = make_context("../generated/index.bin", "../generated/code_blob.bin");
  StencilKey sk = {DIV_OP, VAR_ARG, REG_ARG, 0};

  uint8_t *bin = copy_stencil(sk, &ctx);
  if (!bin) { puts("No such stencil!"); }
  patch_hole_32(bin, sk, 0, 4, &ctx);
  patch_hole_64(bin, sk, 0, (uint64_t)final, &ctx);

  AddType fn = (AddType)(bin);

  uintptr_t stack[32] = {0};
  *(int*)((uint8_t*)stack + 4) = 8;
  
  fn((uintptr_t)stack, 2);
}

