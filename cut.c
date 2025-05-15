
#include "generated/stencils.h"

#define BL_IMPL
#define BL_STRINGBUILDER_IMPL
#include "bl.h"

int main() {
  build_stencils();

  fori(num_stencils) {
    Stencil* s = &stencils[i];
    printf("stencil: %s\n", s->name);
    printf("holes: %d\n", s->num_holes);

    assert(s->num_holes < max_stencil_holes);
    
    uint8_t *func_ptr = (uint8_t *)s->code;

    // Search for our sentinel values
    // Since we read 4 bytes at a time, size-3 is the last 4 byte chunk

    // @TODO - error handling here
    for (size_t i = 0; i < s->code_size - 3; ++i) { 
      uint32_t *val = (uint32_t *)&func_ptr[i]; // read 4 bytes
      if (*val == 0xfffffff0) {
        printf("Found sentinel 0xfffffff0 at offset 0x%zx\n", i);
	s->holes[0].index = i;
        if (*(uint64_t *)val == 0xfffffffffffffff0) s->holes[0].size = hole_64;
      } else if (*val == 0xfffffff1) {
	printf("Found sentinel 0xfffffff1 at offset 0x%zx\n", i);
        s->holes[1].index = i;
	s->holes[1].size = hole_32;
        if (*(uint64_t *)val == 0xfffffffffffffff1) s->holes[1].size = hole_64;
      }
    }

    StringBuilder *path_builder = new_builder(1024);
    add_to(path_builder, "generated/stencils/%s.bin", s->name);

    FILE *f = fopen(to_string(path_builder), "wb");
    fwrite(func_ptr, 1, s->code_size, f);

    uint32_t blur_tag = 0x72756C62;
    fwrite(&blur_tag, sizeof(uint32_t), 1, f);
    fwrite(&s->code_size, sizeof(uint32_t), 1, f);
    fwrite(&s->num_holes, sizeof(uint32_t), 1, f);
    fwrite(&s->holes, sizeof(Hole), 8, f);

    fclose(f);
  }
}
