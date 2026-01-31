#pragma once

struct betree_err;

#ifdef __cplusplus
extern "C" {
#endif

void add_variable_from_string_err(struct betree_err* betree, const char* line);
void empty_tree_err(struct betree_err* betree);

#ifdef __cplusplus
}
#endif
