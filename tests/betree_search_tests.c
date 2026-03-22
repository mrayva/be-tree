#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "alloc.h"
#include "betree.h"
#include "debug.h"
#include "helper.h"
#include "minunit.h"
#include "printer.h"
#include "tree.h"
#include "utils.h"

static int assert_report_ids(const struct report* report, const uint64_t* expected, size_t count)
{
    mu_assert(report->matched == count, "matchedCount");
    for(size_t i = 0; i < count; ++i) {
        mu_assert(report->subs[i] == expected[i], "matchedId");
    }
    return 0;
}

static int assert_report_id_set(const struct report* report, const uint64_t* expected, size_t count)
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
        mu_assert(found, "matchedIdSet");
    }
    return 0;
}

static int assert_reports_equal(const struct report* lhs, const struct report* rhs)
{
    mu_assert(lhs->matched == rhs->matched, "matchedParity");
    for(size_t i = 0; i < lhs->matched; ++i) {
        mu_assert(lhs->subs[i] == rhs->subs[i], "matchedIdParity");
    }
    return 0;
}

static int assert_report_sets_equal(const struct report* lhs, const struct report* rhs)
{
    mu_assert(lhs->matched == rhs->matched, "matchedParity");
    for(size_t i = 0; i < lhs->matched; ++i) {
        bool found = false;
        for(size_t j = 0; j < rhs->matched; ++j) {
            if(lhs->subs[i] == rhs->subs[j]) {
                found = true;
                break;
            }
        }
        mu_assert(found, "matchedIdSetParity");
    }
    return 0;
}

int test_search()
{
    struct betree* tree = betree_make();
    add_attr_domain_bounded_i(tree->config, "a", false, 0, 0);
    add_attr_domain_bounded_i(tree->config, "b", false, 0, 1);

    mu_assert(betree_insert(tree, 1, "a = 0 and b = 0"), "");
    mu_assert(betree_insert(tree, 2, "a = 1"), "");
    mu_assert(betree_insert(tree, 3, "a = 0 and b = 0"), "");
    mu_assert(betree_insert(tree, 4, "a = 0 and b = 1"), "");
    mu_assert(betree_insert(tree, 5, "b = 1 and a = 0"), "");

    struct report* report = make_report();
    mu_assert(betree_search(tree, "{\"a\": 0, \"b\": 1}", report), "");
    mu_assert(report->matched == 2 && report->subs[0] == 4 && report->subs[1] == 5, "goodEvent");

    write_dot_to_file(tree, "tests/beetree_search_tests.dot");

    free_report(report);
    betree_free(tree);
    return 0;
}

int test_search_ids_0()
{
    struct betree* tree = betree_make();
    add_attr_domain_bounded_i(tree->config, "a", false, 0, 0);
    add_attr_domain_bounded_i(tree->config, "b", false, 0, 1);

    mu_assert(betree_insert(tree, 1, "a = 0 and b = 0"), "");
    mu_assert(betree_insert(tree, 2, "a = 1"), "");
    mu_assert(betree_insert(tree, 3, "a = 0 and b = 0"), "");
    mu_assert(betree_insert(tree, 4, "a = 0 and b = 1"), "");
    mu_assert(betree_insert(tree, 5, "b = 1 and a = 0"), "");

    struct report* report = make_report();
    uint64_t ids[] = {1, 2, 3, 4, 5};
    size_t sz = 5;
    mu_assert(betree_search_ids(tree, "{\"a\": 0, \"b\": 1}", report, ids, sz), "");

    free_report(report);
    betree_free(tree);
    return 0;
}

int test_search_ids_1()
{
    struct betree* tree = betree_make();
    add_attr_domain_bounded_i(tree->config, "a", false, 0, 0);
    add_attr_domain_bounded_i(tree->config, "b", false, 0, 1);

    mu_assert(betree_insert(tree, 1, "a = 0 and b = 0"), "");
    mu_assert(betree_insert(tree, 2, "a = 1"), "");
    mu_assert(betree_insert(tree, 3, "a = 0 and b = 0"), "");
    mu_assert(betree_insert(tree, 4, "a = 0 and b = 1"), "");
    mu_assert(betree_insert(tree, 5, "b = 1 and a = 0"), "");

    struct report* report = make_report();
    uint64_t ids[] = {1, 4};
    size_t sz = 2;
    mu_assert(betree_search_ids(tree, "{\"a\": 0, \"b\": 1}", report, ids, sz), "");
    mu_assert(report->matched == 1 && report->subs[0] == 4, "goodEvent");

    free_report(report);
    return 0;
}

