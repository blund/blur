

#include <stdio.h>

#define STB_DS_IMPLEMENTATION
#include <include/stb_ds.h>

#define BL_STRINGBUILDER_IMPL
#define BL_IMPL
#include <bl.h>

#include "../generated/stencils.h"

uint32_t small_holes[4] = {
    small_hole_1,
    small_hole_2,
    small_hole_3,
    small_hole_4,
};

uint32_t small_holes_inv[4] = {
    ~small_hole_1+1,
    ~small_hole_2+1,
    ~small_hole_3+1,
    ~small_hole_4+1,
};


uint64_t big_holes[4] = {
  big_hole_1,
  big_hole_2,
  big_hole_3,
  big_hole_4,
};


int main() {

  build_stencils();

  // Check all stencils and save hole locations
  fori(num_stencils) {
    StencilData* s = &stencils[i];
    // printf("Stencil %s code size: %d\n", s->name, s->stencil.code_size);

    uint8_t *func_ptr = (uint8_t *)s->code;

    int holes_found_32 = 0;
    forj(s->stencil.num_holes_32) {
      for (size_t i = 0; i < s->stencil.code_size - 3; ++i) {
        uint32_t *val = (uint32_t *)&func_ptr[i]; // read 4 bytes
        if (*val == small_holes_inv[j]) {
          dprintf("    -- Found 32 bit INVERSE hole %d at index %ld\n", j, i);
          // @NOTE - we mark the high bit here, to signify that the value
          // written to this memory should be flipped by two's complement.
          // This happens because the compiler will sometimes flip our
	  // value, in the case of a lea replacement for an add for example.
	  s->stencil.holes_32[j] = i |= 0x80;
          holes_found_32++;
        } else if ((*val)+1 == small_holes[j]) {
          dprintf("    -- Found 32 bit INCREMENTED hole %d at index %ld\n", j, i);
	  s->stencil.holes_32[j] = i |= 0x40;
          holes_found_32++;
        } else if (*(val-1) == small_holes[j]+1) {
          dprintf("    -- Found 32 bit DECREMENTED hole %d at index %ld\n", j, i);
	  s->stencil.holes_32[j] = i |= 0x20;
          holes_found_32++;
	} else if (*val == small_holes[j]) {
	  dprintf("    -- Found 32 bit hole %d at index %ld\n", j, i);
	  s->stencil.holes_32[j] = i;
          holes_found_32++;
	}
      }
    }

    int holes_found_64 = 0;
    forj(s->stencil.num_holes_64) {
      for (size_t i = 0; i < s->stencil.code_size - 3; ++i) {
        uint64_t *val = (uint64_t *)&func_ptr[i]; // read 4 bytes
        if (*val == big_holes[j]) {
	  dprintf("    -- Found 64 bit hole %d at index %ld\n", j, i);
	  s->stencil.holes_64[j] = (uint8_t)i;
	  holes_found_64++;
	}
      } 
    }

    // Assert that we have identified a correct amount of holes!
    assert(holes_found_32 == s->stencil.num_holes_32);
    assert(holes_found_64 == s->stencil.num_holes_64);
    assert(s->stencil.num_holes_32 < max_stencil_holes);
    assert(s->stencil.num_holes_64 < max_stencil_holes);
  }

  // Write out all stencil binaries
  FILE *code_file = fopen("../generated/code_blob.bin", "wb");
  int index = 0;
  fori(num_stencils) {
    StencilData *s = &stencils[i];
    s->stencil.index = index;

    //printf("%d %d %d %d\n", s->stencil.opcode, s->stencil.arg1_kind, s->stencil.arg2_kind, s->stencil.pass_through_count);
    //printf(" index: %d code_size: %d\n", index, s->stencil.code_size);
    fwrite(s->code, s->stencil.code_size, 1, code_file);
    index += s->stencil.code_size;
  }
  fclose(code_file);

  // Write out all the keys for codes and indicies into the code_blob
  FILE *index_file = fopen("../generated/index.bin", "wb");
  fori(num_stencils) {
    StencilData *s = &stencils[i];
    fwrite(&s->stencil, sizeof(Stencil), 1, index_file);
  }
  fclose(index_file);
}
