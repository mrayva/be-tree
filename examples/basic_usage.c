/*
 * Basic BE-Tree Usage Example (C API)
 *
 * This example demonstrates:
 * - Creating a BE-tree
 * - Defining variable domains
 * - Inserting subscription expressions
 * - Matching events against subscriptions
 * - Cleaning up resources
 */

#include <stdio.h>
#include <stdlib.h>
#include "betree.h"

int main(void)
{
    printf("BE-Tree Basic Usage Example\n");
    printf("===========================\n\n");

    // 1. Create a new BE-tree
    struct betree* tree = betree_make();
    if (!tree) {
        fprintf(stderr, "Failed to create BE-tree\n");
        return 1;
    }

    // 2. Define variable domains (schema)
    printf("Defining variable domains:\n");
    betree_add_integer_variable(tree, "age", false, 0, 150);
    betree_add_string_variable(tree, "country", false, 100);
    betree_add_boolean_variable(tree, "premium", false);
    printf("  - age: integer [0-150]\n");
    printf("  - country: string (max 100 values)\n");
    printf("  - premium: boolean\n\n");

    // 3. Insert subscription expressions
    printf("Inserting subscriptions:\n");

    // Subscription 1: Target users aged 18-25 in USA
    if (betree_insert(tree, 1, "age >= 18 and age <= 25 and country = \"USA\"")) {
        printf("  ✓ Sub 1: Young adults in USA\n");
    }

    // Subscription 2: Premium users from any country
    if (betree_insert(tree, 2, "premium")) {
        printf("  ✓ Sub 2: Premium users\n");
    }

    // Subscription 3: Adults aged 30+ in Canada or UK
    if (betree_insert(tree, 3, "age >= 30 and (country = \"Canada\" or country = \"UK\")")) {
        printf("  ✓ Sub 3: Adults in Canada/UK\n");
    }

    printf("\n");

    // 4. Create events and match against subscriptions
    printf("Matching events:\n");

    // Event 1: Young premium user from USA
    printf("\nEvent 1: {\"age\": 22, \"country\": \"USA\", \"premium\": true}\n");
    struct report* report1 = make_report();
    if (betree_search(tree, "{\"age\": 22, \"country\": \"USA\", \"premium\": true}", report1)) {
        printf("  Matched %zu subscription(s):", report1->matched);
        for (size_t i = 0; i < report1->matched; i++) {
            printf(" %lu", report1->subs[i]);
        }
        printf("\n");
        printf("  Stats: evaluated=%zu, memoized=%zu, shorted=%zu\n",
               report1->evaluated, report1->memoized, report1->shorted);
    }
    free_report(report1);

    // Event 2: Non-premium adult from Canada
    printf("\nEvent 2: {\"age\": 35, \"country\": \"Canada\", \"premium\": false}\n");
    struct report* report2 = make_report();
    if (betree_search(tree, "{\"age\": 35, \"country\": \"Canada\", \"premium\": false}", report2)) {
        printf("  Matched %zu subscription(s):", report2->matched);
        for (size_t i = 0; i < report2->matched; i++) {
            printf(" %lu", report2->subs[i]);
        }
        printf("\n");
        printf("  Stats: evaluated=%zu, memoized=%zu, shorted=%zu\n",
               report2->evaluated, report2->memoized, report2->shorted);
    }
    free_report(report2);

    // Event 3: Premium child (no age match)
    printf("\nEvent 3: {\"age\": 12, \"country\": \"USA\", \"premium\": true}\n");
    struct report* report3 = make_report();
    if (betree_search(tree, "{\"age\": 12, \"country\": \"USA\", \"premium\": true}", report3)) {
        printf("  Matched %zu subscription(s):", report3->matched);
        for (size_t i = 0; i < report3->matched; i++) {
            printf(" %lu", report3->subs[i]);
        }
        printf("\n");
        printf("  Stats: evaluated=%zu, memoized=%zu, shorted=%zu\n",
               report3->evaluated, report3->memoized, report3->shorted);
    }

    // 5. Clean up
    free_report(report3);
    betree_free(tree);

    printf("\nCleanup complete.\n");
    return 0;
}
