
#include <string.h>
#include <sys/mman.h>

#include "include/stb_ds.h"

#include "bl.h"

#include "copy_and_patch.h"
#include "cps/cps.h"

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

CpsNode *_get_node(CpsNode* node, int count, int index) {
  if (index == count)
    return node;
  return _get_node(node->next, count, index+1);
}

CpsNode *get_node(CpsNode *head, int n) { return _get_node(head, n, 0); }

typedef struct {
  int key;
  uint8_t* value;  // can be anything; `true` is good enough
} CodeLocation;

void copy_and_patch(CpsNode *head, CompileContext* cc) {
  CodeLocation *l = NULL;

  // First pass, copy machine code, patch primitives
  for (CpsNode *n = head; n != NULL; n = n->next) {
    switch (n->kind) {
    case CPS_LET: {
      break;
    }

    case CPS_CALL: {
      CpsCall *c = &n->call_node;
      // @TODO - select stencil from args
      Stencil *add_stencil = hmget(cc->stencils, "add");
      uint8_t *add_loc = copy_stencil(&cc->mem, add_stencil);

      patch_hole_32(add_loc, add_stencil, 0, c->args[1].integer);
      patch_hole_64(add_loc, add_stencil, 0, (uint64_t)cc->print_result);

      hmput(l, n->label, add_loc);
      break;
    }

    case CPS_IF: {
      CpsIf *iff = &n->if_node;
      Stencil *if_stencil = hmget(cc->stencils, "if");
      uint8_t *if_loc = copy_stencil(&cc->mem, if_stencil);
      hmput(l, n->label, if_loc);
      break;
    }

    default: break;
    }
  }

  // Second pass, patch dependent holes
  for (CpsNode *n = head; n != NULL; n = n->next) {
    switch (n->kind) {
    case CPS_IF: {
      CpsIf *iff = &n->if_node;
      uint8_t *if_loc = hmget(l, n->label);

      uint8_t *branch1_loc = hmget(l, iff->then_label);
      uint8_t *branch2_loc = hmget(l, iff->else_label);

      Stencil *if_stencil = hmget(cc->stencils, "if");
      patch_hole_64(if_loc, if_stencil, 0, (uint64_t)branch1_loc);
      patch_hole_64(if_loc, if_stencil, 1, (uint64_t)branch2_loc);
    } break;

    default: break;
    }
  }

}
