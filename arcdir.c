#include "arcdir.h"
#include "misc.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

static uint32_t read_be32(FILE *f) {
    uint8_t b[4];
    read_exact(f, b, 4);
    return (b[0]<<24) | (b[1]<<16) | (b[2]<<8) | b[3];
}

static void write_be32(FILE *f, uint32_t v) {
    uint8_t b[4] = {
        (v>>24)&0xFF, (v>>16)&0xFF, (v>>8)&0xFF, v&0xFF
    };
    fwrite(b, 1, 4, f);
}

void extract_paired_arc_dir(FILE *arc, FILE *dir, PathRedirect *map, int map_count) {
    uint32_t dir_size = read_be32(dir);
    uint32_t count = read_be32(dir);

    for (uint32_t i = 0; i < count; i++) {
        uint32_t offset = read_be32(dir);
        uint32_t size   = read_be32(dir);
        uint32_t strlen = read_be32(dir);

        uint8_t *buf = (uint8_t*)malloc(strlen);
        read_exact(dir, buf, strlen);

        char *path = decode_c_string(buf, strlen);
        free(buf);

        const char *outpath = path;
        if (map && map_count > 0) {
            int found = 0;
            for (int j = 0; j < map_count; j++) {
                if (strcmp(map[j].from, path) == 0) {
                    outpath = map[j].to;
                    found = 1;
                    break;
                }
            }
            if (!found) {
                free(path);
                continue;
            }
        }

        FILE *out = open_helper(outpath, "wb", 1);
        fseek(arc, offset, SEEK_SET);

        uint8_t *data = (uint8_t*)malloc(size);
        read_exact(arc, data, size);
        fwrite(data, 1, size, out);

        fclose(out);
        free(data);

        printf("extracted \"%s\"\n", outpath);
        free(path);
    }
}

void pack_paired_arc_dir(FILE *arc, FILE *dir, char **filepaths, int count) {
    fseek(dir, 8, SEEK_SET);
    uint32_t real_count = 0;

    for (int i = 0; i < count; i++) {
        FILE *f = fopen(filepaths[i], "rb");
        if (!f) continue;

        fseek(f, 0, SEEK_END);
        size_t size = ftell(f);
        fseek(f, 0, SEEK_SET);

        uint8_t *data = (uint8_t*)malloc(size);
        fread(data, 1, size, f);
        fclose(f);

        const char *path = filepaths[i];
        size_t pathlen = strlen(path) + 1;
        size_t pad = pad_dist_pow2(pathlen, 4);
        size_t total = pathlen + pad;

        write_be32(dir, (uint32_t)ftell(arc));
        write_be32(dir, (uint32_t)size);
        write_be32(dir, (uint32_t)total);

        fwrite(path, 1, pathlen, dir);
        for (size_t k = 0; k < pad; k++) fputc(0, dir);

        fwrite(data, 1, size, arc);

        size_t pad2 = pad_dist_pow2(ftell(arc), 32);
        for (size_t k = 0; k < pad2; k++) fputc(0xCC, arc);

        free(data);
        real_count++;
    }

    uint32_t dir_size = (uint32_t)ftell(dir);
    fseek(dir, 0, SEEK_SET);
    write_be32(dir, dir_size);
    write_be32(dir, real_count);
}
