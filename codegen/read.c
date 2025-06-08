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

  StencilKey sk = {ADD_OP, LIT_ARG, VAR_ARG, 0};

  uint8_t *bin = copy_stencil(sk, &ctx);
  patch_hole_32(bin, sk, 0, 13, &ctx);
  patch_hole_32(bin, sk, 1, 0, &ctx);
  //patch_hole_32(bin, sk, 1, 2, &ctx);
  patch_hole_64(bin, sk, 0, (uint64_t)final, &ctx);

  AddType fn = (AddType)(bin);

  uint64_t stack[32] = {0};
  stack[0] = 11;
  
  fn((uintptr_t)stack, 3);
}

