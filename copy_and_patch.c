
#include <string.h>
#include <sys/mman.h>

#include "include/stb_ds.h"

#include "bl.h"

#include "copy_and_patch.h"
#include "ast/traverse.h"

void patch_hole_32(uint8_t* code, Stencil* s, int index, uint32_t value) {
  int code_index = s->holes_32[index];
  *(uint32_t*)&(code[code_index]) = value;
}

void patch_hole_64(uint8_t* code, Stencil* s, int index, uint64_t value) {
  int code_index = s->holes_64[index];
  *(uint64_t*)&(code[code_index]) = value;
}

ExecutableMemory make_executable_memory() {
  // Allocate a buffer with RWX permissions (on Linux)
  ExecutableMemory em = {(void*)-1, 0, 4096*8};

  void *memory = mmap(NULL, em.capacity, PROT_READ | PROT_WRITE | PROT_EXEC,
                      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

  if (memory == MAP_FAILED) {
    perror("mmap");
    return em;
  }

  em.code = memory;
  return em;
}

uint8_t* copy_stencil(ExecutableMemory *em, Stencil *s) {
  memcpy(&em->code[em->write_head], s->code, s->code_size);
  uint8_t* location = &em->code[em->write_head];
  em->write_head += s->code_size;
  return location;
}

void copy_and_patch(NodeType type, void *node, TraverseCtx *ctx, TraversalType traversal) {
  CompileContext *cc = ctx->data;

  static UsedVarSet* set;

  if (traversal == pre_order) {
    switch (type) {

    case block_node: {
      Block* b = node;
      set = b->used_vars;
    } break;

    case call_node: {
      Call *c = node;
      uint8_t *add_loc = copy_stencil(&cc->mem, &cc->add_stencil);

      patch_hole_32(add_loc, &cc->add_stencil, 0, c->args.entries[1]->lit.integer);
      patch_hole_64(add_loc, &cc->add_stencil, 0, (uint64_t)cc->print_result);

      arrput(cc->loc_stack, add_loc);
    } break;

    case if_node: {
      uint8_t *if_loc = copy_stencil(&cc->mem, &cc->if_stencil);
      arrput(cc->loc_stack, if_loc);
    } break;

    default:
      break;
    }
  }

  if (traversal == post_order) {
    if (type == if_node) {
      uint8_t* add1_loc = arrpop(cc->loc_stack);
      uint8_t* add2_loc = arrpop(cc->loc_stack);
      uint8_t *if_loc = arrpop(cc->loc_stack);
      patch_hole_64(if_loc, &cc->if_stencil, 0, (uint64_t)add1_loc);
      patch_hole_64(if_loc, &cc->if_stencil, 1, (uint64_t)add2_loc);

    }
  }
}
