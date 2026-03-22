#include "helper_test_shared.hpp"
#include "tree.h"
#include "tree_err.h"

extern "C" {

void add_variable_from_string_err(struct betree_err* betree, const char* line)
{
    add_variable_from_string_impl(betree->config, line);
}

void empty_tree_err(struct betree_err* betree)
{
    empty_tree_impl(betree, free_cnode_err, make_cnode_err);
}

}
