#ifndef BLUR_COPY_AND_PATCH_H
#define BLUR_COPY_AND_PATCH_H

#include <ast/traverse.h>
#include <ir/ir.h>

#include <stencil.h>

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
  ExecutableMemory mem;
  uint8_t *code_blob;
  StencilMap* stencil_map;
  void (*final)(uintptr_t stack, int result); // For debugging;
} CopyPatchContext;

typedef struct {
  ExecutableMemory mem;
  void (*final)(uintptr_t stack, int result); // For debugging;
  StencilMap *stencils;
} CompileContext;

CopyPatchContext make_context(char *index_path, char *code_blob_path);
ExecutableMemory make_executable_memory();
uint8_t *copy_stencil(StencilKey sk, CopyPatchContext *ctx);
void copy_and_patch(IrNode *head, CopyPatchContext* ctx);
void patch_hole_32(uint8_t *code, StencilKey sk, int index, uint32_t value, CopyPatchContext* ctx);
void patch_hole_64(uint8_t *code, StencilKey sk, int index, uint64_t value, CopyPatchContext* ctx);
#endif
