#include "parse.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

typedef struct {
    char **items;
    int count;
    int cap;
} StrList;

static void list_init(StrList *l) {
    l->items = NULL;
    l->count = 0;
    l->cap = 0;
}

static void list_push(StrList *l, const char *s) {
    if (l->count == l->cap) {
        l->cap = l->cap ? l->cap * 2 : 32;
        l->items = (char**)realloc(l->items, l->cap * sizeof(char*));
    }
    l->items[l->count++] = _strdup(s);
}

#ifndef _WIN32
static char *strdup_local(const char *s) {
    size_t n = strlen(s) + 1;
    char *r = (char*)malloc(n);
    memcpy(r, s, n);
    return r;
}
#endif

static int endswith_ci(const char *s, const char *ext) {
    if (!ext || !*ext) return 1;
    size_t ls = strlen(s);
    size_t le = strlen(ext);

    const char *e = ext;
    if (ext[0] == '.') e++;

    if (ls < le) return 0;
    const char *p = s + ls - le;

    for (size_t i = 0; i < le; i++) {
        if (tolower((unsigned char)p[i]) != tolower((unsigned char)e[i]))
            return 0;
    }
    return 1;
}

#ifdef _WIN32

static void walk_dir(const char *base, const char *ext, StrList *out) {
    char pattern[MAX_PATH];
    snprintf(pattern, sizeof(pattern), "%s\\*", base);

    WIN32_FIND_DATAA fd;
    HANDLE h = FindFirstFileA(pattern, &fd);
    if (h == INVALID_HANDLE_VALUE) return;

    do {
        if (strcmp(fd.cFileName, ".") == 0 || strcmp(fd.cFileName, "..") == 0)
            continue;

        char path[MAX_PATH];
        snprintf(path, sizeof(path), "%s\\%s", base, fd.cFileName);

        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            walk_dir(path, ext, out);
        } else {
            if (endswith_ci(fd.cFileName, ext)) {
                list_push(out, path);
            }
        }
    } while (FindNextFileA(h, &fd));

    FindClose(h);
}

#else

static void walk_dir(const char *base, const char *ext, StrList *out) {
    DIR *d = opendir(base);
    if (!d) return;

    struct dirent *ent;
    while ((ent = readdir(d)) != NULL) {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
            continue;

        char path[4096];
        snprintf(path, sizeof(path), "%s/%s", base, ent->d_name);

        struct stat st;
        if (stat(path, &st) != 0) continue;

        if (S_ISDIR(st.st_mode)) {
            walk_dir(path, ext, out);
        } else if (S_ISREG(st.st_mode)) {
            if (endswith_ci(ent->d_name, ext)) {
                list_push(out, path);
            }
        }
    }
    closedir(d);
}

#endif

static int is_directory(const char *path) {
#ifdef _WIN32
    DWORD attr = GetFileAttributesA(path);
    if (attr == INVALID_FILE_ATTRIBUTES) return 0;
    return (attr & FILE_ATTRIBUTE_DIRECTORY) != 0;
#else
    struct stat st;
    if (stat(path, &st) != 0) return 0;
    return S_ISDIR(st.st_mode);
#endif
}

static int cmp_str(const void *a, const void *b) {
    const char *sa = *(const char**)a;
    const char *sb = *(const char**)b;
    return strcmp(sa, sb);
}

char **nodes_to_filepaths_recursive(char **nodes, int node_count, const char *ext, int *out_count) {
    StrList list;
    list_init(&list);

    for (int i = 0; i < node_count; i++) {
        const char *node = nodes[i];
        if (is_directory(node)) {
            walk_dir(node, ext, &list);
        } else {
            // Явно переданный файл — добавляем без фильтра
            list_push(&list, node);
        }
    }

    // Сортировка как в Python sorted()
    qsort(list.items, list.count, sizeof(char*), cmp_str);

    *out_count = list.count;
    return list.items;
}

void free_file_list(char **list, int count) {
    if (!list) return;
    for (int i = 0; i < count; i++) {
        free(list[i]);
    }
    free(list);
}
