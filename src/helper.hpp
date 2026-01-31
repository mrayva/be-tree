#pragma once

struct betree;

#ifdef __cplusplus
extern "C" {
#endif

void add_variable_from_string(struct betree* betree, const char* line);
void empty_tree(struct betree* betree);

#ifdef __cplusplus
}
#endif
