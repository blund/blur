#include "stdint.h"

#define max_stencil_holes 8

typedef struct {
  const char* name;
  uint8_t *code;
  uint32_t code_size;
  uint32_t num_holes;
  uint32_t holes[max_stencil_holes];
} stencil_t;

