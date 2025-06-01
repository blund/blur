#include <string.h>
#include <sys/mman.h>

#define BL_STRINGBUILDER_IMPL
#define BL_IMPL
#include "bl.h"

#define STB_DS_ASSERT assert
#define STB_DS_IMPLEMENTATION
#include "include/stb_ds.h"


#include "ast/ast.h"
#include "ast/build.h"
#include "ast/traverse.h"

#include "stencil.h"

typedef struct {
  uint8_t *code;
  int write_head;
  int capacity;
} ExecutableMemory;

// Functions used for testing cps functionlaity
void print_result(uintptr_t stack, int result) {
  printf("From continuation passing: %d\n", result);
}
void choice_a(uintptr_t stack) { puts("a!"); }
void choice_b(uintptr_t stack) { puts("b!"); }


uint32_t blur_tag = 0x72756C62;
uint32_t code_tag = 0x636f6465;
uint32_t end_tag = 0x656e6421;

// Read in binary stencil file to memory
Stencil read_stencil(char *file_path) {
  dprintf("  -- Reading stencil at '%s'\n", file_path);
  char* raw;
  int file_size = read_file(file_path, &raw);

  // Read footer values from memory
  uint32_t *header = (uint32_t *)(raw);

  // Verify footer
  uint32_t tag = header[0];
  assert(tag == blur_tag);

  Stencil stencil;
  stencil.code_size = header[1];
  stencil.num_holes_32 = header[2];
  stencil.num_holes_64 = header[3];
  memcpy(&stencil.holes_32, &header[4], sizeof(uint32_t)*max_stencil_holes);
  memcpy(&stencil.holes_64, &header[4 + max_stencil_holes], sizeof(uint32_t) * max_stencil_holes);

  dprintf("    -- Code Size: %d\n", stencil.code_size);

  dprintf("    -- 32-bit holes: %d\n", stencil.num_holes_32);
  fori(stencil.num_holes_32)
    dprintf("      -- 32-bit stencil hole %d at index %d\n", i, stencil.holes_32[i]);

  dprintf("    -- 64-bit holes: %d\n", stencil.num_holes_64);
  fori(stencil.num_holes_64)
      dprintf("      -- 64-bit stencil hole %d at index %d\n", i, stencil.holes_64[i]);


  int code_tag_index;
  fori(1000) {
    if (header[i] == code_tag) {
      code_tag_index = i;
      break;
    }
  }

  assert(header[code_tag_index] == code_tag);

  stencil.code = (uint8_t*)&header[code_tag_index+1];

  return stencil;
}

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

UsedVarSet *clone_set(UsedVarSet *src) {
  UsedVarSet *copy = NULL;
  for (int i = 0; i < hmlen(src); ++i) {
    hmput(copy, src[i].key, 1);
  }
  return copy;
}

void push_scope(UsedVarSet ***stack) {
  UsedVarSet *empty = NULL;
  arrput(*stack, empty);
}

UsedVarSet *pop_scope(UsedVarSet ***stack) {
  return arrpop(*stack);
}

void collect_used_vars(NodeType type, void *node, TraverseCtx *ctx, TraversalType traversal) {
  UsedVarSet ***set_stack = ctx->data;
  UsedVarSet **set = &arrlast(*set_stack);

  if (traversal == pre_order) {
    switch (type) {
    case block_node: {
      Block *b = node;
      push_scope(set_stack);
    } break;

    default: break;
    }
    return;
  }

  
  if (traversal == post_order) {
    switch (type) {
    case block_node: {
      Block *b = node;
      UsedVarSet *popped = arrpop(*set_stack); // pop

      // Merge children’s used_vars
      for (int i = 0; i < b->count; ++i) {
	Statement *stmt = b->statements[i];

	// If the statement contains a nested block (e.g., if-statement),
	// you'll want to look inside that and merge its used_vars.
        if (stmt->kind == if_statement) {
	  If *ifstmt = &stmt->if_block;
          if (ifstmt->then_branch) {
            Block *branch = ifstmt->then_branch;
	    merge_sets(&popped, ifstmt->then_branch->used_vars);
	  }
	  if (ifstmt->else_branch) {
	    Block *branch = ifstmt->else_branch;
	    merge_sets(&popped, ifstmt->else_branch->used_vars);
	  }
	}
      }

      b->used_vars = clone_set(popped);
    } break;

    case literal_node: {
      Literal *lit = node;
      if (lit->kind == identifier_lit) {
	hmput(*set, lit->identifier, 1);
	//printf("  literal '%s'\n", lit->identifier);
      }
    } break;

    case assign_node: {
      Assign *a = node;
      //printf("  assign '%s'\n", a->name);
      //if (!hmget(*set, a->name)) {
	//printf("DEAD: %s is never used\n", a->name);
      //}
    } break;

    default:
      break;
    }
  }
}

