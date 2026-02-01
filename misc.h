#ifndef MISC_H
#define MISC_H

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

size_t read_exact(FILE *f, void *buf, size_t size);
size_t pad_dist_pow2(size_t value, size_t size);
char *decode_c_string(const uint8_t *buf, size_t len);

FILE *open_helper(const char *path, const char *mode, int make_dirs);

#endif
