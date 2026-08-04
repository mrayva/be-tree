#include <stdbool.h>
#include <stdio.h>

#include "alloc.h"
#include "ast.h"
#include "betree.h"
#include "hashmap.h"
#include "minunit.h"
#include "printer.h"
#include "tree.h"

// Flat-tree continuation search: BETREE_UNFETCHED marks an event slot as
// deliberately not-yet-fetched. Covers construction/lifetime, the
// BETREE_PRED_UNFETCHED sentinel, the match_node_tri truth table, and
// end-to-end betree_flatten + betree_search_flat (including yield/resume).

int test_make_and_free_standalone()
{
    struct betree_variable* v = betree_make_unfetched_variable("x");
    mu_assert(v != NULL, "");
    mu_assert(v->value.value_type == BETREE_UNFETCHED, "");
    betree_free_variable(v);
    return 0;
}

int test_attach_to_event_and_free()
{
    struct betree* tree = betree_make();
    add_attr_domain_i(tree->config, "x", false);

    struct betree_event* event = betree_make_event(tree);
    betree_set_variable(event, 0, betree_make_unfetched_variable("x"));
    mu_assert(event->variables[0]->value.value_type == BETREE_UNFETCHED, "");

    betree_free_event(event);
    betree_free(tree);
    return 0;
}

int test_print_does_not_crash()
{
    struct betree_variable* v = betree_make_unfetched_variable("x");
    print_variable(v);
    print_value_type(v->value.value_type);
    betree_free_variable(v);
    return 0;
}

int test_make_environment_maps_to_sentinel()
{
    struct betree* tree = betree_make();
    add_attr_domain_i(tree->config, "x", false);
    add_attr_domain_i(tree->config, "y", false);

    struct betree_event* event = betree_make_event(tree);
    betree_set_variable(event, 0, betree_make_unfetched_variable("x"));
    betree_set_variable(event, 1, betree_make_integer_variable("y", 42));

    const struct betree_variable** preds
        = make_environment(tree->config->attr_domain_count, event);
    mu_assert(preds[0] == &BETREE_PRED_UNFETCHED, "");
    mu_assert(preds[1] != NULL && preds[1] != &BETREE_PRED_UNFETCHED, "");
    mu_assert(preds[1]->value.integer_value == 42, "");

    bfree(preds);
    betree_free_event(event);
    betree_free(tree);
    return 0;
}

// betree_make_sub registers the sub's predicates in tree->config->pred_map
// for memoization dedup; that registration is only torn down by freeing the
// owning tree, not by a standalone free_sub. So, matching the pattern every
// other test in this suite uses, each sub built here is inserted (never
// freed on its own) and the whole tree is torn down by the caller.
static enum match_result eval_expr_with_bool_vars(struct betree* tree,
    betree_sub_t id,
    const char* expr,
    bool a_unfetched,
    bool a_value,
    bool b_unfetched,
    bool b_value)
{
    struct betree_sub* sub = betree_make_sub(tree, id, 0, NULL, expr);
    if(sub == NULL) {
        return MATCH_FALSE;
    }
    betree_insert_sub(tree, sub);

    struct betree_event* event = betree_make_event(tree);
    betree_set_variable(event, 0,
        a_unfetched ? betree_make_unfetched_variable("a") : betree_make_boolean_variable("a", a_value));
    betree_set_variable(event, 1,
        b_unfetched ? betree_make_unfetched_variable("b") : betree_make_boolean_variable("b", b_value));

    const struct betree_variable** preds
        = make_environment(tree->config->attr_domain_count, event);
    struct memoize memoize = make_memoize(tree->config->pred_map->memoize_count);
    struct report* report = make_report();

    enum match_result result = match_node_tri(preds, sub->expr, &memoize, report);

    free_report(report);
    free_memoize(memoize);
    bfree(preds);
    betree_free_event(event);
    return result;
}

