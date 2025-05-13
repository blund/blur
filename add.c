#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define BL_IMPL
#include "bl.h"

// Your target function
int add() {
    int a = 0xfffffff0;
    int b = 0xfffffff1;
    int c = a + b;
    return c;
}
void end_of_add() {}

int main() {
    uint8_t *start = (uint8_t *)add;
    uint8_t *end = (uint8_t *)end_of_add;
    size_t size = end - start;

    // Cast the function pointer to a byte pointer
    uint8_t *func_ptr = (uint8_t *)add;

    int a_offset, b_offset;

    // Search for 0xfffffff0 as a 32-bit value
    for (size_t i = 0; i < size - 3; ++i) {
      uint32_t *val = (uint32_t *)&func_ptr[i]; // read 4 bytes
      if (*val == 0xfffffff0) {
	assert(i == 0xc);
        printf("Found sentinel 0xfffffff0 at offset 0x%zx\n", i);
	a_offset = i;
      } else if (*val == 0xfffffff1) {
	printf("Found sentinel 0xfffffff1 at offset 0x%zx\n", i);
	assert(i == 0x14);
	b_offset = i;
      }
    }

    // Optionally, write to file
    FILE *f = fopen("add.bin", "wb");
    fwrite(func_ptr, 1, size, f);

    uint32_t blur_tag = 0x72756C62;
    fwrite(&blur_tag, sizeof(uint32_t), 1, f);
    fwrite(&size, sizeof(uint32_t), 1, f);
    fwrite(&a_offset, sizeof(uint32_t), 1, f);
    fwrite(&b_offset, sizeof(uint32_t), 1, f);

    fclose(f);

    return 0;
}