int test_search_ids_2()
{
    struct betree* tree = betree_make();
    add_attr_domain_bounded_i(tree->config, "a", false, 0, 0);
    add_attr_domain_bounded_i(tree->config, "b", false, 0, 1);

    mu_assert(betree_insert(tree, 1, "a = 0 and b = 0"), "");
    mu_assert(betree_insert(tree, 2, "a = 1"), "");
    mu_assert(betree_insert(tree, 3, "a = 0 and b = 0"), "");
    mu_assert(betree_insert(tree, 4, "a = 0 and b = 1"), "");
    mu_assert(betree_insert(tree, 5, "b = 1 and a = 0"), "");

    struct report* report = make_report();
    uint64_t ids[] = {1, 4, 5};
    size_t sz = 3;
    mu_assert(betree_search_ids(tree, "{\"a\": 0, \"b\": 1}", report, ids, sz), "");
    mu_assert(report->matched == 2 && report->subs[0] == 4 && report->subs[1] == 5, "goodEvent");

    free_report(report);
    return 0;
}

int test_search_ids_3()
{
    struct betree* tree = betree_make();
    add_attr_domain_bounded_i(tree->config, "a", false, 0, 0);
    add_attr_domain_bounded_i(tree->config, "b", false, 0, 1);

    mu_assert(betree_insert(tree, 1, "a = 0 and b = 0"), "");
    mu_assert(betree_insert(tree, 2, "a = 1"), "");
    mu_assert(betree_insert(tree, 3, "a = 0 and b = 0"), "");
    mu_assert(betree_insert(tree, 4, "a = 0 and b = 1"), "");
    mu_assert(betree_insert(tree, 5, "b = 1 and a = 0"), "");

    struct report* report = make_report();
    uint64_t ids[] = {1, 3};
    size_t sz = 2;
    mu_assert(betree_search_ids(tree, "{\"a\": 0, \"b\": 1}", report, ids, sz), "");
    mu_assert(report->matched == 0, "goodEvent");

    free_report(report);
    return 0;
}

int test_search_ids_4()
{
    struct betree* tree = betree_make();
    add_attr_domain_bounded_i(tree->config, "a", false, 0, 0);
    add_attr_domain_bounded_i(tree->config, "b", false, 0, 1);

    mu_assert(betree_insert(tree, 1, "a = 0 and b = 0"), "");
    mu_assert(betree_insert(tree, 2, "a = 1"), "");
    mu_assert(betree_insert(tree, 3, "a = 0 and b = 0"), "");
    mu_assert(betree_insert(tree, 4, "a = 0 and b = 1"), "");
    mu_assert(betree_insert(tree, 5, "b = 1 and a = 0"), "");

    struct report* report = make_report();
    uint64_t ids[] = {};
    size_t sz = 0;
    mu_assert(betree_search_ids(tree, "{\"a\": 0, \"b\": 1}", report, ids, sz), "");
    mu_assert(report->matched == 0, "goodEvent");

    free_report(report);
    return 0;
}

int test_search_ids_filtered_existence_semantics()
{
    struct betree* tree = betree_make();
    add_attr_domain_bounded_i(tree->config, "a", false, 0, 10);
    add_attr_domain_bounded_i(tree->config, "b", false, 0, 10);

    mu_assert(betree_insert(tree, 1, "a = 1"), "");
    mu_assert(betree_insert(tree, 2, "b = 2"), "");
    mu_assert(betree_insert(tree, 3, "a = 1 and b = 2"), "");
    mu_assert(betree_insert(tree, 4, "a = 2"), "");

    const char* event = "{\"a\": 1, \"b\": 2}";

    struct report* one_match_report = make_report();
    const uint64_t one_match_ids[] = {3, 4};
    mu_assert(betree_search_ids(tree, event, one_match_report, one_match_ids, 2), "");
    mu_assert(one_match_report->matched > 0, "filteredExists");
    {
        const uint64_t expected[] = {3};
        mu_assert(assert_report_id_set(one_match_report, expected, 1) == 0, "");
    }

    struct report* no_match_report = make_report();
    const uint64_t no_match_ids[] = {4};
    mu_assert(betree_search_ids(tree, event, no_match_report, no_match_ids, 1), "");
    mu_assert(no_match_report->matched == 0, "filteredNoMatch");

    struct report* missing_ids_report = make_report();
    const uint64_t missing_ids[] = {99, 100};
    mu_assert(betree_search_ids(tree, event, missing_ids_report, missing_ids, 2), "");
    mu_assert(missing_ids_report->matched == 0, "filteredMissingIds");

    free_report(missing_ids_report);
    free_report(no_match_report);
    free_report(one_match_report);
    betree_free(tree);
    return 0;
}

