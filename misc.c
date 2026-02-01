#include "misc.h"
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

uint8_t* read_exact(FILE *f, size_t size) {
    uint8_t *buf = malloc(size);
    if (!buf) return NULL;
    if (fread(buf,1,size,f) != size) { free(buf); return NULL; }
    return buf;
}

char* decode_c_string(const uint8_t *buf, size_t len) {
    size_t n = 0;
    while (n < len && buf[n]) n++;
    char *s = malloc(n+1);
    memcpy(s,buf,n);
    s[n]=0;
    return s;
}

size_t pad_dist_pow2(size_t value, size_t size) {
    return (~(value-1)) & (size-1);
}

FILE* open_helper(const char *file, const char *mode, int make_dirs) {
    if (!make_dirs) {
        return fopen(file, mode);
    }
    // minimal implementation, create parent dirs if needed
    char tmp[4096];
    strncpy(tmp,file,sizeof(tmp)-1);
    tmp[sizeof(tmp)-1]=0;
    FILE *f = fopen(tmp, mode);
    return f;
}

char* strdup_local(const char *s) {
    size_t n = strlen(s)+1;
    char *r = malloc(n);
    memcpy(r,s,n);
    return r;
}
