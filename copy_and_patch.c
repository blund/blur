
#include <string.h>
#include <sys/mman.h>

#include <include/stb_ds.h>
#include <bl.h>

#include <ir/ir.h>
#include <ir/transform.h>

#include <copy_and_patch.h>

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

IrNode *_get_node(IrNode* node, int count, int index) {
  if (index == count)
    return node;
  return _get_node(node->next, count, index+1);
}

IrNode *get_node(IrNode *head, int n) { return _get_node(head, n, 0); }

typedef struct {
  int key;
  uint8_t* value;  // can be anything; `true` is good enough
} CodeLocation;

void copy_and_patch(IrNode *head, CompileContext* cc) {
  CodeLocation *l = NULL;

  // First pass, copy machine code, patch primitives
  for (IrNode *n = head; n != NULL; n = n->next) {
    switch (n->kind) {

    case IR_CALL: {
      IrCall *c = &n->call_node;
      // @TODO - select stencil from args
      CallSignature add_cs = {"add", {ARG_REG, ARG_IMM}};
      Stencil *add_stencil = hmget(cc->stencils, add_cs);
      uint8_t *add_loc = copy_stencil(&cc->mem, add_stencil);

      patch_hole_32(add_loc, add_stencil, 0, stack_index(c->args[0].var.name));
      patch_hole_32(add_loc, add_stencil, 1, c->args[1].integer);
      patch_hole_64(add_loc, add_stencil, 0, (uint64_t)cc->print_result);

      hmput(l, n->label, add_loc);
      break;
    }

    case IR_IF: {
      IrIf *iff = &n->if_node;
      CallSignature if_cs = {"if", {ARG_REG, ARG_REG}};
      Stencil *if_stencil = hmget(cc->stencils, if_cs);
      uint8_t *if_loc = copy_stencil(&cc->mem, if_stencil);

      patch_hole_32(if_loc, if_stencil, 0, stack_index(iff->cond.name));

      hmput(l, n->label, if_loc);
      break;
    }

    case IR_LET: {
      IrLet *let = &n->let_node;
      CallSignature let_cs = {"stack_write", {ARG_IMM}};
      Stencil *let_stencil = hmget(cc->stencils, let_cs);
      uint8_t *let_loc = copy_stencil(&cc->mem, let_stencil);
      hmput(l, n->label, let_loc);

      patch_hole_32(let_loc, let_stencil, 0, let->var.index);
      patch_hole_32(let_loc, let_stencil, 1, let->value.integer);
    } break;

    default: break;
    }
  }

  // Second pass, patch dependent holes
  for (IrNode *n = head; n != NULL; n = n->next) {
    switch (n->kind) {
    case IR_IF: {
      IrIf *iff = &n->if_node;
      uint8_t *if_loc = hmget(l, n->label);

      uint8_t *branch1_loc = hmget(l, iff->then_label);
      uint8_t *branch2_loc = hmget(l, iff->else_label);

      CallSignature if_cs = {"if", {ARG_REG, ARG_REG}};
      Stencil *if_stencil = hmget(cc->stencils, if_cs);
      patch_hole_64(if_loc, if_stencil, 0, (uint64_t)branch1_loc);
      patch_hole_64(if_loc, if_stencil, 1, (uint64_t)branch2_loc);
    } break;

    case IR_LET: {
      IrLet *let = &n->let_node;
      CallSignature let_cs = {"stack_write", {ARG_IMM}};
      Stencil *let_stencil = hmget(cc->stencils, let_cs);
      uint8_t *let_loc = hmget(l, n->label);
      patch_hole_64(let_loc, let_stencil, 0, (uint64_t)hmget(l, n->next->label));
    }

    default: break;
    }
  }

}