int test_search_empty_lists()
{
    struct betree* tree = betree_make();
    add_attr_domain_il(tree->config, "il", false);
    add_attr_domain_sl(tree->config, "sl", false);

    mu_assert(betree_insert(tree, 1, "il is empty"), "");
    mu_assert(betree_insert(tree, 2, "sl is empty"), "");
    mu_assert(betree_insert(tree, 3, "il one of (1)"), "");

    struct report* report = make_report();
    mu_assert(betree_search(tree, "{\"il\": [], \"sl\": []}", report), "");
    mu_assert(report->matched == 2 && report->subs[0] == 1 && report->subs[1] == 2, "goodEvent");

    free_report(report);
    betree_free(tree);
    return 0;
}

int test_search_integer_enum_event_normalization()
{
    struct betree* tree = betree_make();
    add_attr_domain_bounded_ie(tree->config, "ie", false, 8);

    mu_assert(betree_insert(tree, 1, "ie = 2"), "");
    mu_assert(betree_insert(tree, 2, "ie = 3"), "");

    struct report* report = make_report();
    mu_assert(betree_search(tree, "{\"ie\": 2}", report), "");
    mu_assert(report->matched == 1 && report->subs[0] == 1, "goodEvent");

    free_report(report);
    betree_free(tree);
    return 0;
}

int test_search_allow_undefined_by_domain()
{
    struct betree* tree = betree_make();
    add_attr_domain_i(tree->config, "i", true);
    add_attr_domain_s(tree->config, "s", true);
    add_attr_domain_il(tree->config, "il", true);
    add_attr_domain_sl(tree->config, "sl", true);
    add_attr_domain_ie(tree->config, "ie", true);

    mu_assert(betree_insert(tree, 1, "i is null"), "");
    mu_assert(betree_insert(tree, 2, "s is null"), "");
    mu_assert(betree_insert(tree, 3, "il is null"), "");
    mu_assert(betree_insert(tree, 4, "sl is null"), "");
    mu_assert(betree_insert(tree, 5, "ie is null"), "");
    mu_assert(betree_insert(tree, 6, "il is empty"), "");
    mu_assert(betree_insert(tree, 7, "sl is empty"), "");

    struct report* report = make_report();
    mu_assert(betree_search(tree, "{}", report), "");
    mu_assert(report->matched == 5, "goodMatchCount");
    mu_assert(report->subs[0] == 1 && report->subs[1] == 2 && report->subs[2] == 3
            && report->subs[3] == 4 && report->subs[4] == 5,
        "goodMatchedSubs");

    free_report(report);
    betree_free(tree);
    return 0;
}

int test_search_disallow_undefined_validation()
{
    struct betree* tree = betree_make();
    add_attr_domain_i(tree->config, "i", false);
    add_attr_domain_il(tree->config, "il", false);

    mu_assert(betree_insert(tree, 1, "i = 1"), "");
    mu_assert(betree_insert(tree, 2, "il is empty"), "");

    struct report* report = make_report();
    mu_assert(!betree_search(tree, "{}", report), "");
    mu_assert(report->matched == 0, "noMatches");

    free_report(report);
    betree_free(tree);
    return 0;
}

