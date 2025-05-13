#define BL_IMPL
#include "bl.h"

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
