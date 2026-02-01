#ifndef PARSE_H
#define PARSE_H

char **nodes_to_filepaths_recursive(char **nodes, int node_count, const char *ext, int *out_count);

void free_file_list(char **list, int count);

#endif
