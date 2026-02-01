#include "misc.h"
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#define MKDIR(p) _mkdir(p)
#else
#include <unistd.h>
#define MKDIR(p) mkdir(p, 0755)
#endif

size_t read_exact(FILE *f, void *buf, size_t size) {
    size_t r = fread(buf, 1, size, f);
    if (r != size) {
        fprintf(stderr, "Unexpected EOF\n");
        exit(1);
    }
    return r;
}

size_t pad_dist_pow2(size_t value, size_t size) {
    return (~(value - 1)) & (size - 1);
}

char *decode_c_string(const uint8_t *buf, size_t len) {
    size_t real = 0;
    while (real < len && buf[real] != 0) real++;
    char *s = (char*)malloc(real + 1);
    memcpy(s, buf, real);
    s[real] = 0;
    return s;
}

static void make_dirs_for(const char *path) {
    char tmp[1024];
    strncpy(tmp, path, sizeof(tmp));
    for (char *p = tmp + 1; *p; p++) {
        if (*p == '/' || *p == '\\') {
            char c = *p;
            *p = 0;
            MKDIR(tmp);
            *p = c;
        }
    }
}

FILE *open_helper(const char *path, const char *mode, int make_dirs) {
    if (make_dirs) {
        make_dirs_for(path);
    }
    return fopen(path, mode);
}
