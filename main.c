#include <string.h>
#include <sys/mman.h>

#define BL_STRINGBUILDER_IMPL
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


uint32_t blur_tag = 0x72756C62;
uint32_t code_tag = 0x636f6465;
uint32_t end_tag = 0x656e6421;

// Read in binary stencil file to memory
Stencil read_stencil(char* file_path) {
  char* raw;
  int file_size = read_file(file_path, &raw);
  printf("File Size: %d\n", file_size);

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
  memcpy(&stencil.holes_64, &header[4+max_stencil_holes], sizeof(uint32_t)*max_stencil_holes);

  fori(stencil.num_holes_32) printf("32-bit stencil hole %d at index %d\n", i, stencil.holes_32[i]);
  fori(stencil.num_holes_64)
      printf("64-bit stencil hole %d at index %d\n", i, stencil.holes_64[i]);


  int code_tag_index;
  fori(1000) {
    if (header[i] == code_tag) {
      code_tag_index = i;
      break;
    }
  }

  printf("code tag index: %d\n", code_tag_index);
  assert(header[code_tag_index] == code_tag);

  printf("code_size: %d\n", stencil.code_size);

  stencil.code = (uint8_t*)&header[code_tag_index+1];

  /*
  // printf("%d, %d\n", next_blur_tag, stencil.code_size*8 );

  // stencil.code = (uint8_t*)&header[4+2*max_stencil_holes];
  uint32_t blur_tag2 = header[base + stencil.code_size*8-1];
  if (blur_tag2 != 0x72756C62) {
    fprintf(stderr, "Invalid blur tag!\n");
    free(raw);
    exit(-1); // @TODO - :)
  }
  */


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

void func(uintptr_t, int, int);
int main() {
  Stencil add_stencil = read_stencil("generated/stencils/add_const.bin");
  Stencil if_stencil = read_stencil("generated/stencils/if_test.bin");

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
