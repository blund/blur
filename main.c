#include <string.h>
#include <sys/mman.h>

#define BL_IMPL
#include "bl.h"

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

// Read in binary stencil file to memory
Stencil read_stencil(char* file_path) {
  char* raw;
  int file_size = read_file(file_path, &raw);

  // Read footer values from memory
  uint32_t *footer = (uint32_t *)(raw + file_size - (8*sizeof(Hole) + 4 + 4 + 4));

  // Verify footer
  uint32_t blur_tag = footer[0];
  if (blur_tag != 0x72756C62) {
    fprintf(stderr, "Invalid blur tag!\n");
    free(raw);
    exit(-1); // @TODO - :)
  }

  // Read in structure
  Stencil stencil;
  stencil.code = (uint8_t*)raw;
  stencil.code_size = footer[1];
  stencil.num_holes = footer[2];
  memcpy(&stencil.holes, &footer[3], sizeof(Hole) * 4);
  return stencil;
}

// Patch a hole in a stencil
void patch_hole(uint8_t* code, Stencil* s, int index, uint64_t value) {
  {
    Hole h = s->holes[index];
    if (h.size == hole_32) {
      *(uint32_t*)&(code[h.index]) = value;
    } else {
      *(uint64_t*)&(code[h.index]) = value;
    }
  }
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


int main() {
  Stencil if_stencil = read_stencil("generated/stencils/if_test.bin");
  Stencil add_stencil = read_stencil("generated/stencils/add_const.bin");

  ExecutableMemory em = make_executable_memory();
  uint8_t *if_loc   = copy_stencil(&em, &if_stencil);
  uint8_t* add1_loc = copy_stencil(&em, &add_stencil);
  uint8_t* add2_loc = copy_stencil(&em, &add_stencil);

  // Patch the holes!
  patch_hole(add1_loc, &add_stencil, 0, 4);
  patch_hole(add1_loc, &add_stencil, 1, (uint64_t)print_result);

  patch_hole(add2_loc, &add_stencil, 0, 30);
  patch_hole(add2_loc, &add_stencil, 1, (uint64_t)print_result);

  patch_hole(if_loc, &if_stencil, 0, (uint64_t)add1_loc);
  patch_hole(if_loc, &if_stencil, 1, (uint64_t)add2_loc);
  
  // Initialize our stack (unused)
  int stack_[1024];
  uintptr_t stack = (uintptr_t)&stack;

  // Call the modified machine code
  cps_int func = (cps_int)em.code;
  func(stack, 1);

  // Clean up
  // free(stencil.code);
  munmap(em.code, em.capacity);

  return 0;
}
