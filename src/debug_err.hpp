#pragma once

struct betree_err;

#ifdef __cplusplus
extern "C" {
#endif

void write_dot_file_err(const struct betree_err* tree);
void write_dot_to_file_err(const struct betree_err* tree, const char* fname);

#ifdef __cplusplus
}
#endif
