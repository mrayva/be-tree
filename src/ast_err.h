#pragma once

#include <stdbool.h>

#include "ast.h"
#include "betree.h"
#include "betree_err.h"
#include "config.h"
#include "memoize.h"
#include "value.h"
#include "var.h"

// When compiling as C++, use the modern C++ header for function declarations
#ifdef __cplusplus
#include "ast_err.hpp"
#endif

bool match_node_err(const struct betree_variable** preds,
    const struct ast_node* node,
    struct memoize* memoize,
    struct report_err* report,
    betree_var_t* last_reason,
    betree_var_t* memoize_reason,
    size_t attr_domains_count);

// Tri-state mirror of match_node_err for the flat/continuation search path:
// same last_reason/memoize_reason threading, but returns MATCH_UNKNOWN
// instead of stopping at a BETREE_PRED_UNFETCHED variable.
enum match_result match_node_tri_err(const struct betree_variable** preds,
    const struct ast_node* node,
    struct memoize* memoize,
    struct report_err* report,
    betree_var_t* last_reason,
    betree_var_t* memoize_reason,
    size_t attr_domains_count);

void set_reason_sub_id_list(char* last_reason, const char* variable_name);
