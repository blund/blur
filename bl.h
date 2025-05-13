#ifndef BL_H
#define BL_H

#include <stdint.h>
#include <stdarg.h>

typedef struct string_builder {
  char* buffer;
  int   capacity;
  int   index;
} string_builder;

// Forward declare functions
string_builder* new_builder(int capacity);
void add_to(string_builder* b, char* format, ...);
void reset(string_builder* b);
void free_builder(string_builder* b);
char* to_string(string_builder* b);


int read_file(const char* filename, uint8_t** data);
void _release_assert(const char *assertionExpr, const char *assertionFile,
                    unsigned int assertionLine, const char *assertionFunction);
float random_float(float min, float max);

#define fori(x) for (int i = 0; i < x; i++)

#ifdef BL_IMPL

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

void _release_assert(const char *assertionExpr,
                    const char *assertionFile,
                    unsigned int assertionLine,
                    const char* assertionFunction) {
  fprintf(stderr, "%s:%u: %s: Assertion `%s' failed.\n", assertionFile, assertionLine, assertionFunction, assertionExpr);
  abort();
}

#define bl_assert(expr)							\
  ((expr) ? ((void)0)							\
   : _release_assert(#expr, __FILE__, __LINE__, __extension__ __PRETTY_FUNCTION__))
#define assert bl_assert

int read_file(const char *filename, uint8_t **out) {
    // Open the file containing the machine code
    FILE *f = fopen(filename, "rb");
    if (!f) {
        perror("fopen");
        return 1;
    }

    // Get file size
    fseek(f, 0, SEEK_END);
    size_t file_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    // Allocate a buffer with RWX permissions (on Linux)
    void *raw = mmap(NULL, file_size, PROT_READ | PROT_WRITE | PROT_EXEC,
                     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (raw == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    // Read code into memory
    fread(raw, 1, file_size, f);
    fclose(f);

    *out = raw;

    return file_size;
}

float random_float(float min, float max) {
    float scale = rand() / (float) RAND_MAX; /* [0, 1.0] */
    return min + scale * ( max - min );      /* [min, max] */
}

// Declare functions if ..._IMPLEMENTATION is defined in source file
#endif

#ifdef BL_STRING_BUILDER_IMPL
string_builder* new_builder(int capacity) {
  string_builder* b = (string_builder*)malloc(sizeof(string_builder));
  b->buffer   = (char*)malloc(capacity); b->capacity = capacity;
  b->index    = 0;

  return b;
}

void free_builder(string_builder* b) {
  free(b->buffer);
  free(b);
}

void add_to(string_builder* b, char* format, ...) {
  // Try writing to the buffer
  // If expected written characters exceed the buffer capacity, grow it and try again
  for (;;) {
    char* position = b->buffer + b->index;
    int   remaining_capacity = b->capacity - b->index;

    va_list args;
    va_start(args, format);
    int written = vsnprintf(position, remaining_capacity, format, args);
    va_end(args);

    if (written >= remaining_capacity) {
      b->capacity *= 2;
      b->buffer = (char*)realloc(b->buffer, b->capacity);
    } else {
      b->index += written;
      break;
    }
  }
}

void reset(string_builder* b) {
  b->index = 0;
}

char* to_string(string_builder* b) {
  b->buffer[b->index] = 0;
  return b->buffer;
}

#endif
#endif