int test_tri_state_and_short_circuit()
{
    struct betree* tree = betree_make();
    add_attr_domain_b(tree->config, "a", false);
    add_attr_domain_b(tree->config, "b", false);

    // a=false, b unfetched: lhs alone decides false, no need for b.
    mu_assert(eval_expr_with_bool_vars(tree, 1, "a and b", false, false, true, false) == MATCH_FALSE, "");
    // a=true, b unfetched: can't decide without b.
    mu_assert(eval_expr_with_bool_vars(tree, 2, "a and b", false, true, true, false) == MATCH_UNKNOWN, "");
    // a unfetched, b=false: rhs alone decides false even though lhs is unknown.
    mu_assert(eval_expr_with_bool_vars(tree, 3, "a and b", true, false, false, false) == MATCH_FALSE, "");
    // a unfetched, b=true: still can't decide.
    mu_assert(eval_expr_with_bool_vars(tree, 4, "a and b", true, false, false, true) == MATCH_UNKNOWN, "");

    betree_free(tree);
    return 0;
}

int test_tri_state_or_short_circuit()
{
    struct betree* tree = betree_make();
    add_attr_domain_b(tree->config, "a", false);
    add_attr_domain_b(tree->config, "b", false);

    // a=true, b unfetched: lhs alone decides true.
    mu_assert(eval_expr_with_bool_vars(tree, 1, "a or b", false, true, true, false) == MATCH_TRUE, "");
    // a=false, b unfetched: can't decide without b.
    mu_assert(eval_expr_with_bool_vars(tree, 2, "a or b", false, false, true, false) == MATCH_UNKNOWN, "");
    // a unfetched, b=true: rhs alone decides true.
    mu_assert(eval_expr_with_bool_vars(tree, 3, "a or b", true, false, false, true) == MATCH_TRUE, "");
    // a unfetched, b=false: still can't decide.
    mu_assert(eval_expr_with_bool_vars(tree, 4, "a or b", true, false, false, false) == MATCH_UNKNOWN, "");

    betree_free(tree);
    return 0;
}

int test_tri_state_not_unknown()
{
    struct betree* tree = betree_make();
    add_attr_domain_b(tree->config, "a", false);
    add_attr_domain_b(tree->config, "b", false);

    mu_assert(eval_expr_with_bool_vars(tree, 1, "not a", true, false, false, false) == MATCH_UNKNOWN, "");
    mu_assert(eval_expr_with_bool_vars(tree, 2, "not a", false, true, false, false) == MATCH_FALSE, "");
    mu_assert(eval_expr_with_bool_vars(tree, 3, "not a", false, false, false, false) == MATCH_TRUE, "");

    betree_free(tree);
    return 0;
}

int test_tri_state_compare_expr_unknown()
{
    struct betree* tree = betree_make();
    add_attr_domain_i(tree->config, "x", false);

    struct betree_sub* sub = betree_make_sub(tree, 1, 0, NULL, "x > 5");
    mu_assert(sub != NULL, "");
    betree_insert_sub(tree, sub);

    struct betree_event* event = betree_make_event(tree);
    betree_set_variable(event, 0, betree_make_unfetched_variable("x"));

    const struct betree_variable** preds
        = make_environment(tree->config->attr_domain_count, event);
    struct memoize memoize = make_memoize(tree->config->pred_map->memoize_count);
    struct report* report = make_report();

    mu_assert(match_node_tri(preds, sub->expr, &memoize, report) == MATCH_UNKNOWN, "");

    free_report(report);
    free_memoize(memoize);
    bfree(preds);
    betree_free_event(event);
    betree_free(tree);
    return 0;
}

// betree_flatten requires tree->subs_data to already be allocated (it only
// fills it in); matches the setup betree_search_cb_tests.c uses for the
// same assert around betree_prepare_sub_data.
static void alloc_subs_data(struct betree* tree, size_t capacity)
{
    tree->subs_data = (struct subs_data*)bcalloc(sizeof(struct subs_data));
    tree->subs_data->array = (void**)bcalloc(capacity * sizeof(void*));
    tree->subs_data->limit = capacity;
    tree->subs_data->count = 0;
}

