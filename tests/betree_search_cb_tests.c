
#include <stdio.h>
#include "alloc.h"
#include "betree.h"
#include "tree.h"
#include "minunit.h"

static int test_one()
{
    struct betree* tree = betree_make();

    betree_add_boolean_variable(tree, "stop", false);

    betree_insert(tree, 1, "stop");

    struct report *report = make_report();
    struct betree_event* event = betree_make_event(tree);
    betree_set_variable(event, 0, betree_make_boolean_variable("stop", false));
    betree_search_with_event(tree, event, report);

    mu_assert(report->matched == 0, "");

    free_report(report);
    betree_free_event(event);

    report = make_report();
    event = betree_make_event(tree);
    betree_set_variable(event, 0, betree_make_boolean_variable("stop", true));
    betree_search_with_event(tree, event, report);

    mu_assert(report->matched == 1, "");
    mu_assert(report->subs[0] == 1, "");

    free_report(report);
    betree_free_event(event);
    betree_free(tree);

    return 0;
}

static void *last_arg;
static void *last_data;
static bool last_result;
static const void *last_context;
static void *last_excluded_arg;
static void *last_excluded_data[8];
static size_t last_excluded_count;
static const void *last_excluded_context;
static void *seen_data[8];
static bool seen_result[8];
static const void *seen_context[8];
static size_t seen_count;

static void hook(void *arg, void *data, bool result, const void *context) {
    last_arg = arg;
    last_data = data;
    last_result = result;
    last_context = context;
    if(seen_count < 8) {
        seen_data[seen_count] = data;
        seen_result[seen_count] = result;
        seen_context[seen_count] = context;
        seen_count++;
    }
}

static void hook_excluded(void *arg, void **data, size_t count, const void *context)
{
    size_t i;
    last_excluded_arg = arg;
    last_excluded_count = count;
    last_excluded_context = context;
    for(i = 0; i < count && i < 8; i++) {
        last_excluded_data[i] = data[i];
    }
}

static void reset_hooks(void)
{
    size_t i;
    last_arg = NULL;
    last_data = NULL;
    last_result = false;
    last_context = NULL;
    last_excluded_arg = NULL;
    last_excluded_count = 0;
    last_excluded_context = NULL;
    seen_count = 0;
    for(i = 0; i < 8; i++) {
        last_excluded_data[i] = NULL;
        seen_data[i] = NULL;
        seen_result[i] = false;
        seen_context[i] = NULL;
    }
}

static int seen_contains(void *data, bool result, betree_var_t context)
{
    size_t i;
    for(i = 0; i < seen_count; i++) {
        if(seen_data[i] == data && seen_result[i] == result && (betree_var_t)seen_context[i] == context) {
            return 1;
        }
    }
    return 0;
}

static int test_two()
{
    struct betree* tree = betree_make();

    betree_add_boolean_variable(tree, "stop", false);

    struct betree_sub* sub = betree_make_sub(tree, 1, 0, NULL, "stop");
    mu_assert(sub != NULL, "");
    sub->data = (void *)555;
    mu_assert(betree_insert_sub(tree, sub), "");

    reset_hooks();
    struct report *report = make_report();
    report->cb = hook;
    report->arg = (void *)123;

    struct betree_event* event = betree_make_event(tree);
    struct betree_variable *var = betree_make_boolean_variable("stop", false);
    betree_set_variable(event, 0, var);
    betree_search_with_event(tree, event, report);

    mu_assert(report->matched == 0, "");
    mu_assert(last_arg == (void *)123, "");
    mu_assert(last_data == (void *)555, "");
    mu_assert(last_result == false, "");
    mu_assert((betree_var_t)last_context == 0, "");

    free_report(report);
    betree_free_event(event);

    reset_hooks();
    report = make_report();
    report->cb = hook;
    report->arg = (void *)456;

    event = betree_make_event(tree);
    var = betree_make_boolean_variable("stop", true);
    betree_set_variable(event, 0, var);
    betree_search_with_event(tree, event, report);

    mu_assert(report->matched == 0, "");
    mu_assert(last_arg == (void *)456, "");
    mu_assert(last_data == (void *)555, "");
    mu_assert(last_result == true, "");
    mu_assert((betree_var_t)last_context == 0, "");

    free_report(report);
    betree_free_event(event);
    betree_free(tree);

    return 0;
}

