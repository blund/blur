#ifndef BLUR_COPY_AND_PATCH_H
#define BLUR_COPY_AND_PATCH_H

#include "stencil.h"
#include "ast/traverse.h"

typedef struct {
  uint8_t *code;
  int write_head;
  int capacity;
} ExecutableMemory;

typedef struct {
  ExecutableMemory mem;
  Stencil add_stencil;
  Stencil if_stencil;
  uint8_t **loc_stack;
  void (*print_result)(uintptr_t stack, int result); // For debugging;
} CompileContext;

void patch_hole_64(uint8_t *code, Stencil *s, int index, uint64_t value);
void patch_hole_32(uint8_t* code, Stencil* s, int index, uint32_t value);
uint8_t *copy_stencil(ExecutableMemory *em, Stencil *s);
void copy_and_patch(NodeType type, void *node, TraverseCtx *ctx, TraversalType traversal);
ExecutableMemory make_executable_memory();
#endif
