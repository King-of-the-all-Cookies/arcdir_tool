#ifndef ARCDIR_H
#define ARCDIR_H

#include <stdio.h>

typedef struct {
    const char *from;
    const char *to;
} PathRedirect;

void extract_paired_arc_dir(FILE *arc, FILE *dir, PathRedirect *map, int map_count);
void pack_paired_arc_dir(FILE *arc, FILE *dir, char **filepaths, int count);

#endif
