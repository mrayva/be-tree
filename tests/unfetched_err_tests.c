#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "alloc.h"
#include "ast.h"
#include "ast_err.h"
#include "betree.h"
#include "betree_err.h"
#include "hashmap.h"
#include "minunit.h"
#include "tree.h"
#include "tree_err.h"

// _err mirror of the flat search tests in unfetched_tests.c. Unlike the
// plain API, betree_flatten_err has no external-buffer precondition to set
// up: betree_make_sub_ids(tree) (already a required step before any _err
// search, per every other _err test file) is all it needs.

int test_flatten_err_lifecycle()
{
    struct betree_err* tree = betree_make_err();
    add_attr_domain_i(tree->config, "x", false);
    mu_assert(betree_insert_err(tree, 1, "x > 5"), "");
    betree_make_sub_ids(tree);

    mu_assert(tree->flat.buf == NULL, "");
    betree_flatten_err(tree);
    mu_assert(tree->flat.buf != NULL, "");
    mu_assert(tree->flat.len > 0, "");

    const uint8_t* first_buf = tree->flat.buf;
    betree_flatten_err(tree);
    mu_assert(tree->flat.buf == first_buf, "");

    betree_free_err(tree);
    return 0;
}

int test_search_flat_err_fully_resolved()
{
    struct betree_err* tree = betree_make_err();
    add_attr_domain_i(tree->config, "x", false);
    add_attr_domain_i(tree->config, "y", false);
    mu_assert(betree_insert_err(tree, 1, "y = 1"), "");
    mu_assert(betree_insert_err(tree, 2, "x > 5"), "");
    betree_make_sub_ids(tree);
    betree_flatten_err(tree);

    struct betree_event* event = betree_make_event_err(tree);
    betree_set_variable(event, 0, betree_make_integer_variable("x", 10));
    betree_set_variable(event, 1, betree_make_integer_variable("y", 1));

    struct report_err* report = make_report_err(tree);
    enum flat_search_result res = betree_search_flat_err(tree, event, report);

    mu_assert(res == FLAT_SEARCH_DONE, "");
    mu_assert(report->state == NULL, "");
    mu_assert(report->matched == 2, "");
    mu_assert(report->subs[0] == 1 && report->subs[1] == 2, "");

    free_report_err(report);
    betree_free_event(event);
    betree_free_err(tree);
    return 0;
}

int test_search_flat_err_yields_then_resumes()
{
    struct betree_err* tree = betree_make_err();
    add_attr_domain_i(tree->config, "x", false);
    add_attr_domain_i(tree->config, "y", false);
    mu_assert(betree_insert_err(tree, 1, "y = 1"), "");
    mu_assert(betree_insert_err(tree, 2, "x > 5"), "");
    betree_make_sub_ids(tree);
    betree_flatten_err(tree);

    struct betree_event* event = betree_make_event_err(tree);
    betree_set_variable(event, 0, betree_make_unfetched_variable("x"));
    betree_set_variable(event, 1, betree_make_integer_variable("y", 1));

    struct report_err* report = make_report_err(tree);
    enum flat_search_result res = betree_search_flat_err(tree, event, report);

    mu_assert(res == FLAT_SEARCH_YIELD, "");
    mu_assert(report->state != NULL, "");
    mu_assert(report->matched == 1, "");
    mu_assert(report->subs[0] == 1, "");

    betree_set_variable(event, 0, betree_make_integer_variable("x", 10));
    res = betree_search_flat_err(tree, event, report);

    mu_assert(res == FLAT_SEARCH_DONE, "");
    mu_assert(report->state == NULL, "");
    mu_assert(report->matched == 2, "");
    mu_assert(report->subs[0] == 1 && report->subs[1] == 2, "");

    free_report_err(report);
    betree_free_event(event);
    betree_free_err(tree);
    return 0;
}

int test_search_flat_err_abandoned_mid_yield_does_not_leak()
{
    struct betree_err* tree = betree_make_err();
    add_attr_domain_i(tree->config, "x", false);
    mu_assert(betree_insert_err(tree, 1, "x > 5"), "");
    betree_make_sub_ids(tree);
    betree_flatten_err(tree);

    struct betree_event* event = betree_make_event_err(tree);
    betree_set_variable(event, 0, betree_make_unfetched_variable("x"));

    struct report_err* report = make_report_err(tree);
    enum flat_search_result res = betree_search_flat_err(tree, event, report);
    mu_assert(res == FLAT_SEARCH_YIELD, "");
    mu_assert(report->state != NULL, "");

    // Abandon without resuming: free_report_err must clean up the
    // suspended flat_search_state_err instead of leaking it.
    free_report_err(report);
    betree_free_event(event);
    betree_free_err(tree);
    return 0;
}