int test_search_with_event_parity()
{
    struct betree* tree = betree_make();
    add_attr_domain_b(tree->config, "flag", false);
    add_attr_domain_bounded_i(tree->config, "age", false, 0, 120);
    add_attr_domain_s(tree->config, "country", false);

    mu_assert(betree_insert(tree, 1, "flag and age >= 21"), "");
    mu_assert(betree_insert(tree, 2, "country = \"USA\""), "");
    mu_assert(betree_insert(tree, 3, "flag and country = \"CAN\""), "");

    struct report* json_report = make_report();
    mu_assert(betree_search(tree,
            "{\"flag\": true, \"age\": 25, \"country\": \"USA\"}",
            json_report),
        "");

    struct betree_event* event = betree_make_event(tree);
    betree_set_variable(event, 0, betree_make_boolean_variable("flag", true));
    betree_set_variable(event, 1, betree_make_integer_variable("age", 25));
    betree_set_variable(event, 2, betree_make_string_variable("country", "USA"));

    struct report* event_report = make_report();
    mu_assert(betree_search_with_event(tree, event, event_report), "");

    const uint64_t expected[] = {1, 2};
    mu_assert(assert_report_id_set(json_report, expected, 2) == 0, "");
    mu_assert(assert_report_sets_equal(json_report, event_report) == 0, "");

    free_report(event_report);
    betree_free_event(event);
    free_report(json_report);
    betree_free(tree);
    return 0;
}

int test_search_with_event_ids_parity()
{
    struct betree* tree = betree_make();
    add_attr_domain_bounded_i(tree->config, "a", false, 0, 10);
    add_attr_domain_bounded_i(tree->config, "b", false, 0, 10);

    mu_assert(betree_insert(tree, 1, "a = 1"), "");
    mu_assert(betree_insert(tree, 2, "b = 2"), "");
    mu_assert(betree_insert(tree, 3, "a = 1 and b = 2"), "");
    mu_assert(betree_insert(tree, 4, "a = 2"), "");

    const uint64_t ids[] = {1, 2, 3, 4};

    struct report* json_report = make_report();
    mu_assert(betree_search_ids(tree, "{\"a\": 1, \"b\": 2}", json_report, ids, 4), "");

    struct betree_event* event = betree_make_event(tree);
    betree_set_variable(event, 0, betree_make_integer_variable("a", 1));
    betree_set_variable(event, 1, betree_make_integer_variable("b", 2));

    struct report* event_report = make_report();
    mu_assert(betree_search_with_event_ids(tree, event, event_report, ids, 4), "");

    const uint64_t expected[] = {1, 2, 3};
    mu_assert(assert_report_id_set(json_report, expected, 3) == 0, "");
    mu_assert(assert_report_sets_equal(json_report, event_report) == 0, "");

    free_report(event_report);
    betree_free_event(event);
    free_report(json_report);
    betree_free(tree);
    return 0;
}

int test_exists_parity()
{
    struct betree* tree = betree_make();
    add_attr_domain_b(tree->config, "flag", false);
    add_attr_domain_bounded_i(tree->config, "age", false, 0, 120);
    add_attr_domain_s(tree->config, "country", false);

    mu_assert(betree_insert(tree, 1, "flag and age >= 21"), "");
    mu_assert(betree_insert(tree, 2, "country = \"USA\""), "");
    mu_assert(betree_insert(tree, 3, "flag and country = \"CAN\""), "");

    const char* match_event = "{\"flag\": true, \"age\": 25, \"country\": \"USA\"}";
    struct report* match_report = make_report();
    mu_assert(betree_search(tree, match_event, match_report), "");
    mu_assert(match_report->matched == 2, "");
    mu_assert(betree_exists(tree, match_event), "");

    struct betree_event* match_object = betree_make_event(tree);
    betree_set_variable(match_object, 0, betree_make_boolean_variable("flag", true));
    betree_set_variable(match_object, 1, betree_make_integer_variable("age", 25));
    betree_set_variable(match_object, 2, betree_make_string_variable("country", "USA"));
    mu_assert(betree_exists_with_event(tree, match_object), "");

    const char* miss_event = "{\"flag\": false, \"age\": 17, \"country\": \"MEX\"}";
    struct report* miss_report = make_report();
    mu_assert(betree_search(tree, miss_event, miss_report), "");
    mu_assert(miss_report->matched == 0, "");
    mu_assert(!betree_exists(tree, miss_event), "");

    struct betree_event* miss_object = betree_make_event(tree);
    betree_set_variable(miss_object, 0, betree_make_boolean_variable("flag", false));
    betree_set_variable(miss_object, 1, betree_make_integer_variable("age", 17));
    betree_set_variable(miss_object, 2, betree_make_string_variable("country", "MEX"));
    mu_assert(!betree_exists_with_event(tree, miss_object), "");

    betree_free_event(miss_object);
    free_report(miss_report);
    betree_free_event(match_object);
    free_report(match_report);
    betree_free(tree);
    return 0;
}

