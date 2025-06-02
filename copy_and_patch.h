#ifndef BLUR_COPY_AND_PATCH_H
#define BLUR_COPY_AND_PATCH_H

#include "stencil.h"
#include "ast/traverse.h"
#include "ir/ir.h"

typedef enum {
    ARG_IMM,
    ARG_REG,
    ARG_STACK
} ArgClass;

typedef struct {
    const char *op;     // e.g., "add"
    ArgClass args[2];   // assume binary ops
} CallSignature;

typedef struct {
  uint8_t *code;
  int write_head;
  int capacity;
} ExecutableMemory;

typedef struct {
  CallSignature key;
  Stencil *value;
} StencilMap;

typedef struct {
  ExecutableMemory mem;
  void (*print_result)(uintptr_t stack, int result); // For debugging;
  StencilMap *stencils;
} CompileContext;

void patch_hole_64(uint8_t *code, Stencil *s, int index, uint64_t value);
void patch_hole_32(uint8_t* code, Stencil* s, int index, uint32_t value);
uint8_t *copy_stencil(ExecutableMemory *em, Stencil *s);
ExecutableMemory make_executable_memory();
void copy_and_patch(CpsNode *head, CompileContext* cc);
#endif