// betree_free does not own subs_data (it's caller-allocated, matching
// betree_search_cb_tests.c's test_excluded_branch_callback), so it must be
// freed before the tree.
static void free_subs_data(struct betree* tree)
{
    bfree(tree->subs_data->array);
    bfree(tree->subs_data);
    tree->subs_data = NULL;
}

int test_flatten_lifecycle()
{
    struct betree* tree = betree_make();
    add_attr_domain_i(tree->config, "x", false);
    mu_assert(betree_insert(tree, 1, "x > 5"), "");

    alloc_subs_data(tree, 4);
    mu_assert(tree->flat.buf == NULL, "");
    betree_flatten(tree);
    mu_assert(tree->flat.buf != NULL, "");
    mu_assert(tree->flat.len > 0, "");

    // Idempotent: calling again with an already-flattened tree is a no-op,
    // not a leak-and-reflatten.
    const uint8_t* first_buf = tree->flat.buf;
    betree_flatten(tree);
    mu_assert(tree->flat.buf == first_buf, "");

    free_subs_data(tree);
    betree_free(tree);
    return 0;
}

int test_search_flat_fully_resolved_matches_regular_search()
{
    struct betree* tree = betree_make();
    add_attr_domain_i(tree->config, "x", false);
    add_attr_domain_i(tree->config, "y", false);
    mu_assert(betree_insert(tree, 1, "y = 1"), "");
    mu_assert(betree_insert(tree, 2, "x > 5"), "");

    alloc_subs_data(tree, 4);
    betree_flatten(tree);

    struct betree_event* event = betree_make_event(tree);
    betree_set_variable(event, 0, betree_make_integer_variable("x", 10));
    betree_set_variable(event, 1, betree_make_integer_variable("y", 1));

    struct report* report = make_report();
    enum flat_search_result res = betree_search_flat(tree, event, report);

    mu_assert(res == FLAT_SEARCH_DONE, "");
    mu_assert(report->state == NULL, "");
    mu_assert(report->matched == 2, "");
    mu_assert(report->subs[0] == 1 && report->subs[1] == 2, "");

    free_report(report);
    betree_free_event(event);
    free_subs_data(tree);
    betree_free(tree);
    return 0;
}

int test_search_flat_yields_then_resumes()
{
    struct betree* tree = betree_make();
    add_attr_domain_i(tree->config, "x", false);
    add_attr_domain_i(tree->config, "y", false);
    // Insertion order matters here: sub 1 doesn't touch x and should be
    // decided before the search ever reaches sub 2, which does.
    mu_assert(betree_insert(tree, 1, "y = 1"), "");
    mu_assert(betree_insert(tree, 2, "x > 5"), "");

    alloc_subs_data(tree, 4);
    betree_flatten(tree);

    struct betree_event* event = betree_make_event(tree);
    betree_set_variable(event, 0, betree_make_unfetched_variable("x"));
    betree_set_variable(event, 1, betree_make_integer_variable("y", 1));

    struct report* report = make_report();
    enum flat_search_result res = betree_search_flat(tree, event, report);

    mu_assert(res == FLAT_SEARCH_YIELD, "");
    mu_assert(report->state != NULL, "");
    // sub 1 was already decided before the search stalled on sub 2.
    mu_assert(report->matched == 1, "");
    mu_assert(report->subs[0] == 1, "");

    // Resolve x and resume with the same report.
    betree_set_variable(event, 0, betree_make_integer_variable("x", 10));
    res = betree_search_flat(tree, event, report);

    mu_assert(res == FLAT_SEARCH_DONE, "");
    mu_assert(report->state == NULL, "");
    mu_assert(report->matched == 2, "");
    mu_assert(report->subs[0] == 1 && report->subs[1] == 2, "");

    free_report(report);
    betree_free_event(event);
    free_subs_data(tree);
    betree_free(tree);
    return 0;
}

