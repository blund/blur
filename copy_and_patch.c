
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>


#include <include/stb_ds.h>
#include <bl.h>

#include <ir/ir.h>
#include <ir/transform.h>

#include <copy_and_patch.h>

StencilMap *read_stencil_map(char *index_file_path);
uint8_t *map_code_blob(const char *filename, size_t *out_size);
CopyPatchContext make_context(char *index_path, char *code_blob_path) {
  CopyPatchContext ctx;
  ctx.mem = make_executable_memory();

  size_t code_size;
  ctx.code_blob = map_code_blob(code_blob_path, &code_size);
  // @TODO - error handling

  ctx.stencil_map = read_stencil_map(index_path);

  return ctx;
}


StencilVal get_stencil(StencilKey key, CopyPatchContext* ctx) {
  return hmget(ctx->stencil_map, key);
}

void patch_hole_32(uint8_t *code, StencilKey sk, int index, uint32_t value, CopyPatchContext* ctx) {
  StencilVal sv = get_stencil(sk, ctx);
  int code_index = sv.holes_32[index];
  *(uint32_t*)&(code[code_index]) = value;
}

void patch_hole_64(uint8_t *code, StencilKey sk, int index, uint64_t value, CopyPatchContext* ctx) {
  StencilVal sv = get_stencil(sk, ctx);
  int code_index = sv.holes_64[index];
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

uint8_t *copy_stencil(StencilKey sk, CopyPatchContext *ctx) {
  StencilVal stencil = hmget(ctx->stencil_map, sk);
  memcpy(&ctx->mem.code[ctx->mem.write_head], &ctx->code_blob[stencil.index], stencil.code_size);
  uint8_t* location = &ctx->mem.code[ctx->mem.write_head];
  ctx->mem.write_head += stencil.code_size;
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

// @TODO 04.06.25 - If you are feeling especially inspired, take a look at
// removing jmps when n->cont == n->label+1, i.e. when we want to jump to
// the code directly after this code. In this case, the jmp does nothing.
// A problem though is ensuring that we do this properly, since x86-64 is
// crazy. It seems this is always a mov (48b8 ...) and jmp (ffe0), but we
// would like to manuall detect this operation (through the tailcall stencil)
// and "cut" those bytes off the end. This though depends on the functions
// not being aligned and having one of the many nops of the architeture,
// which we currently get with zig cc.


void copy_and_patch2(IrNode *head, CopyPatchContext* ctx) {
  CodeLocation *l = NULL;

  // First pass, copy machine code, patch primitives
  for (IrNode *n = head; n != NULL; n = n->next) {
    switch (n->kind) {

    case IR_CALL: {
      IrCall *c = &n->call_node;
      // @TODO - select stencil from args
      StencilKey sk = {0,1,0};
      uint8_t *add_loc = copy_stencil(sk, ctx);
      patch_hole_32(add_loc, sk, 0, c->args[0].var.index*4, ctx);
      patch_hole_32(add_loc, sk, 1, c->args[1].integer, ctx);

      hmput(l, n->label, add_loc);
      break;
    }

    case IR_IF: {
      /*
      IrIf *iff = &n->if_node;
      CallSignature if_cs = {"if", {ARG_REG, ARG_REG}};
      Stencil *if_stencil = hmget(cc->stencils, if_cs);
      uint8_t *if_loc = copy_stencil(&cc->mem, if_stencil);

      patch_hole_32(if_loc, if_stencil, 0, stack_index(iff->cond.name));

      hmput(l, n->label, if_loc);
      */
      break;
    }

    case IR_LET: {
      /*
      IrLet *let = &n->let_node;
      CallSignature let_cs = {"stack_write", {ARG_IMM}};
      Stencil *let_stencil = hmget(cc->stencils, let_cs);
      uint8_t *let_loc = copy_stencil(&cc->mem, let_stencil);
      hmput(l, n->label, let_loc);

      patch_hole_32(let_loc, let_stencil, 0, let->var.index*4);
      patch_hole_32(let_loc, let_stencil, 1, let->value.integer);
      */
    } break;

    default: break;
    }
  }

  // @TODO 02.06.25 - for the general case, we can but the 'cont' label on the IrNode,
  // so that only the Nodes that don't have a single 'cont' can use the same
  // default case in the switch, while special nodes do. I think this
  // generalizes, I'm not sure for now, so I am leaving this.

  // Second pass, patch dependent holes
  for (IrNode *n = head; n != NULL; n = n->next) {
    switch (n->kind) {
    case IR_IF: {
      /*
      IrIf *iff = &n->if_node;
      uint8_t *if_loc = hmget(l, n->label);

      uint8_t *then_loc = hmget(l, iff->then_label);
      uint8_t *else_loc = hmget(l, iff->else_label);

      CallSignature if_cs = {"if", {ARG_REG, ARG_REG}};
      Stencil *if_stencil = hmget(cc->stencils, if_cs);
      patch_hole_64(if_loc, if_stencil, 0, (uint64_t)then_loc);
      patch_hole_64(if_loc, if_stencil, 1, (uint64_t)else_loc);
      */
    } break;

    case IR_LET: {
      /*
      IrLet *let = &n->let_node;
      CallSignature let_cs = {"stack_write", {ARG_IMM}};
      Stencil *let_stencil = hmget(cc->stencils, let_cs);
      uint8_t *let_loc = hmget(l, n->label);
      patch_hole_64(let_loc, let_stencil, 0, (uint64_t)hmget(l, let->cont));
      */
    } break;

    case IR_CALL: {
      IrCall *c = &n->call_node;
      // @TODO - select stencil from args
      CallSignature add_cs = {"add", {ARG_REG, ARG_IMM}};
      StencilKey sk = {0, 1, 0};

      uint8_t *add_loc = hmget(l, n->label);

      patch_hole_64(add_loc, sk, 0, (uint64_t)ctx->final, ctx);
      if (c->cont == 0)
        patch_hole_64(add_loc, sk, 0, (uint64_t)ctx->final, ctx);
      else
        patch_hole_64(add_loc, sk, 0, (uint64_t)hmget(l, c->cont), ctx);

      break;
    }

    default: break;
    }
  }
}


void copy_and_patch(IrNode *head, CompileContext *cc) {
  /*
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
      patch_hole_32(add_loc, add_stencil, 0, c->args[0].var.index*4);
      patch_hole_32(add_loc, add_stencil, 1, c->args[1].integer);

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

      patch_hole_32(let_loc, let_stencil, 0, let->var.index*4);
      patch_hole_32(let_loc, let_stencil, 1, let->value.integer);
    } break;

    default: break;
    }
  }

  // @TODO 02.06.25 - for the general case, we can but the 'cont' label on the
IrNode,
  // so that only the Nodes that don't have a single 'cont' can use the same
  // default case in the switch, while special nodes do. I think this
  // generalizes, I'm not sure for now, so I am leaving this.

  // Second pass, patch dependent holes
  for (IrNode *n = head; n != NULL; n = n->next) {
    switch (n->kind) {
    case IR_IF: {
      IrIf *iff = &n->if_node;
      uint8_t *if_loc = hmget(l, n->label);

      uint8_t *then_loc = hmget(l, iff->then_label);
      uint8_t *else_loc = hmget(l, iff->else_label);

      CallSignature if_cs = {"if", {ARG_REG, ARG_REG}};
      Stencil *if_stencil = hmget(cc->stencils, if_cs);
      patch_hole_64(if_loc, if_stencil, 0, (uint64_t)then_loc);
      patch_hole_64(if_loc, if_stencil, 1, (uint64_t)else_loc);
    } break;

    case IR_LET: {
      IrLet *let = &n->let_node;
      CallSignature let_cs = {"stack_write", {ARG_IMM}};
      Stencil *let_stencil = hmget(cc->stencils, let_cs);
      uint8_t *let_loc = hmget(l, n->label);
      patch_hole_64(let_loc, let_stencil, 0, (uint64_t)hmget(l, let->cont));
    } break;

    case IR_CALL: {
      IrCall *c = &n->call_node;
      // @TODO - select stencil from args
      CallSignature add_cs = {"add", {ARG_REG, ARG_IMM}};
      Stencil *add_stencil = hmget(cc->stencils, add_cs);
      uint8_t *add_loc = hmget(l, n->label);

      patch_hole_64(add_loc, add_stencil, 0, (uint64_t)cc->final);
      if (c->cont == 0)
        patch_hole_64(add_loc, add_stencil, 0, (uint64_t)cc->final);
      else
        patch_hole_64(add_loc, add_stencil, 0, (uint64_t)hmget(l, c->cont));

      break;
    }


    default: break;
    }
  }

  */
}


StencilMap *read_stencil_map(char *index_file_path) {
  FILE *index_file = fopen(index_file_path, "rb");

  StencilMap *stencil_map = NULL;  // hash map: key → value
  
  Stencil s;
  while (fread(&s, sizeof(Stencil), 1, index_file) == 1) {
    StencilKey key = {
        .opcode = s.opcode,
        .num_registers = s.num_registers,
        .pass_through_count = s.pass_through_count,
    };
    StencilVal val = {
        .index = s.index,
        .code_size = s.code_size,
        .num_holes_32 = s.num_holes_32,
        .num_holes_64 = s.num_holes_64,
    };
    memcpy(val.holes_32, s.holes_32, s.num_holes_32);
    memcpy(val.holes_64, s.holes_64, s.num_holes_64);

    hmput(stencil_map, key, val);
  }

  fclose(index_file);

  return stencil_map;
}

// Load and map code blob
uint8_t *map_code_blob(const char *filename, size_t *out_size) {
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("open");
        return NULL;
    }

    off_t size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

    uint8_t *mem = mmap(NULL, size, PROT_READ | PROT_EXEC, MAP_PRIVATE, fd, 0);
    if (mem == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return NULL;
    }

    close(fd);
    if (out_size) *out_size = size;
    return mem;
}
