#ifndef STENCIL_H
#define STENCIL_H

#include "stdint.h"

#define max_stencil_holes (4)

#define small_hole_1 0x3e7a91bc  
#define small_hole_2 0xd48cf2a0  
#define small_hole_3 0x9b5d6ee3  
#define small_hole_4 0x71c3ab4f

#define big_hole_1 0xe2d9c7b1843a56f0  
#define big_hole_2 0x4ba1fedc02347a9d  
#define big_hole_3 0xac5f13e7902dcb88  
#define big_hole_4 0x1dce8b4793fa065e

// Stringify macro
#define _STR(x) #x
#define STR(x) _STR(x)

typedef struct {
  const char* name;
  uint8_t *code;
  uint32_t code_size;
  uint32_t num_holes_32;
  uint32_t num_holes_64;

  // The indices into the code of the hole
  int holes_32[4];
  int holes_64[4];
} Stencil;

Stencil read_stencil(char *file_path);

#endif
