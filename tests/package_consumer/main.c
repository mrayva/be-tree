#include <betree.h>
#include <betree_err.h>

int main(void)
{
    struct betree* tree = betree_make();
    betree_add_integer_variable(tree, "age", false, 0, 120);
    if(!betree_insert(tree, 1, "age >= 21")) {
        betree_free(tree);
        return 1;
    }

    struct report* report = make_report();
    const bool matched = betree_search(tree, "{\"age\": 25}", report)
        && report->matched == 1;
    free_report(report);
    betree_free(tree);

    struct betree_err* err_tree = betree_make_err();
    betree_free_err(err_tree);
    return matched ? 0 : 1;
}