int test_search_flat_abandoned_mid_yield_does_not_leak()
{
    struct betree* tree = betree_make();
    add_attr_domain_i(tree->config, "x", false);
    mu_assert(betree_insert(tree, 1, "x > 5"), "");

    alloc_subs_data(tree, 4);
    betree_flatten(tree);

    struct betree_event* event = betree_make_event(tree);
    betree_set_variable(event, 0, betree_make_unfetched_variable("x"));

    struct report* report = make_report();
    enum flat_search_result res = betree_search_flat(tree, event, report);
    mu_assert(res == FLAT_SEARCH_YIELD, "");
    mu_assert(report->state != NULL, "");

    // Abandon the search without resuming: free_report must clean up the
    // suspended flat_search_state instead of leaking it.
    free_report(report);
    betree_free_event(event);
    free_subs_data(tree);
    betree_free(tree);
    return 0;
}

// The next two tests reuse the exact tree shape betree_tests.c's
// test_insert_first_split already verifies: inserting "a = 0", "a = 1",
// "a = 2" then "b = 0" splits the root lnode on "a", leaving "b = 0" (id 3)
// alone in the root lnode and pushing "a = 0"/"a = 1"/"a = 2" (ids 0,1,2)
// into a nested lnode reachable via a pnode + cdir for "a". That gives a
// real CHUNK_PNODE/CHUNK_CDIR to exercise instead of the flat single-lnode
// trees the earlier tests use.
static int build_split_tree(struct betree* tree)
{
    add_attr_domain_bounded_i(tree->config, "a", false, 0, 10);
    add_attr_domain_bounded_i(tree->config, "b", false, 0, 10);
    mu_assert(betree_insert(tree, 0, "a = 0"), "");
    mu_assert(betree_insert(tree, 1, "a = 1"), "");
    mu_assert(betree_insert(tree, 2, "a = 2"), "");
    mu_assert(betree_insert(tree, 3, "b = 0"), "");
    // Sanity-check the assumed shape before building a test on top of it.
    mu_assert(tree->cnode->lnode->sub_count == 1, "");
    mu_assert(tree->cnode->pdir != NULL && tree->cnode->pdir->pnode_count == 1, "");
    mu_assert(tree->cnode->pdir->pnodes[0]->cdir->cnode->lnode->sub_count == 3, "");
    return 0;
}

int test_search_flat_pnode_yields_and_resumes_into_nested_subtree()
{
    struct betree* tree = betree_make();
    mu_assert(build_split_tree(tree) == 0, "");

    alloc_subs_data(tree, 4);
    betree_flatten(tree);

    struct betree_event* event = betree_make_event(tree);
    betree_set_variable(event, 0, betree_make_unfetched_variable("a"));
    betree_set_variable(event, 1, betree_make_integer_variable("b", 0));

    struct report* report = make_report();
    enum flat_search_result res = betree_search_flat(tree, event, report);

    // "b = 0" (root lnode) is decided before the walk ever reaches the
    // CHUNK_PNODE for "a", which is where this must stall.
    mu_assert(res == FLAT_SEARCH_YIELD, "");
    mu_assert(report->state != NULL, "");
    mu_assert(report->matched == 1, "");
    mu_assert(report->subs[0] == 3, "");

    // a = 1 is within the cdir's bound, so resuming must descend into the
    // nested subtree and evaluate all three subs there, matching only id 1.
    betree_set_variable(event, 0, betree_make_integer_variable("a", 1));
    res = betree_search_flat(tree, event, report);

    mu_assert(res == FLAT_SEARCH_DONE, "");
    mu_assert(report->state == NULL, "");
    mu_assert(report->matched == 2, "");
    mu_assert(report->subs[0] == 3 && report->subs[1] == 1, "");

    free_report(report);
    betree_free_event(event);
    free_subs_data(tree);
    betree_free(tree);
    return 0;
}

