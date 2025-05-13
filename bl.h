#ifndef BL_H
#define BL_H

#include <stdint.h>

int read_file(const char* filename, uint8_t** data);
void _release_assert(const char *assertionExpr, const char *assertionFile,
                    unsigned int assertionLine, const char *assertionFunction);
float random_float(float min, float max);

#ifdef BL_IMPL

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>

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

#endif

#endif
