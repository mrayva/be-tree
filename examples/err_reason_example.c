#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>

#include "../src/betree_err.h"

static void print_reason_report(const struct report_err* report)
{
    printf("Matched: %zu\n", report->matched);
    if (report->matched > 0) {
        printf("Matched ids:");
        for (size_t i = 0; i < report->matched; ++i) {
            printf(" %" PRIu64, report->subs[i]);
        }
        printf("\n");
    }

    puts("Reasons:");
    for (size_t i = 0; i < report->reason_sub_id_list->size; ++i) {
        struct betree_reason_t* reason = report->reason_sub_id_list->reasons[i];
        if (reason == NULL || reason->list == NULL || reason->list->size == 0) {
            continue;
        }

        printf("  %s:", reason->name);
        for (size_t j = 0; j < reason->list->size; ++j) {
            printf(" %" PRIu64, (uint64_t)reason->list->data[j]);
        }
        printf("\n");
    }
}

int main(void)
{
    puts("BE-Tree _err Reason Reporting Example");
    puts("=====================================");

    struct betree_err* tree = betree_make_err();
    betree_add_boolean_variable_err(tree, "premium", false);
    betree_add_integer_variable_err(tree, "age", false, 0, 120);
    betree_add_string_variable_err(tree, "country", false, 16);

    betree_insert_err(tree, 1, "premium and age >= 21");
    betree_insert_err(tree, 2, "country = \"USA\"");
    betree_insert_err(tree, 3, "premium and country = \"CAN\"");
    betree_make_sub_ids(tree);

    struct report_err* report = make_report_err(tree);
    const char* event = "{\"premium\": false, \"age\": 19, \"country\": \"MEX\"}";

    if (!betree_search_err(tree, event, report)) {
        puts("Search failed");
        free_report_err(report);
        betree_free_err(tree);
        return 1;
    }

    printf("Event: %s\n", event);
    print_reason_report(report);

    free_report_err(report);
    betree_free_err(tree);
    return 0;
}
