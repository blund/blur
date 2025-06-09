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

typedef int (*Fn)(uintptr_t);
typedef int (*FnInt)(uintptr_t, int);
typedef int (*FnIntInt)(uintptr_t, int, int);

int main() {
  CopyPatchContext ctx =
      make_context("../generated/index.bin", "../generated/code_blob.bin");

  StencilKey if_sk = {IF_OP, REG_ARG, ARG_NONE, 0};

  uint8_t *if_bin = copy_stencil(if_sk, &ctx);
  
  StencilKey sk = {ADD_OP, VAR_ARG, VAR_ARG, 0};
  uint8_t *bin1 = copy_stencil(sk, &ctx);
  if (!bin1) { puts("No such stencil!"); }
  patch_hole_32(bin1, sk, 0, 0, &ctx);
  patch_hole_32(bin1, sk, 1, 4, &ctx);
  patch_hole_64(bin1, sk, 0, (uint64_t)final, &ctx);

  uint8_t *bin2 = copy_stencil(sk, &ctx);
  if (!bin2) { puts("No such stencil!"); }
  patch_hole_32(bin2, sk, 0, 0, &ctx);
  patch_hole_32(bin2, sk, 1, 8, &ctx);
  patch_hole_64(bin2, sk, 0, (uint64_t)final, &ctx);

  patch_hole_64(if_bin, if_sk, 0, (uint64_t)bin1, &ctx);
  patch_hole_64(if_bin, if_sk, 1, (uint64_t)bin2, &ctx);

  uintptr_t stack[32] = {0};
  *(int*)((uint8_t*)stack + 0) = 1;
  *(int*)((uint8_t*)stack + 4) = 5;
  *(int*)((uint8_t*)stack + 8) = 10;

  FnInt fn = (FnInt)if_bin;

  fn((uintptr_t)stack, 1);
}