int test_search_flat_err_records_reason_on_mismatch()
{
    struct betree_err* tree = betree_make_err();
    add_attr_domain_i(tree->config, "x", false);
    mu_assert(betree_insert_err(tree, 1, "x = 5"), "");
    betree_make_sub_ids(tree);
    betree_flatten_err(tree);

    struct betree_event* event = betree_make_event_err(tree);
    betree_set_variable(event, 0, betree_make_integer_variable("x", 10));

    struct report_err* report = make_report_err(tree);
    enum flat_search_result res = betree_search_flat_err(tree, event, report);

    mu_assert(res == FLAT_SEARCH_DONE, "");
    mu_assert(report->matched == 0, "");
    dynamic_array_t* rlist = betree_reason_map_get(report->reason_sub_id_list, 0);
    mu_assert(rlist != NULL, "");
    mu_assert(rlist->size == 1 && (betree_var_t)rlist->data[0] == (betree_var_t)1, "");
    mu_assert(strcmp(report->reason_sub_id_list->reasons[0]->name, "x") == 0, "");

    free_report_err(report);
    betree_free_event(event);
    betree_free_err(tree);
    return 0;
}

// Reuses the exact tree shape betree_tests.c's test_cdir_split_twice
// verifies (see unfetched_tests.c's test_search_flat_cdir_prunes_out_of_
// bound_subtree for why the simpler single-cdir shape can't prove
// anything here: that cdir has no lchild/rchild yet and is unbounded on
// both sides).
static int build_double_split_tree_err(struct betree_err* tree)
{
    add_attr_domain_bounded_i(tree->config, "a", false, 0, 10);
    add_attr_domain_bounded_i(tree->config, "b", false, 0, 10);
    mu_assert(betree_insert_err(tree, 1, "a = 2"), "");
    mu_assert(betree_insert_err(tree, 2, "a = 2"), "");
    mu_assert(betree_insert_err(tree, 3, "a = 2"), "");
    mu_assert(betree_insert_err(tree, 4, "b = 0"), "");
    mu_assert(betree_insert_err(tree, 5, "a = 7"), "");
    mu_assert(betree_insert_err(tree, 6, "a = 7"), "");
    mu_assert(betree_insert_err(tree, 7, "a = 7"), "");
    struct cdir_err* cdir = tree->cnode->pdir->pnodes[0]->cdir;
    mu_assert(cdir->lchild != NULL && cdir->lchild->cnode->lnode->sub_count == 3, "");
    mu_assert(cdir->rchild != NULL && cdir->rchild->cnode->lnode->sub_count == 3, "");
    return 0;
}

int test_search_flat_err_records_reason_on_cdir_exclusion()
{
    struct betree_err* tree = betree_make_err();
    mu_assert(build_double_split_tree_err(tree) == 0, "");
    betree_make_sub_ids(tree);
    betree_flatten_err(tree);

    struct betree_event* event = betree_make_event_err(tree);
    // a = 2 falls in the lchild branch; the rchild branch (ids 5,6,7, all
    // "a = 7") is bounded away from it and should be skipped as a whole,
    // with all three ids attributed to "a" as the rejection reason in one
    // shot rather than evaluated (and rejected) individually.
    betree_set_variable(event, 0, betree_make_integer_variable("a", 2));
    betree_set_variable(event, 1, betree_make_integer_variable("b", 0));

    struct report_err* report = make_report_err(tree);
    enum flat_search_result res = betree_search_flat_err(tree, event, report);

    mu_assert(res == FLAT_SEARCH_DONE, "");
    mu_assert(report->matched == 4, "");
    mu_assert(report->evaluated == 4, "");

    dynamic_array_t* rlist = betree_reason_map_get(report->reason_sub_id_list, 0);
    mu_assert(rlist != NULL, "");
    mu_assert(strcmp(report->reason_sub_id_list->reasons[0]->name, "a") == 0, "");
    mu_assert(rlist->size == 3, "");
    {
        bool saw5 = false, saw6 = false, saw7 = false;
        size_t i;
        for(i = 0; i < rlist->size; i++) {
            betree_var_t id = (betree_var_t)rlist->data[i];
            if(id == 5) saw5 = true;
            if(id == 6) saw6 = true;
            if(id == 7) saw7 = true;
        }
        mu_assert(saw5 && saw6 && saw7, "");
    }

    free_report_err(report);
    betree_free_event(event);
    betree_free_err(tree);
    return 0;
}

int all_tests()
{
    mu_run_test(test_flatten_err_lifecycle);
    mu_run_test(test_search_flat_err_fully_resolved);
    mu_run_test(test_search_flat_err_yields_then_resumes);
    mu_run_test(test_search_flat_err_abandoned_mid_yield_does_not_leak);
    mu_run_test(test_search_flat_err_records_reason_on_mismatch);
    mu_run_test(test_search_flat_err_records_reason_on_cdir_exclusion);

    return 0;
}

RUN_TESTS()