int test_search_with_event_invalid_event_rejection()
{
    struct betree* tree = betree_make();
    add_attr_domain_bounded_i(tree->config, "age", false, 0, 120);
    add_attr_domain_s(tree->config, "country", false);

    mu_assert(betree_insert(tree, 1, "age >= 21"), "");
    mu_assert(betree_insert(tree, 2, "country = \"USA\""), "");

    struct betree_event* invalid_event = betree_make_event(tree);
    betree_set_variable(invalid_event, 1, betree_make_string_variable("country", "USA"));

    struct report* report = make_report();
    mu_assert(!betree_search_with_event(tree, invalid_event, report), "");
    mu_assert(report->matched == 0, "");

    betree_free_event(invalid_event);
    free_report(report);
    betree_free(tree);
    return 0;
}

int test_search_with_event_ids_invalid_event_rejection()
{
    struct betree* tree = betree_make();
    add_attr_domain_bounded_i(tree->config, "age", false, 0, 120);
    add_attr_domain_s(tree->config, "country", false);

    mu_assert(betree_insert(tree, 1, "age >= 21"), "");
    mu_assert(betree_insert(tree, 2, "country = \"USA\""), "");
    mu_assert(betree_insert(tree, 3, "age >= 30 and country = \"USA\""), "");

    const uint64_t ids[] = {1, 3};

    struct betree_event* invalid_event = betree_make_event(tree);
    betree_set_variable(invalid_event, 1, betree_make_string_variable("country", "USA"));

    struct report* report = make_report();
    mu_assert(!betree_search_with_event_ids(tree, invalid_event, report, ids, 2), "");
    mu_assert(report->matched == 0, "");

    betree_free_event(invalid_event);
    free_report(report);
    betree_free(tree);
    return 0;
}

int test_exists_invalid_event_rejection()
{
    struct betree* tree = betree_make();
    add_attr_domain_bounded_i(tree->config, "age", false, 0, 120);
    add_attr_domain_s(tree->config, "country", false);

    mu_assert(betree_insert(tree, 1, "age >= 21"), "");
    mu_assert(betree_insert(tree, 2, "country = \"USA\""), "");

    mu_assert(!betree_exists(tree, "{\"country\": \"USA\"}"), "");

    struct betree_event* invalid_event = betree_make_event(tree);
    betree_set_variable(invalid_event, 1, betree_make_string_variable("country", "USA"));
    mu_assert(!betree_exists_with_event(tree, invalid_event), "");

    betree_free_event(invalid_event);
    betree_free(tree);
    return 0;
}

int test_search_with_event_type_mismatch_rejection()
{
    struct betree* tree = betree_make();
    add_attr_domain_bounded_i(tree->config, "age", false, 0, 120);
    add_attr_domain_s(tree->config, "country", false);

    mu_assert(betree_insert(tree, 1, "age >= 21"), "");
    mu_assert(betree_insert(tree, 2, "country = \"USA\""), "");

    struct betree_event* invalid_event = betree_make_event(tree);
    betree_set_variable(invalid_event, 0, betree_make_string_variable("age", "twenty five"));
    betree_set_variable(invalid_event, 1, betree_make_string_variable("country", "USA"));

    struct report* report = make_report();
    const uint64_t ids[] = {1, 2};

    mu_assert(!betree_search_with_event(tree, invalid_event, report), "");
    mu_assert(report->matched == 0, "");
    mu_assert(!betree_search_with_event_ids(tree, invalid_event, report, ids, 2), "");
    mu_assert(report->matched == 0, "");
    mu_assert(!betree_exists_with_event(tree, invalid_event), "");

    betree_free_event(invalid_event);
    free_report(report);
    betree_free(tree);
    return 0;
}

