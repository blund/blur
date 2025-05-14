#include "string.h"

#define BL_IMPL
#include "bl.h"

#include "stencil.h"

int main() {
  uint8_t* raw;
  int file_size = read_file("stencils/array.bin", &raw);

  // Read footer values from memory
  uint32_t *footer = (uint32_t *)(raw + file_size - 24);

  // Verify footer
  uint32_t blur_tag = footer[0];
  if (blur_tag != 0x72756C62) {
    fprintf(stderr, "Invalid blur tag!\n");
    free(raw);
    return 1;
  }

  // Read in structure
  stencil_t stencil;
  stencil.code = raw;
  stencil.code_size = footer[1];
  memcpy(&stencil.holes, &footer[2], sizeof(hole_t)*4);


  // Set sentinel values

  int array[3] = {7,8,9};

  // Path array addr (64 bit) and index (32 bit)
  {
    hole_t h = stencil.holes[0];
    uint64_t val = (uint64_t)array;
    if (h.size == hole_32) {
      *(uint32_t*)&(stencil.code[h.index]) = val;
    } else {
      *(uint64_t*)&(stencil.code[h.index]) = val;
    }
  }

  {
    hole_t h = stencil.holes[1];
    uint64_t val = 2;
    if (h.size == hole_32) {
      *(uint32_t*)&(stencil.code[h.index]) = val;
    } else {
      *(uint64_t*)&(stencil.code[h.index]) = val;
    }
  }
  

  puts("bim");
  // Call the modified machine code
  int (*func)() = (int (*)())stencil.code;
  int result = func();
  printf("Result: %d\n", result);

  // Clean up
  munmap(raw, file_size);
  return 0;
}
