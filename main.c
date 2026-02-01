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
        "Usage: arcdir_tool EXTRACT <*.arc path> <*.dir path> "
        "[<file 1> <destination 1>] [<file 2> <destination 2>] ... [<file N> <destination N>]\n"
        "You may provide optional file and destination argument pairs to extract individual files to an exact destination. "
        "Otherwise, all files will be extracted to their relative filepaths."
    );
}

static void help_pack(void) {
    puts(
        "Usage: arcdir_tool PACK <*.arc path> <*.dir path> [path 1] [path 2] ... [path N]\n"
        "       arcdir_tool PACK_BIN <*.arc path> <*.dir path> [path 1] [path 2] ... [path N]\n"
        "Each optional path argument will pack that file (or directory, recursively) into the archive. "
        "The latter command will filter recursively found files by their extension."
    );
}

static void help_all(void) {
    puts("All filepaths are relative to the current working directory, so it is recommended to run this program from the dataDir directory of Pikmin 1 when packing.\n");
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