static int test_excluded_branch_callback()
{
    struct betree* tree = betree_make_with_parameters(1, 0);

    betree_add_integer_variable(tree, "i", false, 0, 1);

    struct betree_sub* sub1 = betree_make_sub(tree, 1, 0, NULL, "i = 0");
    struct betree_sub* sub2 = betree_make_sub(tree, 2, 0, NULL, "i = 0");
    struct betree_sub* sub3 = betree_make_sub(tree, 3, 0, NULL, "i = 1");
    struct betree_sub* sub4 = betree_make_sub(tree, 4, 0, NULL, "i = 1");
    mu_assert(sub1 != NULL && sub2 != NULL && sub3 != NULL && sub4 != NULL, "");
    sub1->data = (void*)111;
    sub2->data = (void*)222;
    sub3->data = (void*)333;
    sub4->data = (void*)444;
    mu_assert(betree_insert_sub(tree, sub1), "");
    mu_assert(betree_insert_sub(tree, sub2), "");
    mu_assert(betree_insert_sub(tree, sub3), "");
    mu_assert(betree_insert_sub(tree, sub4), "");

    tree->subs_data = (struct subs_data*)bcalloc(sizeof(struct subs_data));
    mu_assert(tree->subs_data != NULL, "");
    tree->subs_data->array = (void**)bcalloc(4 * sizeof(void*));
    mu_assert(tree->subs_data->array != NULL, "");
    tree->subs_data->limit = 4;
    tree->subs_data->count = 0;
    betree_prepare_sub_data(tree);

    reset_hooks();
    struct report* report = make_report();
    report->cba = hook_excluded;
    report->arg = (void*)789;

    struct betree_event* event = betree_make_event(tree);
    betree_set_variable(event, 0, betree_make_integer_variable("i", 0));
    betree_search_with_event(tree, event, report);

    mu_assert(report->matched == 2, "");
    mu_assert(report->subs[0] == 1 && report->subs[1] == 2, "");
    mu_assert(last_excluded_arg == (void*)789, "");
    mu_assert(last_excluded_count == 2, "");
    mu_assert(
        (last_excluded_data[0] == (void*)333 && last_excluded_data[1] == (void*)444)
            || (last_excluded_data[0] == (void*)444 && last_excluded_data[1] == (void*)333),
        "");
    mu_assert((betree_var_t)last_excluded_context == 0, "");

    free_report(report);
    betree_free_event(event);
    bfree(tree->subs_data->array);
    bfree(tree->subs_data);
    tree->subs_data = NULL;
    betree_free(tree);

    return 0;
}

static int test_callback_set_contract()
{
    struct betree* tree = betree_make();

    betree_add_boolean_variable(tree, "stop", false);
    betree_add_integer_variable(tree, "i", false, 0, 1);

    struct betree_sub* sub1 = betree_make_sub(tree, 1, 0, NULL, "stop");
    struct betree_sub* sub2 = betree_make_sub(tree, 2, 0, NULL, "i = 1");
    struct betree_sub* sub3 = betree_make_sub(tree, 3, 0, NULL, "stop and i = 0");
    mu_assert(sub1 != NULL && sub2 != NULL && sub3 != NULL, "");
    sub1->data = (void*)101;
    sub2->data = (void*)202;
    sub3->data = (void*)303;
    mu_assert(betree_insert_sub(tree, sub1), "");
    mu_assert(betree_insert_sub(tree, sub2), "");
    mu_assert(betree_insert_sub(tree, sub3), "");

    reset_hooks();
    struct report* report = make_report();
    report->cb = hook;
    report->arg = (void*)999;

    struct betree_event* event = betree_make_event(tree);
    betree_set_variable(event, 0, betree_make_boolean_variable("stop", true));
    betree_set_variable(event, 1, betree_make_integer_variable("i", 1));
    betree_search_with_event(tree, event, report);

    mu_assert(report->matched == 0, "");
    mu_assert(seen_count == 3, "");
    mu_assert(seen_contains((void*)101, true, 0), "");
    mu_assert(seen_contains((void*)202, true, 1), "");
    mu_assert(seen_contains((void*)303, false, 1), "");
    mu_assert(last_arg == (void*)999, "");
    mu_assert(last_excluded_count == 0, "");

    free_report(report);
    betree_free_event(event);
    betree_free(tree);
    return 0;
}