int test_search_with_event_list_type_mismatch_rejection()
{
    struct betree* tree = betree_make();
    add_attr_domain_il(tree->config, "ints", false);
    add_attr_domain_sl(tree->config, "strings", false);

    mu_assert(betree_insert(tree, 1, "1 in ints"), "");
    mu_assert(betree_insert(tree, 2, "\"x\" in strings"), "");

    struct betree_event* invalid_event = betree_make_event(tree);

    struct betree_string_list* wrong_strings = betree_make_string_list(1);
    betree_add_string(wrong_strings, 0, "1");
    betree_set_variable(invalid_event, 0,
        betree_make_string_list_variable("ints", wrong_strings));

    struct betree_integer_list* wrong_ints = betree_make_integer_list(1);
    betree_add_integer(wrong_ints, 0, 1);
    betree_set_variable(invalid_event, 1,
        betree_make_integer_list_variable("strings", wrong_ints));

    struct report* report = make_report();
    const uint64_t ids[] = {1, 2};

    mu_assert(!betree_search_with_event(tree, invalid_event, report), "");
    mu_assert(report->matched == 0, "");
    mu_assert(!betree_search_with_event_ids(tree, invalid_event, report, ids, 2), "");
    mu_assert(report->matched == 0, "");
    mu_assert(!betree_exists_with_event(tree, invalid_event), "");

    betree_free_event(invalid_event);
    free_report(report);
    betree_free(tree);
    return 0;
}

int test_search_with_event_variable_replacement_semantics()
{
    struct betree* tree = betree_make();
    add_attr_domain_bounded_i(tree->config, "age", false, 0, 120);

    mu_assert(betree_insert(tree, 1, "age = 25"), "");
    mu_assert(betree_insert(tree, 2, "age = 30"), "");

    struct betree_event* event = betree_make_event(tree);
    betree_set_variable(event, 0, betree_make_integer_variable("age", 25));
    betree_set_variable(event, 0, betree_make_integer_variable("age", 30));

    struct report* report = make_report();
    mu_assert(betree_search_with_event(tree, event, report), "");
    {
        const uint64_t expected[] = {2};
        mu_assert(assert_report_id_set(report, expected, 1) == 0, "");
    }
    mu_assert(betree_exists_with_event(tree, event), "");

    betree_free_event(event);
    free_report(report);
    betree_free(tree);
    return 0;
}

int test_search_with_event_slot_reset_semantics()
{
    struct betree* tree = betree_make();
    add_attr_domain_bounded_i(tree->config, "age", false, 0, 120);

    mu_assert(betree_insert(tree, 1, "age = 25"), "");
    mu_assert(betree_insert(tree, 2, "age = 30"), "");

    struct betree_event* event = betree_make_event(tree);
    betree_set_variable(event, 0, betree_make_integer_variable("age", 30));

    struct report* report = make_report();
    mu_assert(betree_search_with_event(tree, event, report), "");
    {
        const uint64_t expected[] = {2};
        mu_assert(assert_report_id_set(report, expected, 1) == 0, "");
    }

    betree_set_variable(event, 0, betree_make_integer_variable("age", 25));
    struct report* replaced_report = make_report();
    mu_assert(betree_search_with_event(tree, event, replaced_report), "");
    {
        const uint64_t expected[] = {1};
        mu_assert(assert_report_id_set(replaced_report, expected, 1) == 0, "");
    }
    mu_assert(betree_exists_with_event(tree, event), "");

    betree_free_event(event);
    free_report(replaced_report);
    free_report(report);
    betree_free(tree);
    return 0;
}

