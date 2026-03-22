#include "helper_test_shared.hpp"
#include "tree.h"

extern "C" {

void add_variable_from_string(struct betree* betree, const char* line)
{
    add_variable_from_string_impl(betree->config, line);
}

void empty_tree(struct betree* betree)
{
    empty_tree_impl(betree, free_cnode, make_cnode);
}

}
