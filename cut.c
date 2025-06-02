
#include <generated/stencils.h>

#define BL_IMPL
#define BL_STRINGBUILDER_IMPL
#include <bl.h>

#include <stencil.h>

uint32_t small_holes[4] = {
    small_hole_1,
    small_hole_2,
    small_hole_3,
    small_hole_4,
};

uint64_t big_holes[4] = {
  big_hole_1,
  big_hole_2,
  big_hole_3,
  big_hole_4,
};

int main() {
  dprintf("\n [ Cutting Stencils ]\n");
  build_stencils();

  fori(num_stencils) {
    Stencil* s = &stencils[i];
    dprintf("  -- Stencil '%s': %d 32-bit holes, %d 64-bit-holes\n", s->name,
	   s->num_holes_32,
	   s->num_holes_64);

    s->holes_32 = malloc(sizeof(int)*s->num_holes_32);
    s->holes_64 = malloc(sizeof(int)*s->num_holes_64);

    uint8_t *func_ptr = (uint8_t *)s->code;

    dprintf("    -- Code size: %d\n", s->code_size);

    // Search for our sentinel values
    // Since we read 4 bytes at a time, size-3 is the last 4 byte chunk
    int holes_found_32 = 0;
    forj(s->num_holes_32) {
      for (size_t i = 0; i < s->code_size - 3; ++i) {
        uint32_t *val = (uint32_t *)&func_ptr[i]; // read 4 bytes
        if (*val == small_holes[j]) {
	  dprintf("    -- Found 32 bit hole %d at index %ld\n", j, i);
	  s->holes_32[j] = i;
          holes_found_32++;
	}
      }
    }

    int holes_found_64 = 0;
    forj(s->num_holes_64) {
      for (size_t i = 0; i < s->code_size - 3; ++i) {
        uint64_t *val = (uint64_t *)&func_ptr[i]; // read 4 bytes
        if (*val == big_holes[j]) {
	  dprintf("    -- Found 64 bit hole %d at index %ld\n", j, i);
	  s->holes_64[j] = i;
	  holes_found_64++;
	}
      } 
    }

    // Assert that we have identified a correct amount of holes!
    assert(holes_found_32 == s->num_holes_32);
    assert(holes_found_64 == s->num_holes_64);
    assert(s->num_holes_32 < max_stencil_holes);
    assert(s->num_holes_64 < max_stencil_holes);

    StringBuilder *path_builder = new_builder(1024);
    add_to(path_builder, "generated/stencils/%s.bin", s->name);

    FILE *f = fopen(to_string(path_builder), "wb");

    uint32_t blur_tag = 0x72756C62;
    uint32_t code_tag = 0x636f6465;
    uint32_t end_tag  = 0x656e6421;

    fwrite(&blur_tag, sizeof(uint32_t), 1, f);
    fwrite(&s->code_size, sizeof(uint32_t), 1, f);
    fwrite(&s->num_holes_32, sizeof(uint32_t), 1, f);
    fwrite(&s->num_holes_64, sizeof(uint32_t), 1, f);
    fwrite(s->holes_32, sizeof(int), s->num_holes_32, f);
    fwrite(s->holes_64, sizeof(int), s->num_holes_64, f);
    fwrite(&code_tag, sizeof(uint32_t), 1, f);
    fwrite(func_ptr, 1, s->code_size, f);
    fwrite(&end_tag, sizeof(uint32_t), 1, f);
    fclose(f);

    }
}
