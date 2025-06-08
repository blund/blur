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

typedef enum {
  OP_INIT = 0,
  ADD_OP,
  SUB_OP,
  MUL_OP,
  DIV_OP,
  OP_END,
} OpCodes;

typedef enum {
  REG_ARG = 0, // Passed in register
  LIT_ARG,     // Passed as literal
  VAR_ARG,     // Pass as variable (on the stack)
  ARG_COUNT,
} ArgumentKind;

#define MAX_STENCILS 1024
#define MAX_HOLES_32 3
#define MAX_HOLES_64 3

// Used for saving the index binary file during
// stencil creation
typedef struct {
  uint8_t opcode;
  uint32_t index;
  uint32_t code_size;
  uint8_t num_holes_32;
  uint8_t num_holes_64;
  uint8_t pass_through_count;
  uint8_t arg1_kind;
  uint8_t arg2_kind;

  // The indices into the code of the hole
  uint8_t holes_32[MAX_HOLES_32];
  uint8_t holes_64[MAX_HOLES_64];
} Stencil;

// Used during stencil creation to store
// all data about stencil
typedef struct {
  char *name;
  uint8_t *code;
  Stencil stencil;
} StencilData;

// Used during compilation as 'key' into
// the hash map with stencil data
typedef struct {
  uint8_t opcode;
  uint8_t arg1_kind;
  uint8_t arg2_kind;
  uint8_t pass_through_count;
} StencilKey;

// Used during compilation, is the 'value'
// of the hash map of stencils
typedef struct {
  uint16_t index;
  uint16_t code_size;
  uint8_t num_holes_32;
  uint8_t num_holes_64;
  uint8_t holes_32[MAX_HOLES_32];
  uint8_t holes_64[MAX_HOLES_64];
} StencilVal;

typedef struct {
  StencilKey key;
  StencilVal value;
} StencilMap;

Stencil* read_stencil(char *file_path);

#endif
