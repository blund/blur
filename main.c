#include "string.h"

#define BL_IMPL
#include "bl.h"

#include "stencil.h"

// Functions used for testing cps functionlaity
void print_result(uintptr_t stack, int result) {
  printf("From continuation passing: %d\n", result);
}
void choice_a(uintptr_t stack) { puts("a!"); }
void choice_b(uintptr_t stack) { puts("b!"); }

// Used to patch a hole in the stencil.
void patch_hole(Stencil* s, int index, uint64_t value) {
  {
    Hole h = s->holes[index];
    if (h.size == hole_32) {
      *(uint32_t*)&(s->code[h.index]) = value;
    } else {
      *(uint64_t*)&(s->code[h.index]) = value;
    }
  }
}

typedef struct {
  void *memory;
  int write_index;
  int capacity;
} ExecutableMemory;

ExecutableMemory make_executable_memory() {
  // Allocate a buffer with RWX permissions (on Linux)
  void *memory = mmap(NULL, 1024, PROT_READ | PROT_WRITE | PROT_EXEC,
                      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

  ExecutableMemory m = {(void*)-1, -1, -1};
  if (memory == MAP_FAILED) {
    perror("mmap");
    return m;
  }

  m.memory = memory;
  m.write_index = 0;
  m.capacity = 1024;

  return m;
}

int main() {
  uint8_t* raw;
  int file_size = read_file("generated/stencils/if_test.bin", &raw);

  // Read footer values from memory
  uint32_t *footer = (uint32_t *)(raw + file_size - (8*sizeof(Hole) + 4 + 4 + 4));

  // Verify footer
  uint32_t blur_tag = footer[0];
  if (blur_tag != 0x72756C62) {
    fprintf(stderr, "Invalid blur tag!\n");
    free(raw);
    return 1;
  }

  // Read in structure
  Stencil stencil;
  stencil.code = raw;
  stencil.code_size = footer[1];
  stencil.num_holes = footer[2];
  memcpy(&stencil.holes, &footer[3], sizeof(Hole)*4);

  // Patch the holes!
  uint64_t val1 = (uint64_t)choice_a;
  patch_hole(&stencil, 0, val1);
  uint64_t val2 = (uint64_t)choice_b;
  patch_hole(&stencil, 1, val2);

  ExecutableMemory em = make_executable_memory();
  
  // Initialize our stack (unused)
  int stack_[1024];
  uintptr_t stack = (uintptr_t)&stack;

  // Call the modified machine code
  cps_int func = (cps_int)stencil.code;
  func(stack, 0);

  // Clean up
  munmap(raw, file_size);
  return 0;
}
