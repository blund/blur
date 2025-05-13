#define BL_IMPL
#include "bl.h"

#include "stencil.h"

typedef struct {
  uint8_t *code;
  uint32_t code_size;
  uint32_t a_offset;
  uint32_t b_offset;
} add_data;

int main() {
  uint8_t* raw;
  int file_size = read_file("stencils/mul.bin", &raw);

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
  stencil_t stencil;
  stencil.code = raw;
  stencil.code_size = footer[1];
  stencil.holes[0]  = footer[2];
  stencil.holes[1]  = footer[3];

  int arr[3] = {7,8,9};
  // Set sentinel values
  *(uint32_t*)&(stencil.code[stencil.holes[0]]) = 3;
  *(uint32_t*)&(stencil.code[stencil.holes[1]]) = 2;

  // Call the modified machine code
  int (*func)() = (int (*)())stencil.code;
  int result = func();
  printf("Result: %d\n", result);

  // Clean up
  munmap(raw, file_size);
  return 0;
}
