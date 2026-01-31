#pragma once

#include <cstdio>

struct betree;
struct cnode;
struct pdir;
struct lnode;
struct cdir;
struct pnode;

#ifdef __cplusplus
extern "C" {
#endif

void wrt_dot_to_file(const struct betree* tree, FILE* f);
void write_dot_to_file(const struct betree* tree, const char* fname);
void write_dot_file(const struct betree* tree);
void print_betree(const struct betree* betree);

#ifdef __cplusplus
}
#endif