int test_search_with_event_special_domain_parity()
{
    struct betree* tree = betree_make();
    add_attr_domain_segments(tree->config, "seg", false);
    add_attr_domain_frequency(tree->config, "frequency_caps", false);
    add_attr_domain_i(tree->config, "now", false);

    const struct betree_constant* constants[] = {
        betree_make_integer_constant("flight_id", 10),
    };

    mu_assert(betree_insert_with_constants(tree,
            1,
            1,
            constants,
            "segment_within(seg, 1, 20) and within_frequency_cap(\"flight\", \"ns\", 100, 0)"),
        "");
    betree_free_constant((struct betree_constant*)constants[0]);

    struct report* json_report = make_report();
    mu_assert(betree_search(tree,
            "{\"now\": 0, \"seg\": [[1, 10000000]], "
            "\"frequency_caps\": [[\"flight\", 10, \"ns\", 0, 0]]}",
            json_report),
        "");

    struct betree_event* event = betree_make_event(tree);
    struct betree_segments* seg = betree_make_segments(1);
    betree_add_segment(seg, 0, betree_make_segment(1, 10000000));
    betree_set_variable(event, 0, betree_make_segments_variable("seg", seg));

    struct betree_frequency_caps* frequency_caps = betree_make_frequency_caps(1);
    betree_add_frequency_cap(
        frequency_caps, 0, betree_make_frequency_cap("flight", 10, "ns", false, 0, 0));
    betree_set_variable(
        event, 1, betree_make_frequency_caps_variable("frequency_caps", frequency_caps));
    betree_set_variable(event, 2, betree_make_integer_variable("now", 0));

    struct report* event_report = make_report();
    mu_assert(betree_search_with_event(tree, event, event_report), "");
    {
        const uint64_t expected[] = {1};
        mu_assert(assert_report_id_set(json_report, expected, 1) == 0, "");
        mu_assert(assert_report_sets_equal(json_report, event_report) == 0, "");
    }
    mu_assert(betree_exists_with_event(tree, event), "");

    free_report(event_report);
    betree_free_event(event);
    free_report(json_report);
    betree_free(tree);
    return 0;
}

int test_search_with_event_special_domain_type_mismatch_rejection()
{
    struct betree* tree = betree_make();
    add_attr_domain_segments(tree->config, "seg", false);
    add_attr_domain_frequency(tree->config, "frequency_caps", false);
    add_attr_domain_i(tree->config, "now", false);

    const struct betree_constant* constants[] = {
        betree_make_integer_constant("flight_id", 10),
    };

    mu_assert(betree_insert_with_constants(tree,
            1,
            1,
            constants,
            "segment_within(seg, 1, 20) and within_frequency_cap(\"flight\", \"ns\", 100, 0)"),
        "");
    betree_free_constant((struct betree_constant*)constants[0]);

    struct betree_event* invalid_event = betree_make_event(tree);
    struct betree_integer_list* wrong_seg = betree_make_integer_list(1);
    betree_add_integer(wrong_seg, 0, 1);
    betree_set_variable(invalid_event, 0, betree_make_integer_list_variable("seg", wrong_seg));

    struct betree_string_list* wrong_caps = betree_make_string_list(1);
    betree_add_string(wrong_caps, 0, "bad");
    betree_set_variable(
        invalid_event, 1, betree_make_string_list_variable("frequency_caps", wrong_caps));
    betree_set_variable(invalid_event, 2, betree_make_integer_variable("now", 0));

    struct report* report = make_report();
    const uint64_t ids[] = {1};

    mu_assert(!betree_search_with_event(tree, invalid_event, report), "");
    mu_assert(report->matched == 0, "");
    mu_assert(!betree_search_with_event_ids(tree, invalid_event, report, ids, 1), "");
    mu_assert(report->matched == 0, "");
    mu_assert(!betree_exists_with_event(tree, invalid_event), "");

    betree_free_event(invalid_event);
    free_report(report);
    betree_free(tree);
    return 0;
}

int test_search_with_event_conversion_invariants()
{
    struct betree* tree = betree_make();
    add_attr_domain_ie(tree->config, "ie", false);
    add_attr_domain_sl(tree->config, "sl", false);
    add_attr_domain_segments(tree->config, "seg", false);
    add_attr_domain_frequency(tree->config, "frequency_caps", false);

    mu_assert(betree_insert(tree, 1, "ie = 2"), "");
    mu_assert(betree_insert(tree, 2, "sl is empty"), "");
    mu_assert(betree_insert(tree, 3, "seg is empty"), "");
    mu_assert(betree_insert(tree, 4, "frequency_caps is empty"), "");

    struct betree_event* event = betree_make_event(tree);
    betree_set_variable(event, 0, betree_make_integer_variable("ie", 2));
    betree_set_variable(event, 1, betree_make_integer_list_variable("sl", betree_make_integer_list(0)));
    betree_set_variable(event, 2, betree_make_integer_list_variable("seg", betree_make_integer_list(0)));
    betree_set_variable(
        event, 3, betree_make_integer_list_variable("frequency_caps", betree_make_integer_list(0)));

    struct report* report = make_report();
    mu_assert(betree_search_with_event(tree, event, report), "");
    {
        const uint64_t expected[] = {1, 2, 3, 4};
        mu_assert(assert_report_id_set(report, expected, 4) == 0, "");
    }
    mu_assert(betree_exists_with_event(tree, event), "");

    betree_free_event(event);
    free_report(report);
    betree_free(tree);
    return 0;
}

