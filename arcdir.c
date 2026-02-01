#include "arcdir.h"
#include "misc.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static uint32_t read_be32(FILE *f) {
    uint8_t b[4];
    if (fread(b, 1, 4, f) != 4) exit(1);
    return ((uint32_t)b[0] << 24) |
           ((uint32_t)b[1] << 16) |
           ((uint32_t)b[2] << 8)  |
           (uint32_t)b[3];
}

static void write_be32(FILE *f, uint32_t v) {
    uint8_t b[4] = {
        (uint8_t)((v >> 24) & 0xFF),
        (uint8_t)((v >> 16) & 0xFF),
        (uint8_t)((v >> 8) & 0xFF),
        (uint8_t)(v & 0xFF)
    };
    fwrite(b, 1, 4, f);
}

void extract_paired_arc_dir(FILE *arc, FILE *dir, PathRedirect *map, int map_count) {
    (void)read_be32(dir); // dir_size, не используется
    uint32_t count = read_be32(dir);

    for (uint32_t i = 0; i < count; i++) {
        uint32_t offset = read_be32(dir);
        uint32_t size = read_be32(dir);
        uint32_t path_len = read_be32(dir);

        char *path_raw = malloc(path_len);
        fread(path_raw, 1, path_len, dir);

        char *path = decode_c_string((const uint8_t*)path_raw, path_len);
        free(path_raw);

        if (map_count > 0) {
            int found = 0;
            for (int j = 0; j < map_count; j++) {
                if (strcmp(path, map[j].from) == 0) {
                    free(path);
                    path = strdup_local(map[j].to);
                    found = 1;
                    break;
                }
            }
            if (!found) { free(path); continue; }
        }

        fseek(arc, (long)offset, SEEK_SET);
        FILE *out = open_helper(path, "wb", 1);
        uint8_t *buf = malloc((size_t)size);
        fread(buf, 1, (size_t)size, arc);
        fwrite(buf, 1, (size_t)size, out);
        fclose(out);
        free(buf);

        printf("extracted \"%s\"\n", path);
        free(path);
    }
}

void pack_paired_arc_dir(FILE *arc, FILE *dir, char **filepaths, int count) {
    fseek(dir, 8, SEEK_SET);
    for (int i = 0; i < count; i++) {
        FILE *f = fopen(filepaths[i], "rb");
        if (!f) continue;

        fseek(f, 0, SEEK_END);
        long size_long = ftell(f);
        fseek(f, 0, SEEK_SET);
        size_t size = (size_t)size_long;

        uint8_t *data = malloc(size);
        fread(data, 1, size, f);
        fclose(f);

        char *path = strdup_local(filepaths[i]);
        write_be32(dir, (uint32_t)ftell(arc));
        write_be32(dir, (uint32_t)size);
        size_t path_len = strlen(path) + 1;
        size_t pad = pad_dist_pow2(path_len, 4);
        write_be32(dir, (uint32_t)(path_len + pad)); // <-- исправлено
        fwrite(path, 1, path_len, dir);
        for (size_t p = 0; p < pad; p++) fputc(0, dir);

        fwrite(data, 1, size, arc);
        free(data);
        free(path);

        size_t pad2 = pad_dist_pow2((size_t)ftell(arc), 32);
        for (size_t p = 0; p < pad2; p++) fputc(0xCC, arc);
    }
    uint32_t dir_size = (uint32_t)ftell(dir);
    fseek(dir, 0, SEEK_SET);
    write_be32(dir, dir_size);
    write_be32(dir, (uint32_t)count);
}
