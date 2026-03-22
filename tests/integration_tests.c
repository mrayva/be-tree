#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "betree.h"
#include "betree_err.h"
#include "dyn_arr.h"
#include "minunit.h"
#include "tree.h"

static int report_has_id_set(const struct report* report, const uint64_t* expected, size_t count)
{
    mu_assert(report->matched == count, "matchedCount");
    for(size_t i = 0; i < count; ++i) {
        bool found = false;
        for(size_t j = 0; j < report->matched; ++j) {
            if(report->subs[j] == expected[i]) {
                found = true;
                break;
            }
        }
        mu_assert(found, "matchedId");
    }
    return 0;
}

static int reason_has_id_set(dynamic_array_t* reason, const uint64_t* expected, size_t count)
{
    mu_assert(reason != NULL, "reasonExists");
    mu_assert(reason->size == count, "reasonCount");
    for(size_t i = 0; i < count; ++i) {
        bool found = false;
        for(size_t j = 0; j < reason->size; ++j) {
            if((uint64_t)reason->data[j] == expected[i]) {
                found = true;
                break;
            }
        }
        mu_assert(found, "reasonId");
    }
    return 0;
}

static int test_medium_partitioned_search_flow()
{
    struct betree* tree = betree_make_with_parameters(1, 0);

    betree_add_integer_variable(tree, "age", false, 0, 100);
    betree_add_string_variable(tree, "country", false, 8);
    betree_add_boolean_variable(tree, "premium", false);
    betree_add_integer_enum_variable(tree, "status", false, 8);

    mu_assert(betree_insert(tree, 1, "age >= 21 and country = \"USA\""), "");
    mu_assert(betree_insert(tree, 2, "age >= 21 and premium"), "");
    mu_assert(betree_insert(tree, 3, "country = \"CAN\" and status = 2"), "");
    mu_assert(betree_insert(tree, 4, "age < 18"), "");
    mu_assert(betree_insert(tree, 5, "premium and status = 1"), "");
    mu_assert(betree_insert(tree, 6, "country = \"USA\" and status = 1"), "");
    mu_assert(betree_insert(tree, 7, "age >= 21 and country = \"USA\" and premium"), "");
    mu_assert(betree_insert(tree, 8, "status = 3"), "");

    mu_assert(tree->cnode->pdir != NULL || tree->cnode->lnode->sub_count < 8, "treeSplit");

    struct report* report = make_report();
    mu_assert(
        betree_search(tree, "{\"age\": 30, \"country\": \"USA\", \"premium\": true, \"status\": 1}", report),
        "");
    {
        const uint64_t expected[] = {1, 2, 5, 6, 7};
        mu_assert(report_has_id_set(report, expected, 5) == 0, "");
    }

    const uint64_t ids[] = {2, 4, 7, 8};
    struct report* filtered = make_report();
    mu_assert(
        betree_search_ids(
            tree, "{\"age\": 30, \"country\": \"USA\", \"premium\": true, \"status\": 1}", filtered, ids, 4),
        "");
    {
        const uint64_t expected[] = {2, 7};
        mu_assert(report_has_id_set(filtered, expected, 2) == 0, "");
    }

    free_report(filtered);
    free_report(report);
    betree_free(tree);
    return 0;
}

static int test_medium_partitioned_reason_flow()
{
    struct betree_err* tree = betree_make_with_parameters_err(1, 0);

    betree_add_integer_variable_err(tree, "age", false, 0, 100);
    betree_add_string_variable_err(tree, "country", false, 8);
    betree_add_boolean_variable_err(tree, "premium", false);
    betree_add_integer_enum_variable_err(tree, "status", false, 8);

    mu_assert(betree_insert_err(tree, 1, "premium and age >= 21"), "");
    mu_assert(betree_insert_err(tree, 2, "country = \"USA\" and status = 1"), "");
    mu_assert(betree_insert_err(tree, 3, "country = \"CAN\" and status = 2"), "");
    mu_assert(betree_insert_err(tree, 4, "age < 18"), "");
    mu_assert(betree_insert_err(tree, 5, "premium and country = \"USA\" and status = 2"), "");
    mu_assert(betree_insert_err(tree, 6, "premium and country = \"USA\" and status = 3"), "");
    mu_assert(betree_insert_err(tree, 7, "status = 2 and age >= 25"), "");
    mu_assert(betree_insert_err(tree, 8, "country = \"MEX\""), "");

    betree_make_sub_ids(tree);

    struct report_err* report = make_report_err(tree);
    mu_assert(
        betree_search_err(
            tree, "{\"age\": 30, \"country\": \"USA\", \"premium\": true, \"status\": 2}", report),
        "");
    {
        const uint64_t expected[] = {1, 5, 7};
        mu_assert(report_has_id_set((const struct report*)report, expected, 3) == 0, "");
    }

    dynamic_array_t* status_reason = betree_reason_map_get(report->reason_sub_id_list, 3);
    {
        const uint64_t expected[] = {2, 6};
        mu_assert(reason_has_id_set(status_reason, expected, 2) == 0, "");
    }

    dynamic_array_t* country_reason = betree_reason_map_get(report->reason_sub_id_list, 1);
    {
        const uint64_t expected[] = {3, 8};
        mu_assert(reason_has_id_set(country_reason, expected, 2) == 0, "");
    }

    dynamic_array_t* age_reason = betree_reason_map_get(report->reason_sub_id_list, 0);
    {
        const uint64_t expected[] = {4};
        mu_assert(reason_has_id_set(age_reason, expected, 1) == 0, "");
    }

    free_report_err(report);
    betree_free_err(tree);
    return 0;
}

int all_tests()
{
    mu_run_test(test_medium_partitioned_search_flow);
    mu_run_test(test_medium_partitioned_reason_flow);
    return 0;
}

RUN_TESTS()