int test_search_with_event_reuse_after_normalization()
{
    struct betree* tree = betree_make();
    add_attr_domain_ie(tree->config, "ie", false);
    add_attr_domain_sl(tree->config, "sl", false);
    add_attr_domain_segments(tree->config, "seg", false);
    add_attr_domain_frequency(tree->config, "frequency_caps", false);

    mu_assert(betree_insert(tree, 1, "ie = 2"), "");
    mu_assert(betree_insert(tree, 2, "sl is empty"), "");
    mu_assert(betree_insert(tree, 3, "seg is empty"), "");
    mu_assert(betree_insert(tree, 4, "frequency_caps is empty"), "");

    struct betree_event* event = betree_make_event(tree);
    betree_set_variable(event, 0, betree_make_integer_variable("ie", 2));
    betree_set_variable(event, 1, betree_make_integer_list_variable("sl", betree_make_integer_list(0)));
    betree_set_variable(event, 2, betree_make_integer_list_variable("seg", betree_make_integer_list(0)));
    betree_set_variable(
        event, 3, betree_make_integer_list_variable("frequency_caps", betree_make_integer_list(0)));

    const uint64_t expected[] = {1, 2, 3, 4};
    const uint64_t ids[] = {1, 3, 4};

    struct report* first_report = make_report();
    mu_assert(betree_search_with_event(tree, event, first_report), "");
    mu_assert(assert_report_id_set(first_report, expected, 4) == 0, "");

    mu_assert(betree_exists_with_event(tree, event), "");

    struct report* second_report = make_report();
    mu_assert(betree_search_with_event(tree, event, second_report), "");
    mu_assert(assert_report_id_set(second_report, expected, 4) == 0, "");

    struct report* ids_report = make_report();
    mu_assert(betree_search_with_event_ids(tree, event, ids_report, ids, 3), "");
    {
        const uint64_t expected_ids[] = {1, 3, 4};
        mu_assert(assert_report_id_set(ids_report, expected_ids, 3) == 0, "");
    }

    free_report(ids_report);
    free_report(second_report);
    free_report(first_report);
    betree_free_event(event);
    betree_free(tree);
    return 0;
}

int all_tests()
{
    mu_run_test(test_search);
    mu_run_test(test_search_ids_0);
    mu_run_test(test_search_ids_1);
    mu_run_test(test_search_ids_2);
    mu_run_test(test_search_ids_3);
    mu_run_test(test_search_ids_4);
    mu_run_test(test_search_ids_filtered_existence_semantics);
    mu_run_test(test_search_empty_lists);
    mu_run_test(test_search_integer_enum_event_normalization);
    mu_run_test(test_search_allow_undefined_by_domain);
    mu_run_test(test_search_disallow_undefined_validation);
    mu_run_test(test_search_with_event_parity);
    mu_run_test(test_search_with_event_ids_parity);
    mu_run_test(test_exists_parity);
    mu_run_test(test_search_with_event_invalid_event_rejection);
    mu_run_test(test_search_with_event_ids_invalid_event_rejection);
    mu_run_test(test_exists_invalid_event_rejection);
    mu_run_test(test_search_with_event_type_mismatch_rejection);
    mu_run_test(test_search_with_event_list_type_mismatch_rejection);
    mu_run_test(test_search_with_event_variable_replacement_semantics);
    mu_run_test(test_search_with_event_slot_reset_semantics);
    mu_run_test(test_search_with_event_special_domain_parity);
    mu_run_test(test_search_with_event_special_domain_type_mismatch_rejection);
    mu_run_test(test_search_with_event_conversion_invariants);
    mu_run_test(test_search_with_event_reuse_after_normalization);
    return 0;
}

RUN_TESTS()