// build_split_tree's cdir has no lchild/rchild yet (test_insert_first_split
// confirms this), so both its sides are "open"/unbounded -- nothing there
// could ever be pruned by a bound check. This mirrors betree_tests.c's
// test_cdir_split_twice instead: "a = 2" x3 (ids 1,2,3), then "b = 0" (id
// 4) triggers the same pnode+cdir split as above, then "a = 7" x3 (ids
// 5,6,7) is enough to further split that cdir into a real lchild (ids
// 1,2,3, bounded on the right now) and rchild (ids 5,6,7, bounded on the
// left) -- the first level of the tree with an actually prunable bound.
static int build_double_split_tree(struct betree* tree)
{
    add_attr_domain_bounded_i(tree->config, "a", false, 0, 10);
    add_attr_domain_bounded_i(tree->config, "b", false, 0, 10);
    mu_assert(betree_insert(tree, 1, "a = 2"), "");
    mu_assert(betree_insert(tree, 2, "a = 2"), "");
    mu_assert(betree_insert(tree, 3, "a = 2"), "");
    mu_assert(betree_insert(tree, 4, "b = 0"), "");
    mu_assert(betree_insert(tree, 5, "a = 7"), "");
    mu_assert(betree_insert(tree, 6, "a = 7"), "");
    mu_assert(betree_insert(tree, 7, "a = 7"), "");
    // Sanity-check the assumed shape before building a test on top of it.
    struct cdir* cdir = tree->cnode->pdir->pnodes[0]->cdir;
    mu_assert(cdir->lchild != NULL && cdir->lchild->cnode->lnode->sub_count == 3, "");
    mu_assert(cdir->rchild != NULL && cdir->rchild->cnode->lnode->sub_count == 3, "");
    mu_assert(tree->cnode->lnode->sub_count == 1 && tree->cnode->lnode->subs[0]->id == 4, "");
    return 0;
}

int test_search_flat_cdir_prunes_out_of_bound_subtree()
{
    struct betree* tree = betree_make();
    mu_assert(build_double_split_tree(tree) == 0, "");

    alloc_subs_data(tree, 8);
    betree_flatten(tree);

    struct betree_event* event = betree_make_event(tree);
    // a = 2 falls in the lchild branch (ids 1,2,3); the rchild branch (ids
    // 5,6,7, all "a = 7") is now genuinely bounded away from it and should
    // be skipped via its cdir's serialized skip length -- never visiting,
    // let alone evaluating, any of the three subs in it.
    betree_set_variable(event, 0, betree_make_integer_variable("a", 2));
    betree_set_variable(event, 1, betree_make_integer_variable("b", 0));

    struct report* report = make_report();
    enum flat_search_result res = betree_search_flat(tree, event, report);

    mu_assert(res == FLAT_SEARCH_DONE, "");
    // "b = 0" (id 4) and all three "a = 2" subs (ids 1,2,3) match.
    mu_assert(report->matched == 4, "");
    // The real proof the rchild subtree was skipped rather than walked and
    // rejected sub-by-sub: only 4 subs were ever evaluated, not all 7.
    mu_assert(report->evaluated == 4, "");

    free_report(report);
    betree_free_event(event);
    free_subs_data(tree);
    betree_free(tree);
    return 0;
}

int all_tests()
{
    mu_run_test(test_make_and_free_standalone);
    mu_run_test(test_attach_to_event_and_free);
    mu_run_test(test_print_does_not_crash);
    mu_run_test(test_make_environment_maps_to_sentinel);
    mu_run_test(test_tri_state_and_short_circuit);
    mu_run_test(test_tri_state_or_short_circuit);
    mu_run_test(test_tri_state_not_unknown);
    mu_run_test(test_tri_state_compare_expr_unknown);
    mu_run_test(test_flatten_lifecycle);
    mu_run_test(test_search_flat_fully_resolved_matches_regular_search);
    mu_run_test(test_search_flat_yields_then_resumes);
    mu_run_test(test_search_flat_abandoned_mid_yield_does_not_leak);
    mu_run_test(test_search_flat_pnode_yields_and_resumes_into_nested_subtree);
    mu_run_test(test_search_flat_cdir_prunes_out_of_bound_subtree);

    return 0;
}

RUN_TESTS()
