#pragma once

#include <cstddef>
#include <cstdio>
#include <cstdlib>

#include "ast.h"
#include "jsw_rbtree.h"
#include "memoize.h"

// Maintain C struct layout for ABI compatibility
// C code in tree.c and tree_err.c directly accesses pred_map->pred_count fields
struct pred_map {
    betree_pred_t pred_count;
    betree_pred_t memoize_count;
    struct jsw_rbtree* m;
};

// C API compatibility
#ifdef __cplusplus
extern "C" {
#endif

void assign_pred(struct pred_map* pred_map, struct ast_node* node);
struct pred_map* make_pred_map(void);
void free_pred_map(struct pred_map* pred_map);

#ifdef __cplusplus
}
#endif
