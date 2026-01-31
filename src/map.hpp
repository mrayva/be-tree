#pragma once

// Note: map_base_t and map_iter_t are already defined in map.h before this file is included
// This header only provides C++ function declarations with extern "C" linkage

#ifdef __cplusplus
extern "C" {
#endif

void map_deinit_(map_base_t *m);
void *map_get_(map_base_t *m, const char *key);
int map_set_(map_base_t *m, const char *key, void *value, int vsize);
void map_remove_(map_base_t *m, const char *key);
map_iter_t map_iter_(void);
const char *map_next_(map_base_t *m, map_iter_t *iter);

#ifdef __cplusplus
}
#endif