static int test_excluded_callback_not_called_without_prune()
{
    struct betree* tree = betree_make();

    betree_add_integer_variable(tree, "i", false, 0, 2);
    mu_assert(betree_insert(tree, 1, "i = 1"), "");
    mu_assert(betree_insert(tree, 2, "i = 2"), "");

    reset_hooks();
    struct report* report = make_report();
    report->cba = hook_excluded;
    report->arg = (void*)321;

    struct betree_event* event = betree_make_event(tree);
    betree_set_variable(event, 0, betree_make_integer_variable("i", 1));
    betree_search_with_event(tree, event, report);

    mu_assert(report->matched == 1, "");
    mu_assert(report->subs[0] == 1, "");
    mu_assert(last_excluded_arg == NULL, "");
    mu_assert(last_excluded_count == 0, "");
    mu_assert(last_excluded_context == NULL, "");

    free_report(report);
    betree_free_event(event);
    betree_free(tree);
    return 0;
}

static int test_short_circuit_context_beyond_first_word()
{
    struct betree* tree = betree_make();
    char name[16];
    size_t i;

    for(i = 0; i <= 64; i++) {
        snprintf(name, sizeof(name), "v%zu", i);
        betree_add_boolean_variable(tree, name, true);
    }

    struct betree_sub* fail_sub = betree_make_sub(tree, 1, 0, NULL, "v64");
    struct betree_sub* pass_sub = betree_make_sub(tree, 2, 0, NULL, "not v64");
    mu_assert(fail_sub != NULL && pass_sub != NULL, "high-index subscriptions");
    fail_sub->data = (void*)640;
    pass_sub->data = (void*)641;
    mu_assert(betree_insert_sub(tree, fail_sub), "insert high-index fail subscription");
    mu_assert(betree_insert_sub(tree, pass_sub), "insert high-index pass subscription");

    reset_hooks();
    struct report* report = make_report();
    report->cb = hook;
    struct betree_event* event = betree_make_event(tree);
    mu_assert(betree_search_with_event(tree, event, report), "search empty event");

    mu_assert(seen_count == 2, "two callbacks");
    mu_assert(seen_contains((void*)640, false, 64), "fail-mask word offset included");
    mu_assert(seen_contains((void*)641, true, 64), "pass-mask word offset included");

    free_report(report);
    betree_free_event(event);
    betree_free(tree);
    return 0;
}

static int test_memoized_context_replay()
{
    struct betree* tree = betree_make();
    betree_add_boolean_variable(tree, "a", false);
    betree_add_boolean_variable(tree, "b", false);

    struct betree_sub* sub1 = betree_make_sub(tree, 1, 0, NULL, "a");
    struct betree_sub* sub2 = betree_make_sub(tree, 2, 0, NULL, "b and a");
    mu_assert(sub1 != NULL && sub2 != NULL, "memoized subscriptions");
    sub1->data = (void*)1001;
    sub2->data = (void*)1002;
    mu_assert(betree_insert_sub(tree, sub1), "insert first memoized subscription");
    mu_assert(betree_insert_sub(tree, sub2), "insert second memoized subscription");

    reset_hooks();
    struct report* report = make_report();
    report->cb = hook;
    struct betree_event* event = betree_make_event(tree);
    betree_set_variable(event, 0, betree_make_boolean_variable("a", true));
    betree_set_variable(event, 1, betree_make_boolean_variable("b", true));
    mu_assert(betree_search_with_event(tree, event, report), "memoized callback search");

    mu_assert(report->memoized > 0, "shared predicate memoized");
    mu_assert(seen_contains((void*)1001, true, 0), "first result attributed to a");
    mu_assert(seen_contains((void*)1002, true, 0), "memoized result restores a attribution");
    mu_assert(report->memoize_vars == NULL, "per-search attribution storage released");

    free_report(report);
    betree_free_event(event);

    reset_hooks();
    report = make_report();
    report->cb = hook;
    event = betree_make_event(tree);
    betree_set_variable(event, 0, betree_make_boolean_variable("a", false));
    betree_set_variable(event, 1, betree_make_boolean_variable("b", true));
    mu_assert(betree_search_with_event(tree, event, report), "memoized fail callback search");

    mu_assert(report->memoized > 0, "shared failing predicate memoized");
    mu_assert(seen_contains((void*)1001, false, 0), "first failure attributed to a");
    mu_assert(seen_contains((void*)1002, false, 0), "memoized failure restores a attribution");
    mu_assert(report->memoize_vars == NULL, "failing search attribution storage released");

    free_report(report);
    betree_free_event(event);
    betree_free(tree);
    return 0;
}

static int all_tests()
{
    mu_run_test(test_one);
    mu_run_test(test_two);
    mu_run_test(test_excluded_branch_callback);
    mu_run_test(test_callback_set_contract);
    mu_run_test(test_excluded_callback_not_called_without_prune);
    mu_run_test(test_short_circuit_context_beyond_first_word);
    mu_run_test(test_memoized_context_replay);
    return 0;
}

RUN_TESTS()
