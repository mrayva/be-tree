#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "../src/betree.h"

static void print_matches(const struct report* report)
{
    printf("Matched: %zu\n", report->matched);
    if (report->matched == 0) {
        return;
    }

    printf("Matched ids:");
    for (size_t i = 0; i < report->matched; ++i) {
        printf(" %" PRIu64, report->subs[i]);
    }
    printf("\n");
}

int main(void)
{
    puts("BE-Tree Filtered Search Example");
    puts("===============================");

    struct betree* tree = betree_make();
    betree_add_integer_variable(tree, "age", false, 0, 120);
    betree_add_string_variable(tree, "country", false, 16);

    betree_insert(tree, 1, "age >= 18");
    betree_insert(tree, 2, "country = \"USA\"");
    betree_insert(tree, 3, "age >= 30 and country = \"CAN\"");
    betree_insert(tree, 5, "age >= 18 and country = \"USA\"");

    const uint64_t sorted_ids[] = {1, 3, 5};
    const char* event = "{\"age\": 25, \"country\": \"USA\"}";
    struct report* report = make_report();

    if (!betree_search_ids(tree, event, report, sorted_ids, 3)) {
        puts("Filtered search failed");
        free_report(report);
        betree_free(tree);
        return 1;
    }

    printf("Event: %s\n", event);
    printf("Sorted filter ids: [%" PRIu64 ", %" PRIu64 ", %" PRIu64 "]\n",
        sorted_ids[0], sorted_ids[1], sorted_ids[2]);
    print_matches(report);

    puts("Note: filtered id arrays are expected to be sorted ascending.");

    free_report(report);
    betree_free(tree);
    return 0;
}
