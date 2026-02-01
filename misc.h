#ifndef MISC_H
#define MISC_H

#include <stdio.h>
#include <stdint.h>

uint8_t* read_exact(FILE *f, size_t size);
char* decode_c_string(const uint8_t *buf, size_t len);
size_t pad_dist_pow2(size_t value, size_t size);
FILE* open_helper(const char *path, const char *mode, int make_dirs);
char* strdup_local(const char *s);

#endif // MISC_H