void indent_(int depth) {
  for (int i = 0; i < depth; ++i) {
    printf("  "); // 2 spaces per level
  }
}

void print_nodes(NodeType type, void *node, TraverseCtx *ctx, TraversalType traversal) {
  int* depth = &ctx->data;

  if (traversal == post_order) {
    if (type == block_node) {
      *depth -= 1;
    }
    if (type == assign_node) {
      *depth -= 1;
    }
    if (type == if_node) {
      *depth -= 1;
    }
  }

  if (traversal == pre_order) {
  switch (type) {
  case literal_node: {
    Literal *lit = node;
    indent_(*depth);

    switch(lit->kind) {
    case identifier_lit:
      printf("(Literal ident %s)\n", lit->identifier);
      break;

    case integer_lit:
      printf("(Literal int %d)\n", lit->integer);
      break;


    default: break;
    }
  } break;

  case assign_node: {
    indent_(*depth);
    *depth += 1;
    Assign *a = node;
    printf("(Assign)\n");
  } break;

  case call_node: {
    indent_(*depth);
    printf("Call\n");
    Call *c = node;
  } break;

  case statement_node: {
  } break;

  case block_node: {
    indent_(*depth);
    *depth += 1;
    Block *b = node;
    printf("(Block %p ", b);
    if (b->count == 0) break;
    printf(" [used vars :");
    for (int i = 0; i < hmlen(b->used_vars); ++i) {
      printf(" %s,", b->used_vars[i].key);
    }
    printf("])\n");
	   
  } break;

  case if_node: {
    indent_(*depth);
    *depth += 1;
    printf("(If block)\n");
  } break;

  case expression_node: {
  } break;

  default:
    break;
  }
  }
}


Block* example_ast();
int main() {
  dprintf(" Running copy-patch compiler...\n");
  Stencil add_stencil = read_stencil("generated/stencils/add_const.bin");
  Stencil if_stencil = read_stencil("generated/stencils/if_test.bin");

  Block *b = example_ast();

  print_block(b);

  // Traverse block to gather aliveness
  TraverseCtx ctx;



  UsedVarSet **set_stack = NULL;
  UsedVarSet *initial_set = NULL;
  arrput(set_stack, initial_set);
  
  ctx = (TraverseCtx){.traversal=post_order, .data = &set_stack};
  traverse_block(b, collect_used_vars, &ctx);

  puts("Printing ast:");
 
  ctx = (TraverseCtx){.traversal= pre_order, .data = 0};
  traverse_block(b, print_nodes, &ctx);
  
  ExecutableMemory em = make_executable_memory();

  uint8_t *if_loc   = copy_stencil(&em, &if_stencil);
  uint8_t* add1_loc = copy_stencil(&em, &add_stencil);
  uint8_t* add2_loc = copy_stencil(&em, &add_stencil);

  // Patch the holes!
  patch_hole_32(add1_loc, &add_stencil, 0, 4);
  patch_hole_64(add1_loc, &add_stencil, 0, (uint64_t)print_result);

  patch_hole_32(add2_loc, &add_stencil, 0, 30);
  patch_hole_64(add2_loc, &add_stencil, 0, (uint64_t)print_result);

  patch_hole_64(if_loc, &if_stencil, 0, (uint64_t)add1_loc);
  patch_hole_64(if_loc, &if_stencil, 1, (uint64_t)add2_loc);

  // Initialize our stack (unused)
  int stack_[1024];
  uintptr_t stack = (uintptr_t)&stack;

  // Call the modified machine code
  void (*func)(uintptr_t, int, int) = (void (*)(uintptr_t, int, int))em.code;
  func(stack, 0, 2);

  // Clean up
  //free(stencil.code);
  munmap(em.code, em.capacity);

  return 0;
}

Block* example_ast() {
  Block *b = new_block(
	     new_assign("test", new_integer(3)),
             new_if(new_integer(1),
                    new_block(new_call(
                        "add", (Arguments){.count = 2,
                                           .entries = {new_identifier("test"),
                                                       new_integer(4)}})),
                    new_block(new_call(
                        "add", (Arguments){.count = 2,
                                           .entries = {new_identifier("test2"),
                                                       new_integer(7)}}))));
  return b;
}
