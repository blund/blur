#include "stdint.h"

#define max_stencil_holes 8

typedef void (*cps)(uintptr_t);
typedef void (*cps_int)(uintptr_t, int);
typedef void (*cps_int_int)(uintptr_t, int, int);

typedef enum {
  hole_32,
  hole_64,
} HoleSize;

typedef struct {
  HoleSize size;
  uint32_t index;
} Hole;

typedef struct {
  const char* name;
  uint8_t *code;
  uint32_t code_size;
  uint32_t num_holes;
  Hole holes[max_stencil_holes];
} Stencil;
