#include <betree.h>
#include <betree_cpp.hpp>
#include <betree_err.h>

int main()
{
    be::Tree tree;
    tree.add_integer("age", false, 0, 120).insert(1, "age >= 21");
    if(tree.search(R"({"age": 25})").size() != 1) {
        return 1;
    }

    betree_err* err_tree = betree_make_err();
    betree_add_integer_variable_err(err_tree, "age", false, 0, 120);
    const bool inserted = betree_insert_err(err_tree, 1, "age >= 21");
    betree_free_err(err_tree);
    return inserted ? 0 : 1;
}
