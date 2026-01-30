#include "hashmap.hpp"

#include <cstdio>
#include <cstdlib>

#include "alloc.h"
#include "ast.h"
#include "ast_compare.h"
#include "jsw_rbtree.h"
#include "map.h"
#include "printer.h"
#include "utils.h"

extern "C" {

void assign_pred(struct pred_map* pred_map, struct ast_node* node) {
    if (node->type == AST_TYPE_BOOL_EXPR && node->bool_expr.op == AST_BOOL_NOT) {
        assign_pred(pred_map, node->bool_expr.unary.expr);
    } else if (node->type == AST_TYPE_BOOL_EXPR && node->bool_expr.op == AST_BOOL_OR) {
        assign_pred(pred_map, node->bool_expr.binary.lhs);
        assign_pred(pred_map, node->bool_expr.binary.rhs);
    } else if (node->type == AST_TYPE_BOOL_EXPR && node->bool_expr.op == AST_BOOL_AND) {
        assign_pred(pred_map, node->bool_expr.binary.lhs);
        assign_pred(pred_map, node->bool_expr.binary.rhs);
    }

    auto* find = static_cast<struct ast_node*>(jsw_rbfind(pred_map->m, node));
    if (find == nullptr) {
        betree_pred_t global_id = pred_map->pred_count;
        pred_map->pred_count++;
        node->global_id = global_id;
        int ret = jsw_rbinsert(pred_map->m, node);
        if (ret == 0) {
            std::abort();
        }
    } else {
        node->global_id = find->global_id;
        if (find->memoize_id == INVALID_PRED) {
            betree_pred_t memoize_id = pred_map->memoize_count;
            pred_map->memoize_count++;
            find->memoize_id = memoize_id;
        }
        node->memoize_id = find->memoize_id;
    }
}

static struct jsw_rbtree* exprmap_new() {
    return jsw_rbnew(expr_cmp);
}

struct pred_map* make_pred_map() {
    auto* map = static_cast<struct pred_map*>(bcalloc(sizeof(struct pred_map)));
    if (map == nullptr) {
        std::fprintf(stderr, "%s bcalloc failed\n", __func__);
        std::abort();
    }
    map->pred_count = 0;
    map->memoize_count = 0;
    map->m = exprmap_new();
    return map;
}

void free_pred_map(struct pred_map* pred_map) {
    jsw_rbdelete(pred_map->m);
    bfree(pred_map);
}

} // extern "C"
