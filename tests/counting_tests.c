#include <stdbool.h>
#include <stdio.h>

#include "alloc.h"
#include "betree.h"
#include "hashmap.h"
#include "minunit.h"
#include "tree.h"

static struct report_counting* search_counting(
    const struct betree* tree, const char* event_str)
{
    struct betree_event* event = make_event_from_string(tree, event_str);
    const struct betree_variable** preds
        = make_environment(tree->config->attr_domain_count, event);
    uint64_t* undefined = make_undefined(tree->config->attr_domain_count, preds);
    struct memoize memoize = make_memoize(tree->config->pred_map->memoize_count);
    struct subs_to_eval subs;
    struct report_counting* report = make_report_counting();

    init_subs_to_eval(&subs);
    match_be_tree_node_counting(
        (const struct attr_domain**)tree->config->attr_domains,
        preds,
        tree->cnode,
        &subs,
        &report->node_count);

    for(size_t i = 0; i < subs.count; i++) {
        const struct betree_sub* sub = subs.subs[i];
        report->evaluated++;
        if(match_sub_counting(
               tree->config->attr_domain_count, preds, sub, report, &memoize, undefined)) {
            add_sub_counting(sub->id, report);
        }
    }

    bfree(subs.subs);
    free_memoize(memoize);
    bfree(undefined);
    bfree(preds);
    free_event(event);
    return report;
}

int test_counting_memoized_match()
{
    struct betree* tree = betree_make();
    add_attr_domain_bounded_i(tree->config, "i", false, 0, 10);

    mu_assert(betree_insert(tree, 1, "i = 1"), "");
    mu_assert(betree_insert(tree, 2, "i = 1"), "");

    struct report_counting* report = search_counting(tree, "{\"i\": 1}");

    mu_assert(report->evaluated == 2, "evaluated");
    mu_assert(report->matched == 2, "matched");
    mu_assert(report->memoized == 1, "memoized");
    mu_assert(report->shorted == 0, "shorted");
    mu_assert(report->node_count == 4, "node_count");
    mu_assert(report->ops_count == 1, "ops_count");
    mu_assert(report->subs[0] == 1 && report->subs[1] == 2, "matchedSubs");

    free_report_counting(report);
    betree_free(tree);
    return 0;
}

int test_counting_short_circuit_fail()
{
    struct betree* tree = betree_make();
    add_attr_domain_i(tree->config, "i", true);

    mu_assert(betree_insert(tree, 1, "i = 1"), "");

    struct report_counting* report = search_counting(tree, "{}");

    mu_assert(report->evaluated == 1, "evaluated");
    mu_assert(report->matched == 0, "matched");
    mu_assert(report->memoized == 0, "memoized");
    mu_assert(report->shorted == 1, "shorted");
    mu_assert(report->node_count == 1, "node_count");
    mu_assert(report->ops_count == 0, "ops_count");

    free_report_counting(report);
    betree_free(tree);
    return 0;
}

int all_tests()
{
    mu_run_test(test_counting_memoized_match);
    mu_run_test(test_counting_short_circuit_fail);
    return 0;
}

RUN_TESTS()
