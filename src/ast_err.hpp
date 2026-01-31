#pragma once

#include <cstddef>

#include "var.h"  // For betree_var_t

struct betree_variable;
struct ast_node;
struct memoize;
struct report_err;

#ifdef __cplusplus
extern "C" {
#endif

void set_reason_sub_id_list(char* last_reason, const char* variable_name);
bool match_node_err(const struct betree_variable** preds,
    const struct ast_node* node,
    struct memoize* memoize,
    struct report_err* report,
    betree_var_t* last_reason,
    betree_var_t* memoize_reason,
    std::size_t attr_domain_count);

#ifdef __cplusplus
}
#endif
