#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arcdir.h"
#include "parse.h"

#ifdef _WIN32
#define strcasecmp _stricmp
#endif

static void help_extract(void) {
    puts(
        "Usage:\n"
        "  arcdir_tool EXTRACT <archive.arc> <index.dir> [<src> <dst>]...\n"
        "\n"
        "Example:\n"
        "  arcdir_tool EXTRACT data.arc data.dir\n"
        "  arcdir_tool EXTRACT data.arc data.dir a/b.bin out.bin\n"
    );
}

static void help_pack(void) {
    puts(
        "Usage:\n"
        "  arcdir_tool PACK <archive.arc> <index.dir> [path...]\n"
        "  arcdir_tool PACK_BIN <archive.arc> <index.dir> [path...]\n"
        "\n"
        "Example:\n"
        "  arcdir_tool PACK data.arc data.dir assets/\n"
        "  arcdir_tool PACK_BIN data.arc data.dir assets/\n"
    );
}

static void help_all(void) {
    puts("arcdir_tool - ARC/DIR packer and extractor\n");
    help_extract();
    puts("");
    help_pack();
}


int main(int argc, char **argv) {
    if (argc < 2) {
        help_all();
        return 0;
    }

    if (strcasecmp(argv[1], "extract") == 0) {
        if (argc < 4 || ((argc - 4) % 2) != 0) {
            help_extract();
            return 0;
        }

        FILE *arc = fopen(argv[2], "rb");
        FILE *dir = fopen(argv[3], "rb");
        if (!arc || !dir) {
            fprintf(stderr, "Failed to open input files.\n");
            return 1;
        }

        int map_count = (argc - 4) / 2;
        PathRedirect *map = NULL;

        if (map_count > 0) {
            map = (PathRedirect*)malloc(sizeof(PathRedirect) * map_count);
            for (int i = 0; i < map_count; i++) {
                map[i].from = argv[4 + i * 2];
                map[i].to   = argv[5 + i * 2];
            }
        }

        extract_paired_arc_dir(arc, dir, map, map_count);

        fclose(arc);
        fclose(dir);
        free(map);
        return 0;
    }

    if (strcasecmp(argv[1], "pack") == 0 || strcasecmp(argv[1], "pack_bin") == 0) {
        if (argc < 4) {
            help_pack();
            return 0;
        }

        FILE *arc = fopen(argv[2], "wb");
        FILE *dir = fopen(argv[3], "wb");
        if (!arc || !dir) {
            fprintf(stderr, "Failed to open output files.\n");
            return 1;
        }

        const char *ext = "";
        if (strcasecmp(argv[1], "pack_bin") == 0) {
            ext = "bin";
        }

        int file_count = 0;
        char **files = nodes_to_filepaths_recursive(&argv[4], argc - 4, ext, &file_count);

        pack_paired_arc_dir(arc, dir, files, file_count);

        free_file_list(files, file_count);

        fclose(arc);
        fclose(dir);
        return 0;
    }

    help_all();
    return 0;
}
