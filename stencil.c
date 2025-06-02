#include <malloc.h>
#include <string.h>

#include <bl.h>

#include <stencil.h>

uint32_t blur_tag = 0x72756C62;
uint32_t code_tag = 0x636f6465;
uint32_t end_tag = 0x656e6421;

// Read in binary stencil file to memory
Stencil* read_stencil(char *file_path) {
  dprintf("  -- Reading stencil at '%s'\n", file_path);
  char* raw;
  int file_size = read_file(file_path, &raw);

  // Read footer values from memory
  uint32_t *header = (uint32_t *)(raw);

  // Verify footer
  uint32_t tag = header[0];
  assert(tag == blur_tag);

  Stencil* stencil = malloc(sizeof(Stencil));
  stencil->code_size = header[1];
  stencil->num_holes_32 = header[2];
  stencil->num_holes_64 = header[3];

  stencil->holes_32 = malloc(sizeof(int)*stencil->num_holes_32);
  stencil->holes_64 = malloc(sizeof(int)*stencil->num_holes_64);

  if (stencil->num_holes_32)
    memcpy(stencil->holes_32, &header[4], sizeof(uint32_t)*stencil->num_holes_32);
  if (stencil->num_holes_64)
    memcpy(stencil->holes_64, &header[4 + stencil->num_holes_32], sizeof(uint32_t)*stencil->num_holes_64);

  dprintf("    -- Code Size: %d\n", stencil->code_size);

  dprintf("    -- 32-bit holes: %d\n", stencil->num_holes_32);
  fori(stencil->num_holes_32) {
    dprintf("      -- 32-bit stencil hole %d at index %d\n", i,
            stencil->holes_32[i]);
  }

  dprintf("    -- 64-bit holes: %d\n", stencil->num_holes_64);
  fori(stencil->num_holes_64)
      dprintf("      -- 64-bit stencil hole %d at index %d\n", i, stencil->holes_64[i]);


  int code_tag_index;
  fori(1000) {
    if (header[i] == code_tag) {
      code_tag_index = i;
      break;
    }
  }

  assert(header[code_tag_index] == code_tag);

  stencil->code = (uint8_t*)&header[code_tag_index+1];

  return stencil;
}
