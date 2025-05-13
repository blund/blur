#include <stdio.h>
#include <stdint.h>

#define BL_IMPL
#include "bl.h"

/*
  We want to have a split:
  - Compile time execution
  - Runtime compilation/optimization
  - Storing runtime compilations (real profile guided)
  - We compile to bytecode at compile time, and allow modifying it
  - What's hard is having semantics that are useful to optimmize at runtime
  - Metaprogamming is what is useful. Lisp like


  - What is most important is to be able to introspect the language, both at runtime and compile time. We have memory. We have fast CPU's. Let's make use of them in the runtime.
 */


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/mman.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

typedef struct {
  uint8_t *code;
  uint32_t code_size;
  uint32_t a_offset;
  uint32_t b_offset;
} add_data;

int main() {
  uint8_t* raw;
  int file_size = read_file("add.bin", &raw);

  // Read footer values from memory
  uint32_t *footer = (uint32_t *)(raw + file_size - 16);

  // Verify footer
  uint32_t blur_tag = footer[0];
  if (blur_tag != 0x72756C62) {
    fprintf(stderr, "Invalid blur tag!\n");
    free(raw);
    return 1;
  }

  // Read in structure
  add_data adder;
  adder.code = raw;
  adder.code_size = footer[1];
  adder.a_offset  = footer[2];
  adder.b_offset  = footer[3];

  // Set sentinel values
  *(uint32_t*)&(adder.code[adder.a_offset]) = 1;
  *(uint32_t*)&(adder.code[adder.b_offset]) = 2;

  // Call the modified machine code
  int (*func)() = (int (*)())adder.code;
  int result = func();
  printf("Result: %d\n", result);

  // Clean up
  munmap(raw, file_size);
  return 0;
}
