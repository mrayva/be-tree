#pragma once

// When compiling as C++, use the modern C++ header
#ifdef __cplusplus
#include "hashmap.hpp"
#else

#include <stddef.h>

#include "jsw_rbtree.h"
#include "memoize.h"

struct ast_node;

struct pred_map {
    betree_pred_t pred_count;
    betree_pred_t memoize_count;
    struct jsw_rbtree* m;
};

void assign_pred(struct pred_map* pred_map, struct ast_node* node);
struct pred_map* make_pred_map(void);
void free_pred_map(struct pred_map* pred_map);

#endif // __cplusplus
