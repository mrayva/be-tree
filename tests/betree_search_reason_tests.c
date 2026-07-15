#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "alloc.h"
#include "dyn_arr.h"
#include "betree_err.h"
#include "debug.h"
#include "debug_err.h"
#include "helper.h"
#include "minunit.h"
#include "printer.h"
#include "special.h"
#include "tree_err.h"
#include "utils.h"

enum ATTR_DOMAIN_POSITION {
    ATTR_BOOL = 0,
    ATTR_INT,
    ATTR_FLOAT,
    ATTR_STR,
    ATTR_INT_LIST,
    ATTR_STR_LIST,
    ATTR_SEGMENTS,
    ATTR_FCAP,
    ATTR_GEO,
    ATTR_INT64,
};

#define ATTR_CONFIG_1(r) ((1 << r))
#define ATTR_CONFIG_2(r, s) ((1 << r) | (1 << s))
#define ATTR_CONFIG_3(r, s, t) ((1 << r) | (1 << s) | (1 << t))
#define ATTR_CONFIG_4(r, s, t, u) ((1 << r) | (1 << s) | (1 << t) | (1 << u))
#define ATTR_CONFIG_5(r, s, t, u, v) ((1 << r) | (1 << s) | (1 << t) | (1 << u) | (1 << v))
#define ATTR_CONFIG_6(r, s, t, u, v, w) \
    ((1 << r) | (1 << s) | (1 << t) | (1 << u) | (1 << v) | (1 << w))
#define ATTR_CONFIG_7(r, s, t, u, v, w, x) \
    ((1 << r) | (1 << s) | (1 << t) | (1 << u) | (1 << v) | (1 << w) | (1 << x))
#define ATTR_CONFIG_8(r, s, t, u, v, w, x, y) \
    ((1 << r) | (1 << s) | (1 << t) | (1 << u) | (1 << v) | (1 << w) | (1 << x) | (1 << y))
#define ATTR_CONFIG_9(r, s, t, u, v, w, x, y, z)                                           \
    ((1 << r) | (1 << s) | (1 << t) | (1 << u) | (1 << v) | (1 << w) | (1 << x) | (1 << y) \
        | (1 << z))
#define ATTR_CONFIG_10(r, s, t, u, v, w, x, y, z, q)                                       \
    ((1 << r) | (1 << s) | (1 << t) | (1 << u) | (1 << v) | (1 << w) | (1 << x) | (1 << y) \
        | (1 << z) | (1 << q))


void betree_bulk_insert(struct betree_err* tree, const char** exprs, const size_t count);
void betree_bulk_insert_with_constants(struct betree_err* tree,
    const char** exprs,
    size_t exprs_count,
    const struct betree_constant** constants,
    size_t constants_count);

void make_attr_domains(struct betree_err* tree, size_t config);
void make_attr_domains_undefined(struct betree_err* tree, size_t config, size_t config_undefined);

static int assert_match_parity(const struct report* report, const struct report_err* report_err)
{
    mu_assert(report->matched == report_err->matched, "matchedParity");
    for(size_t i = 0; i < report->matched; ++i) {
        mu_assert(report->subs[i] == report_err->subs[i], "matchedIdParity");
    }
    return 0;
}

static int assert_report_err_id_set(const struct report_err* report, const uint64_t* expected, size_t count)
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

int test_bool_fail()
{
    struct betree_err* tree = betree_make_err();

    make_attr_domains(tree, ATTR_CONFIG_2(ATTR_BOOL, ATTR_INT));

    const char* exprs[] = { "b and i = 1" };
    const size_t exprs_count = 1;
    betree_bulk_insert(tree, exprs, exprs_count);

    betree_make_sub_ids(tree);
    const char* event = "{\"b\": false, \"i\": 1}";
    struct report_err* report = make_report_err(tree);
#if defined(DEBUG)
    fprintf(stderr, "search ... %s\n", event);
#endif
    betree_search_err(tree, event, report);

    mu_assert(report->matched == 0, "goodEvent");
    dynamic_array_t* rlist = betree_reason_map_get(report->reason_sub_id_list, 0);
    mu_assert(rlist != NULL, "goodReport");
    mu_assert((betree_var_t)rlist->data[0] == (betree_var_t)1, "goodReason");
    mu_assert(strcmp(report->reason_sub_id_list->reasons[0]->name, "b") == 0, "goodReason");

    free_report_err(report);
    betree_free_err(tree);
#if defined(DEBUG)
    fprintf(stderr, "\n");
#endif
    return 0;
}


int test_int_fail()
{
    struct betree_err* tree = betree_make_err();

    make_attr_domains(tree, ATTR_CONFIG_2(ATTR_BOOL, ATTR_INT));

    const char* exprs[] = { "b and i = 1" };
    const size_t exprs_count = 1;
    betree_bulk_insert(tree, exprs, exprs_count);

    betree_make_sub_ids(tree);
    const char* event = "{\"b\": true, \"i\": 2}";
    struct report_err* report = make_report_err(tree);
#if defined(DEBUG)
    fprintf(stderr, "search ... %s\n", event);
#endif
    betree_search_err(tree, event, report);

    mu_assert(report->matched == 0, "goodEvent");
    dynamic_array_t* rlist = betree_reason_map_get(report->reason_sub_id_list, 1);
    mu_assert(rlist != NULL, "goodReport");
    mu_assert((betree_var_t)rlist->data[0] == (betree_var_t)1, "goodReason");
    mu_assert(strcmp(report->reason_sub_id_list->reasons[1]->name, "i") == 0, "goodReason");

    free_report_err(report);
    betree_free_err(tree);
#if defined(DEBUG)
    fprintf(stderr, "\n");
#endif
    return 0;
}


int test_float_fail()
{
    struct betree_err* tree = betree_make_err();

    make_attr_domains(tree, ATTR_CONFIG_2(ATTR_BOOL, ATTR_FLOAT));

    const char* exprs[] = { "b and f = 0.1" };
    const size_t exprs_count = 1;
    betree_bulk_insert(tree, exprs, exprs_count);

    betree_make_sub_ids(tree);
    const char* event = "{\"b\": true, \"f\": 0.2}";
    struct report_err* report = make_report_err(tree);
#if defined(DEBUG)
    fprintf(stderr, "search ... %s\n", event);
#endif
    betree_search_err(tree, event, report);

    mu_assert(report->matched == 0, "goodEvent");
    dynamic_array_t* rlist = betree_reason_map_get(report->reason_sub_id_list, 1);
    mu_assert(rlist != NULL, "goodReport");
    mu_assert((betree_var_t)rlist->data[0] == (betree_var_t)1, "goodReason");
    mu_assert(strcmp(report->reason_sub_id_list->reasons[1]->name, "f") == 0, "goodReason");

    free_report_err(report);
    betree_free_err(tree);
#if defined(DEBUG)
    fprintf(stderr, "\n");
#endif
    return 0;
}

int test_bin_fail()
{
    struct betree_err* tree = betree_make_err();

    make_attr_domains(tree, ATTR_CONFIG_2(ATTR_BOOL, ATTR_STR));

    const char* exprs[] = { "b and s = \"betrees\"" };
    const size_t exprs_count = 1;
    betree_bulk_insert(tree, exprs, exprs_count);

    betree_make_sub_ids(tree);
    const char* event = "{\"b\": true, \"s\": \"betree\"}";
    struct report_err* report = make_report_err(tree);
#if defined(DEBUG)
    fprintf(stderr, "search ... %s\n", event);
#endif

    betree_search_err(tree, event, report);
    mu_assert(report->matched == 0, "goodEvent");
    dynamic_array_t* rlist = betree_reason_map_get(report->reason_sub_id_list, 1);
    mu_assert(rlist != NULL, "goodReport");
    mu_assert((betree_var_t)rlist->data[0] == (betree_var_t)1, "goodReason");
    mu_assert(strcmp(report->reason_sub_id_list->reasons[1]->name, "s") == 0, "goodReason");

    free_report_err(report);
    betree_free_err(tree);
#if defined(DEBUG)
    fprintf(stderr, "\n");
#endif
    return 0;
}


int test_int_list_fail()
{
    struct betree_err* tree = betree_make_err();

    make_attr_domains(tree, ATTR_CONFIG_2(ATTR_BOOL, ATTR_INT_LIST));

    const char* exprs[] = { "b and il one of (1,2)" };
    const size_t exprs_count = 1;
    betree_bulk_insert(tree, exprs, exprs_count);

    betree_make_sub_ids(tree);
    const char* event = "{\"b\": true, \"il\": [3]}";
    struct report_err* report = make_report_err(tree);
#if defined(DEBUG)
    fprintf(stderr, "search ... %s\n", event);
#endif
    betree_search_err(tree, event, report);

    mu_assert(report->matched == 0, "goodEvent");
    dynamic_array_t* rlist = betree_reason_map_get(report->reason_sub_id_list, 1);
    mu_assert(rlist != NULL, "goodReport");
    mu_assert((betree_var_t)rlist->data[0] == (betree_var_t)1, "goodReason");
    mu_assert(strcmp(report->reason_sub_id_list->reasons[1]->name, "il") == 0, "goodReason");

    free_report_err(report);
    betree_free_err(tree);
#if defined(DEBUG)
    fprintf(stderr, "\n");
#endif
    return 0;
}


int test_bin_list_fail()
{
    struct betree_err* tree = betree_make_err();

    make_attr_domains(tree, ATTR_CONFIG_2(ATTR_BOOL, ATTR_STR_LIST));

    const char* exprs[] = { "b and sl one of (\"How\",\"is\", \"it\", \"going\")" };
    const size_t exprs_count = 1;
    betree_bulk_insert(tree, exprs, exprs_count);

    betree_make_sub_ids(tree);
    const char* event = "{\"b\": true, \"sl\": [\"how\"]}";
    struct report_err* report = make_report_err(tree);
#if defined(DEBUG)
    fprintf(stderr, "search ... %s\n", event);
#endif
    betree_search_err(tree, event, report);

    mu_assert(report->matched == 0, "goodEvent");
    dynamic_array_t* rlist = betree_reason_map_get(report->reason_sub_id_list, 1);
    mu_assert(rlist != NULL, "goodReport");
    mu_assert((betree_var_t)rlist->data[0] == (betree_var_t)1, "goodReason");
    mu_assert(strcmp(report->reason_sub_id_list->reasons[1]->name, "sl") == 0, "goodReason");

    free_report_err(report);
    betree_free_err(tree);
#if defined(DEBUG)
    fprintf(stderr, "\n");
#endif
    return 0;
}


int test_segments_fail()
{
    struct betree_err* tree = betree_make_err();

    make_attr_domains(tree, ATTR_CONFIG_2(ATTR_INT64, ATTR_SEGMENTS));

    const char* exprs[] = { "segment_within(seg, 1, 10)" };
    const size_t exprs_count = 1;
    betree_bulk_insert(tree, exprs, exprs_count);

    betree_make_sub_ids(tree);
    const char* event = "{\"now\": 30, \"seg\": [[1, 10000000]]}";
    struct report_err* report = make_report_err(tree);
#if defined(DEBUG)
    fprintf(stderr, "search ... %s\n", event);
#endif
    betree_search_err(tree, event, report);

    mu_assert(report->matched == 0, "goodEvent");
    dynamic_array_t* rlist = betree_reason_map_get(report->reason_sub_id_list, 0);
    mu_assert(rlist != NULL, "goodReport");
    mu_assert((betree_var_t)rlist->data[0] == (betree_var_t)1, "goodReason");
    mu_assert(strcmp(report->reason_sub_id_list->reasons[0]->name, "seg") == 0, "goodReason");

    free_report_err(report);
    betree_free_err(tree);
#if defined(DEBUG)
    fprintf(stderr, "\n");
#endif
    return 0;
}


int test_frequency_cap_fail()
{
    struct betree_err* tree = betree_make_err();

    const size_t constant_count = 1;
    const struct betree_constant* constants[]
        = { betree_make_integer_constant("advertiser_id", 20) };

    make_attr_domains(tree, ATTR_CONFIG_2(ATTR_INT64, ATTR_FCAP));

    const char* exprs[] = { "not within_frequency_cap(\"advertiser\", \"namespace\", 100, 100)" };
    const size_t exprs_count = 1;
    betree_bulk_insert_with_constants(tree, exprs, exprs_count, constants, constant_count);

    betree_make_sub_ids(tree);
    const char* event
        = "{\"now\": 30, \"frequency_caps\": [[\"campaign\", 30, \"namespace\", 20, 10]]}";
    struct report_err* report = make_report_err(tree);
#if defined(DEBUG)
    fprintf(stderr, "search ... %s\n", event);
#endif
    betree_search_err(tree, event, report);

    mu_assert(report->matched == 0, "goodEvent");
    dynamic_array_t* rlist = betree_reason_map_get(report->reason_sub_id_list, 0);
    mu_assert(rlist != NULL, "goodReport");
    mu_assert((betree_var_t)rlist->data[0] == (betree_var_t)1, "goodReason");
    mu_assert(
        strcmp(report->reason_sub_id_list->reasons[0]->name, "frequency_caps") == 0, "goodReason");

    free_report_err(report);
    betree_free_err(tree);
#if defined(DEBUG)
    fprintf(stderr, "\n");
#endif
    return 0;
}


int test_geo_fail()
{
    struct betree_err* tree = betree_make_err();

    make_attr_domains(tree, ATTR_CONFIG_2(ATTR_BOOL, ATTR_GEO));

    const char* exprs[] = { "b and geo_within_radius(10, 100, 100)" };
    const size_t exprs_count = 1;
    betree_bulk_insert(tree, exprs, exprs_count);

    betree_make_sub_ids(tree);
    const char* event = "{\"b\": true, \"latitude\": 101.0, \"longitude\": 99.0}";
    struct report_err* report = make_report_err(tree);
#if defined(DEBUG)
    fprintf(stderr, "search ... %s\n", event);
#endif
    betree_search_err(tree, event, report);

    mu_assert(report->matched == 0, "goodEvent");
    dynamic_array_t* rlist = betree_reason_map_get(report->reason_sub_id_list, 3);
    mu_assert(rlist != NULL, "goodReport");
    mu_assert((betree_var_t)rlist->data[0] == (betree_var_t)1, "goodReason");

    free_report_err(report);
    betree_free_err(tree);
#if defined(DEBUG)
    fprintf(stderr, "\n");
#endif
    return 0;
}


int test_int64_fail()
{
    struct betree_err* tree = betree_make_err();

    make_attr_domains(tree, ATTR_CONFIG_2(ATTR_BOOL, ATTR_INT64));

    const char* exprs[] = { "b and now = 1" };
    const size_t exprs_count = 1;
    betree_bulk_insert(tree, exprs, exprs_count);

    betree_make_sub_ids(tree);
    const char* event = "{\"b\": true, \"now\": 2}";
    struct report_err* report = make_report_err(tree);
#if defined(DEBUG)
    fprintf(stderr, "search ... %s\n", event);
#endif
    betree_search_err(tree, event, report);

    mu_assert(report->matched == 0, "goodEvent");
    dynamic_array_t* rlist = betree_reason_map_get(report->reason_sub_id_list, 1);
    mu_assert(rlist != NULL, "goodReport");
    mu_assert((betree_var_t)rlist->data[0] == (betree_var_t)1, "goodReason");

    free_report_err(report);
    betree_free_err(tree);
#if defined(DEBUG)
    fprintf(stderr, "\n");
#endif
    return 0;
}


int test_short_circuit_fail()
{
    struct betree_err* tree = betree_make_err();

    make_attr_domains_undefined(tree,
        ATTR_CONFIG_5(ATTR_BOOL, ATTR_INT, ATTR_FLOAT, ATTR_STR, ATTR_INT_LIST),
        ATTR_CONFIG_3(ATTR_FLOAT, ATTR_STR, ATTR_INT_LIST));

    const char* exprs[] = { "b and i = 1 and f = 0.1 and s = \"s1\"",
        "b and i = 2 and s = \"s2\"",
        "b and i = 3 and (il one of (1, 2, 3))" };
    const size_t exprs_count = 3;
    betree_bulk_insert(tree, exprs, exprs_count);

    betree_make_sub_ids(tree);
    const char* event = "{\"b\": true, \"i\": 0}";
    struct report_err* report = make_report_err(tree);
#if defined(DEBUG)
    fprintf(stderr, "search ... %s\n", event);
#endif
    betree_search_err(tree, event, report);

    mu_assert(report->matched == 0, "goodEvent");
    dynamic_array_t* rlist = betree_reason_map_get(report->reason_sub_id_list, 2);
    mu_assert(rlist != NULL, "goodReport");
    dynamic_array_t* rlist2 = betree_reason_map_get(report->reason_sub_id_list, 3);
    mu_assert(rlist2 != NULL, "goodReport");
    dynamic_array_t* rlist3 = betree_reason_map_get(report->reason_sub_id_list, 4);
    mu_assert(rlist3 != NULL, "goodReport");
    mu_assert((betree_var_t)rlist->data[0] == (betree_var_t)1, "goodReason");
    mu_assert((betree_var_t)rlist2->data[0] == (betree_var_t)2, "goodReason");
    mu_assert((betree_var_t)rlist3->data[0] == (betree_var_t)3, "goodReason");

    free_report_err(report);
    betree_free_err(tree);
#if defined(DEBUG)
    fprintf(stderr, "\n");
#endif
    return 0;
}


int test_multiple_bool_exprs_fail()
{
    struct betree_err* tree = betree_make_err();

    make_attr_domains(tree, ATTR_CONFIG_4(ATTR_BOOL, ATTR_INT, ATTR_FLOAT, ATTR_STR));

    const char* exprs[] = {
        "not (((not b) and i = 2 and f = 0.3) or (s <> \"s0\"))",
        "not ((b and i = 1 and f = 0.0) or (s <> \"s1\"))",
        "(b or i = 0 or f = 0.1) or (s <> \"s3\")",
        "not ((b or i = 1 or f = 0.2) or (s = \"s2\"))",
        "not ((b or i = 2 or f = 0.1) or (s = \"s3\"))",
    };
    const size_t exprs_count = 5;
    betree_bulk_insert(tree, exprs, exprs_count);

    betree_make_sub_ids(tree);
    const char* event = "{\"b\": false, \"i\": 2, \"f\": 0.2, \"s\": \"s3\"}";
    struct report_err* report = make_report_err(tree);
#if defined(DEBUG)
    fprintf(stderr, "search ... %s\n", event);
#endif
    betree_search_err(tree, event, report);

    mu_assert(report->matched == 0, "goodEvent");
    dynamic_array_t* rlist = betree_reason_map_get(report->reason_sub_id_list, 1);
    mu_assert(rlist != NULL, "goodReport");
    dynamic_array_t* rlist2 = betree_reason_map_get(report->reason_sub_id_list, 2);
    mu_assert(rlist2 != NULL, "goodReport");
    dynamic_array_t* rlist3 = betree_reason_map_get(report->reason_sub_id_list, 3);
    mu_assert(rlist3 != NULL, "goodReport");
    mu_assert((betree_var_t)rlist->data[0] == (betree_var_t)3, "goodReason");
    mu_assert((betree_var_t)rlist2->data[0] == (betree_var_t)4, "goodReason");

    const betree_var_t expected_set[] = { 1, 2, 5 };
    size_t found_total = 0;
    for(size_t j = 0; j < 3; j++) {
        for(size_t k = 0; k < 3; k++) {
            found_total += ((betree_var_t)rlist3->data[k] == (betree_var_t)expected_set[j]) ? 1 : 0;
        }
    }
    mu_assert(found_total == 3, "goodReason");

    write_dot_to_file_err(tree, "tests/beetree_search_reason_tests_multipl_bool_exprs.dot");

    free_report_err(report);
    betree_free_err(tree);
#if defined(DEBUG)
    fprintf(stderr, "\n");
#endif
    return 0;
}


int test_memoize_fail()
{
    struct betree_err* tree = betree_make_err();

    make_attr_domains(tree, ATTR_CONFIG_4(ATTR_BOOL, ATTR_INT, ATTR_FLOAT, ATTR_STR));

    const char* exprs[] = {
        "(b and i = 1 and f = 1.0 and s = \"s3\") or (s = \"s7\" and (not b))",
        "(b and i = 2 and f = 2.0 and s = \"s4\") or (s = \"s8\" and (not b))",
        "(b and i = 2 and f = 3.0 and s = \"s5\") or (s = \"s9\" and (not b))",
        "(b and i = 2 and f = 3.0 and s = \"s6\") or (s = \"s9\" and (not b))",
        "not (b and i = 2 and f = 3.0 and s = \"s6\") and (s = \"s9\" and (not b))",
    };
    const size_t exprs_count = 5;
    betree_bulk_insert(tree, exprs, exprs_count);

    betree_make_sub_ids(tree);

    const char* event = "{\"b\": false, \"i\": 3, \"f\": 0.0, \"s\": \"s12\"}";
    struct report_err* report = make_report_err(tree);
#if defined(DEBUG)
    fprintf(stderr, "search ... %s\n", event);
#endif
    betree_search_err(tree, event, report);

    mu_assert(report->matched == 0, "goodEvent");
    const betree_var_t expected_set[] = { 1, 2, 3, 4, 5 };
    size_t found_total = 0;
    dynamic_array_t* rlist = betree_reason_map_get(report->reason_sub_id_list, 3);
    mu_assert(rlist != NULL, "goodReport");
    for(size_t j = 0; j < 5; j++) {
        for(size_t k = 0; k < 5; k++) {
            found_total += ((betree_var_t)(rlist->data[k]) == (betree_var_t)expected_set[j]) ? 1 : 0;
        }
    }
    mu_assert(found_total == 5, "goodReason");

    // write_dot_to_file_err(tree, "tests/beetree_search_reason_tests_multipl_bool_exprs.dot");

    free_report_err(report);
    betree_free_err(tree);
#if defined(DEBUG)
    fprintf(stderr, "\n");
#endif
    return 0;
}

int test_memoize_fail_shared_reason()
{
    struct betree_err* tree = betree_make_err();

    make_attr_domains(tree, ATTR_CONFIG_2(ATTR_BOOL, ATTR_INT));

    const char* exprs[] = { "i = 1 and b", "i = 1 and not b" };
    const size_t exprs_count = 2;
    betree_bulk_insert(tree, exprs, exprs_count);

    betree_make_sub_ids(tree);

    const char* event = "{\"b\": true, \"i\": 2}";
    struct report_err* report = make_report_err(tree);
    betree_search_err(tree, event, report);

    mu_assert(report->matched == 0, "goodEvent");
    mu_assert(report->evaluated == 2, "evaluated");
    mu_assert(report->memoized == 1, "memoizedSharedPredicate");

    dynamic_array_t* rlist = betree_reason_map_get(report->reason_sub_id_list, 1);
    mu_assert(rlist != NULL, "goodReport");
    mu_assert(rlist->size == 2, "reasonCount");
    mu_assert(
        ((betree_var_t)rlist->data[0] == (betree_var_t)1
            && (betree_var_t)rlist->data[1] == (betree_var_t)2)
            || ((betree_var_t)rlist->data[0] == (betree_var_t)2
                && (betree_var_t)rlist->data[1] == (betree_var_t)1),
        "goodReason");
    mu_assert(strcmp(report->reason_sub_id_list->reasons[1]->name, "i") == 0, "goodReasonName");

    free_report_err(report);
    betree_free_err(tree);
    return 0;
}

int test_memoize_fail_shared_reason_ids()
{
    struct betree_err* tree = betree_make_err();

    make_attr_domains(tree, ATTR_CONFIG_2(ATTR_BOOL, ATTR_INT));

    const char* exprs[] = { "i = 1 and b", "i = 1 and not b", "i = 2 and b" };
    const size_t exprs_count = 3;
    const uint64_t ids[] = { 2, 3 };
    const size_t sz = 2;
    betree_bulk_insert(tree, exprs, exprs_count);

    betree_make_sub_ids(tree);

    const char* event = "{\"b\": true, \"i\": 2}";
    struct report_err* report = make_report_err(tree);
    betree_search_ids_err(tree, event, report, ids, sz);

    mu_assert(report->matched == 1 && report->subs[0] == 3, "goodMatch");
    mu_assert(report->evaluated == 2, "evaluated");
    mu_assert(report->memoized == 0, "filteredSharedPredicateNotReused");

    dynamic_array_t* rlist = betree_reason_map_get(report->reason_sub_id_list, 1);
    mu_assert(rlist != NULL, "goodReport");
    mu_assert(rlist->size == 1, "reasonCount");
    mu_assert((betree_var_t)rlist->data[0] == (betree_var_t)2, "goodReason");
    mu_assert(strcmp(report->reason_sub_id_list->reasons[1]->name, "i") == 0, "goodReasonName");

    free_report_err(report);
    betree_free_err(tree);
    return 0;
}

int test_all_search_term()
{
    struct betree_err* tree = betree_make_err();

    const size_t constant_count = 4;
    const struct betree_constant* constants[] = { betree_make_integer_constant("campaign_id", 10),
        betree_make_integer_constant("advertiser_id", 20),
        betree_make_integer_constant("flight_id", 30),
        betree_make_integer_constant("product_id", 40) };

    make_attr_domains(tree,
        ATTR_CONFIG_9(ATTR_BOOL,
            ATTR_INT,
            ATTR_FLOAT,
            ATTR_STR,
            ATTR_FCAP,
            ATTR_STR_LIST,
            ATTR_INT_LIST,
            ATTR_SEGMENTS,
            ATTR_INT64));

    const char* exprs[]
        = { "b and i = 10 and f > 3.13 and s = \"good\" and 1 in il and sl none of (\"good\") and "
            "segment_within(seg, 1, 20) and within_frequency_cap(\"flight\", \"ns\", 100, 0)",
              "i = 10 and f > 3.13 and s = \"good\" and 1 in il and sl none of (\"good\") and "
              "segment_within(seg, 1, 20) and within_frequency_cap(\"flight\", \"ns\", 100, 0)" };
    const size_t exprs_count = 2;

    betree_bulk_insert_with_constants(tree, exprs, exprs_count, constants, constant_count);

    betree_make_sub_ids(tree);

    const char* event = "{\"b\": true, \"i\": 10, \"f\": 3.14, \"s\": \"good\", \"il\": [1,2,3], "
                        "\"sl\": [\"bad\"], \"seg\": [[1, 20000001]], \"frequency_caps\": "
                        "[[\"flight\", 10, \"ns\", 0, 0]], \"now\": 100}";
    struct report_err* report = make_report_err(tree);
#if defined(DEBUG)
    fprintf(stderr, "search ... %s\n", event);
#endif
    betree_search_err(tree, event, report);

    mu_assert(report->matched == 0, "goodEvent");
    const betree_var_t expected_set[] = { 1, 2 };
    size_t found_total = 0;
    dynamic_array_t* rlist = betree_reason_map_get(report->reason_sub_id_list, 6);
    mu_assert(rlist != NULL, "goodReport");
    for(size_t j = 0; j < 2; j++) {
        for(size_t k = 0; k < 2; k++) {
            found_total += ((betree_var_t)(rlist->data[k]) == (betree_var_t)expected_set[j]) ? 1 : 0;
        }
    }
    mu_assert(found_total == 2, "goodReason");

    free_report_err(report);
    betree_free_err(tree);
#if defined(DEBUG)
    fprintf(stderr, "\n");
#endif
    return 0;
}


int test_event_search_reason()
{
    struct betree_err* tree = betree_make_err();

    make_attr_domains(tree, ATTR_CONFIG_4(ATTR_BOOL, ATTR_INT, ATTR_FLOAT, ATTR_STR));

    const char* exprs[] = {
        "b and i = 10 and f < 3.13 and s = \"good\"",
        "b and i = 10 and f > 3.13 and s = \"bad\"",
        "b and i = 10 and f < 3.13 and s = \"good\"",
        "not b and i = 11 and f > 3.13 and s = \"bad\"",
        "not b and i = 11 and f < 3.13 and s = \"good\"",
        "not b and i = 11 and f > 3.13 and s = \"bad\"",
        "not b and i = 11 and f < 3.13 and s = \"good\"",
    };
    const size_t exprs_count = 7;

    betree_bulk_insert(tree, exprs, exprs_count);

    betree_make_sub_ids(tree);

    const char* event = "{\"b\": true, \"i\": 10, \"f\": 3.14, \"s\": \"cool\"}";

    struct report_err* report = make_report_err(tree);
#if defined(DEBUG)
    fprintf(stderr, "search ... %s\n", event);
#endif
    betree_search_err(tree, event, report);

    mu_assert(report->matched == 0, "goodEvent");
    const betree_var_t expected_set1[] = { 1, 2, 3 };
    size_t found_total1 = 0;
    dynamic_array_t* rlist = betree_reason_map_get(report->reason_sub_id_list, 3);
    mu_assert(rlist != NULL, "goodReport");
    for(size_t j = 0; j < 3; j++) {
        for(size_t k = 0; k < 3; k++) {
            found_total1 += ((betree_var_t)(rlist->data[k]) == (betree_var_t)expected_set1[j]) ? 1 : 0;
        }
    }
    mu_assert(found_total1 == 3, "goodReason");

    const betree_var_t expected_set2[] = { 4, 5, 6, 7 };
    size_t found_total2 = 0;
    dynamic_array_t* rlist2 = betree_reason_map_get(report->reason_sub_id_list, 0);
    mu_assert(rlist2 != NULL, "goodReport");
    for(size_t j = 0; j < 4; j++) {
        for(size_t k = 0; k < 4; k++) {
            found_total2 += ((betree_var_t)(rlist2->data[k]) == (betree_var_t)expected_set2[j]) ? 1 : 0;
        }
    }
    mu_assert(found_total2 == 4, "goodReason");

    free_report_err(report);
    betree_free_err(tree);
#if defined(DEBUG)
    fprintf(stderr, "\n");
#endif
    return 0;
}

int test_excluded_branch_reason()
{
    struct betree_err* tree = betree_make_err();

    make_attr_domains(tree, ATTR_CONFIG_1(ATTR_INT));

    const char* exprs[] = { "i = 0", "i = 1" };
    const size_t exprs_count = 2;
    betree_bulk_insert(tree, exprs, exprs_count);

    betree_make_sub_ids(tree);

    struct report_err* report = make_report_err(tree);
    betree_search_err(tree, "{\"i\": 0}", report);

    mu_assert(report->matched == 1 && report->subs[0] == 1, "goodMatch");
    dynamic_array_t* rlist = betree_reason_map_get(report->reason_sub_id_list, 0);
    mu_assert(rlist != NULL, "goodReport");
    mu_assert(rlist->size == 1, "singleExcludedReason");
    mu_assert((betree_var_t)rlist->data[0] == (betree_var_t)2, "excludedBranchSub");
    mu_assert(strcmp(report->reason_sub_id_list->reasons[0]->name, "i") == 0, "goodReason");

    free_report_err(report);
    betree_free_err(tree);
    return 0;
}

int test_search_ids_err_excluded_and_evaluated_reasons()
{
    struct betree_err* tree = betree_make_err();

    make_attr_domains(tree, ATTR_CONFIG_2(ATTR_BOOL, ATTR_INT));

    const char* exprs[] = { "i = 0", "i = 1", "i = 0 and b" };
    const size_t exprs_count = 3;
    const uint64_t ids[] = { 2, 3 };
    const size_t sz = 2;
    betree_bulk_insert(tree, exprs, exprs_count);

    betree_make_sub_ids(tree);

    struct report_err* report = make_report_err(tree);
    betree_search_ids_err(tree, "{\"b\": false, \"i\": 0}", report, ids, sz);

    mu_assert(report->matched == 0, "goodMatch");

    dynamic_array_t* int_reason = betree_reason_map_get(report->reason_sub_id_list, 1);
    mu_assert(int_reason != NULL, "intReason");
    mu_assert(int_reason->size == 1, "singleIntReason");
    mu_assert((betree_var_t)int_reason->data[0] == (betree_var_t)2, "excludedBranchSub");
    mu_assert(strcmp(report->reason_sub_id_list->reasons[1]->name, "i") == 0, "goodIntName");

    dynamic_array_t* bool_reason = betree_reason_map_get(report->reason_sub_id_list, 0);
    mu_assert(bool_reason != NULL, "boolReason");
    mu_assert(bool_reason->size == 1, "singleBoolReason");
    mu_assert((betree_var_t)bool_reason->data[0] == (betree_var_t)3, "evaluatedFailSub");
    mu_assert(strcmp(report->reason_sub_id_list->reasons[0]->name, "b") == 0, "goodBoolName");

    free_report_err(report);
    betree_free_err(tree);
    return 0;
}

int test_allow_undefined_reason_search_by_domain()
{
    struct betree_err* tree = betree_make_err();

    make_attr_domains_undefined(tree,
        ATTR_CONFIG_5(ATTR_INT, ATTR_STR, ATTR_INT_LIST, ATTR_STR_LIST, ATTR_INT64),
        ATTR_CONFIG_5(ATTR_INT, ATTR_STR, ATTR_INT_LIST, ATTR_STR_LIST, ATTR_INT64));

    const char* exprs[] = {
        "i is null",
        "s is null",
        "il is null",
        "sl is null",
        "now is null",
        "il is empty",
        "sl is empty",
    };
    const size_t exprs_count = 7;
    betree_bulk_insert(tree, exprs, exprs_count);

    betree_make_sub_ids(tree);

    struct report_err* report = make_report_err(tree);
    mu_assert(betree_search_err(tree, "{}", report), "searchSucceeds");
    mu_assert(report->matched == 5, "goodMatchCount");
    mu_assert(report->subs[0] == 1 && report->subs[1] == 2 && report->subs[2] == 3
            && report->subs[3] == 4 && report->subs[4] == 5,
        "goodMatchedSubs");

    betree_var_t invalid_event
        = ADDITIONAL_REASON(tree->config->attr_domain_count, REASON_INVALID_EVENT);
    dynamic_array_t* invalid_reason = betree_reason_map_get(report->reason_sub_id_list, invalid_event);
    mu_assert(invalid_reason == NULL || invalid_reason->size == 0, "noInvalidEventReason");

    free_report_err(report);
    betree_free_err(tree);
    return 0;
}

int test_disallow_undefined_reason_invalid_event()
{
    struct betree_err* tree = betree_make_err();

    make_attr_domains(tree, ATTR_CONFIG_2(ATTR_INT, ATTR_INT_LIST));

    const char* exprs[] = { "i = 1", "il is empty" };
    const size_t exprs_count = 2;
    betree_bulk_insert(tree, exprs, exprs_count);

    betree_make_sub_ids(tree);

    struct report_err* report = make_report_err(tree);
    mu_assert(!betree_search_err(tree, "{}", report), "searchFailsValidation");
    mu_assert(report->matched == 0, "noMatches");

    betree_var_t invalid_event
        = ADDITIONAL_REASON(tree->config->attr_domain_count, REASON_INVALID_EVENT);
    dynamic_array_t* invalid_reason = betree_reason_map_get(report->reason_sub_id_list, invalid_event);
    mu_assert(invalid_reason != NULL, "invalidEventReason");
    mu_assert(invalid_reason->size == 2, "allSubsTagged");
    mu_assert(
        ((betree_var_t)invalid_reason->data[0] == (betree_var_t)1
            && (betree_var_t)invalid_reason->data[1] == (betree_var_t)2)
            || ((betree_var_t)invalid_reason->data[0] == (betree_var_t)2
                && (betree_var_t)invalid_reason->data[1] == (betree_var_t)1),
        "goodReasonSubs");

    free_report_err(report);
    betree_free_err(tree);
    return 0;
}

int test_search_err_match_set_parity()
{
    struct betree* tree = betree_make();
    struct betree_err* tree_err = betree_make_err();

    add_attr_domain_b(tree->config, "flag", false);
    add_attr_domain_bounded_i(tree->config, "age", false, 0, 120);
    add_attr_domain_s(tree->config, "country", false);

    add_attr_domain_b(tree_err->config, "flag", false);
    add_attr_domain_bounded_i(tree_err->config, "age", false, 0, 120);
    add_attr_domain_s(tree_err->config, "country", false);

    const char* exprs[] = {
        "flag and age >= 21",
        "country = \"USA\"",
        "flag and country = \"CAN\"",
    };

    for(size_t i = 0; i < 3; ++i) {
        mu_assert(betree_insert(tree, i + 1, exprs[i]), "");
        mu_assert(betree_insert_err(tree_err, i + 1, exprs[i]), "");
    }

    struct report* report = make_report();
    struct report_err* report_err = make_report_err(tree_err);
    const char* event = "{\"flag\": true, \"age\": 25, \"country\": \"USA\"}";

    mu_assert(betree_search(tree, event, report), "");
    mu_assert(betree_search_err(tree_err, event, report_err), "");
    mu_assert(assert_match_parity(report, report_err) == 0, "");

    const uint64_t ids[] = {1, 2, 3};
    struct report* ids_report = make_report();
    struct report_err* ids_report_err = make_report_err(tree_err);
    mu_assert(betree_search_ids(tree, event, ids_report, ids, 3), "");
    mu_assert(betree_search_ids_err(tree_err, event, ids_report_err, ids, 3), "");
    mu_assert(assert_match_parity(ids_report, ids_report_err) == 0, "");

    free_report(ids_report);
    free_report_err(ids_report_err);
    free_report(report);
    free_report_err(report_err);
    betree_free(tree);
    betree_free_err(tree_err);
    return 0;
}

int test_search_with_event_err_parity()
{
    struct betree_err* tree = betree_make_err();

    add_attr_domain_b(tree->config, "flag", false);
    add_attr_domain_bounded_i(tree->config, "age", false, 0, 120);
    add_attr_domain_s(tree->config, "country", false);

    const char* exprs[] = {
        "flag and age >= 21",
        "country = \"USA\"",
        "flag and country = \"CAN\"",
    };

    betree_bulk_insert(tree, exprs, 3);
    betree_make_sub_ids(tree);

    struct report_err* json_report = make_report_err(tree);
    const char* event_json = "{\"flag\": true, \"age\": 25, \"country\": \"USA\"}";
    mu_assert(betree_search_err(tree, event_json, json_report), "");

    struct betree_event* event = betree_make_event_err(tree);
    betree_set_variable(event, 0, betree_make_boolean_variable("flag", true));
    betree_set_variable(event, 1, betree_make_integer_variable("age", 25));
    betree_set_variable(event, 2, betree_make_string_variable("country", "USA"));

    struct report_err* event_report = make_report_err(tree);
    mu_assert(betree_search_with_event_err(tree, event, event_report), "");

    const uint64_t expected[] = {1, 2};
    mu_assert(assert_report_err_id_set(json_report, expected, 2) == 0, "");
    mu_assert(assert_report_err_id_set(event_report, expected, 2) == 0, "");

    free_report_err(event_report);
    betree_free_event(event);
    free_report_err(json_report);
    betree_free_err(tree);
    return 0;
}

int test_search_with_event_err_special_domain_parity()
{
    struct betree_err* tree = betree_make_err();

    const struct betree_constant* constants[] = {
        betree_make_integer_constant("flight_id", 10),
    };

    add_attr_domain_segments(tree->config, "seg", false);
    add_attr_domain_frequency(tree->config, "frequency_caps", false);
    add_attr_domain_i(tree->config, "now", false);

    const char* exprs[] = {
        "segment_within(seg, 1, 20)",
        "within_frequency_cap(\"flight\", \"ns\", 100, 0)",
        "segment_within(seg, 1, 20) and within_frequency_cap(\"flight\", \"ns\", 100, 0)",
    };
    betree_bulk_insert_with_constants(tree, exprs, 3, constants, 1);
    betree_make_sub_ids(tree);
    betree_free_constant((struct betree_constant*)constants[0]);

    struct report_err* json_report = make_report_err(tree);
    const char* event_json
        = "{\"now\": 0, \"seg\": [[1, 10000000]], "
          "\"frequency_caps\": [[\"flight\", 10, \"ns\", 0, 0]]}";
    mu_assert(betree_search_err(tree, event_json, json_report), "");

    struct betree_event* event = betree_make_event_err(tree);
    struct betree_segments* seg = betree_make_segments(1);
    betree_add_segment(seg, 0, betree_make_segment(1, 10000000));
    betree_set_variable(event, 0, betree_make_segments_variable("seg", seg));

    struct betree_frequency_caps* frequency_caps = betree_make_frequency_caps(1);
    betree_add_frequency_cap(
        frequency_caps, 0, betree_make_frequency_cap("flight", 10, "ns", false, 0, 0));
    betree_set_variable(
        event, 1, betree_make_frequency_caps_variable("frequency_caps", frequency_caps));
    betree_set_variable(event, 2, betree_make_integer_variable("now", 0));

    struct report_err* event_report = make_report_err(tree);
    mu_assert(betree_search_with_event_err(tree, event, event_report), "");

    const uint64_t expected[] = {1, 2, 3};
    mu_assert(assert_report_err_id_set(json_report, expected, 3) == 0, "");
    mu_assert(assert_report_err_id_set(event_report, expected, 3) == 0, "");

    const uint64_t ids[] = {2, 3};
    struct report_err* json_ids_report = make_report_err(tree);
    struct report_err* event_ids_report = make_report_err(tree);
    mu_assert(betree_search_ids_err(tree, event_json, json_ids_report, ids, 2), "");
    mu_assert(betree_search_with_event_ids_err(tree, event, event_ids_report, ids, 2), "");

    const uint64_t expected_ids[] = {2, 3};
    mu_assert(assert_report_err_id_set(json_ids_report, expected_ids, 2) == 0, "");
    mu_assert(assert_report_err_id_set(event_ids_report, expected_ids, 2) == 0, "");

    free_report_err(event_ids_report);
    free_report_err(json_ids_report);
    free_report_err(event_report);
    betree_free_event(event);
    free_report_err(json_report);
    betree_free_err(tree);
    return 0;
}

int test_search_with_event_err_conversion_invariants()
{
    struct betree_err* tree = betree_make_err();

    add_attr_domain_ie(tree->config, "ie", false);
    add_attr_domain_sl(tree->config, "sl", false);
    add_attr_domain_segments(tree->config, "seg", false);
    add_attr_domain_frequency(tree->config, "frequency_caps", false);

    const char* exprs[] = { "ie = 2", "sl is empty", "seg is empty", "frequency_caps is empty" };
    betree_bulk_insert(tree, exprs, 4);
    betree_make_sub_ids(tree);

    struct betree_event* event = betree_make_event_err(tree);
    betree_set_variable(event, 0, betree_make_integer_variable("ie", 2));
    betree_set_variable(event, 1, betree_make_integer_list_variable("sl", betree_make_integer_list(0)));
    betree_set_variable(event, 2, betree_make_integer_list_variable("seg", betree_make_integer_list(0)));
    betree_set_variable(
        event, 3, betree_make_integer_list_variable("frequency_caps", betree_make_integer_list(0)));

    struct report_err* report = make_report_err(tree);
    mu_assert(betree_search_with_event_err(tree, event, report), "");
    {
        const uint64_t expected[] = {1, 2, 3, 4};
        mu_assert(assert_report_err_id_set(report, expected, 4) == 0, "");
    }

    free_report_err(report);
    betree_free_event(event);
    betree_free_err(tree);
    return 0;
}

int test_search_with_event_err_reuse_after_normalization()
{
    struct betree_err* tree = betree_make_err();

    add_attr_domain_ie(tree->config, "ie", false);
    add_attr_domain_sl(tree->config, "sl", false);
    add_attr_domain_segments(tree->config, "seg", false);
    add_attr_domain_frequency(tree->config, "frequency_caps", false);

    const char* exprs[] = { "ie = 2", "sl is empty", "seg is empty", "frequency_caps is empty" };
    betree_bulk_insert(tree, exprs, 4);
    betree_make_sub_ids(tree);

    struct betree_event* event = betree_make_event_err(tree);
    betree_set_variable(event, 0, betree_make_integer_variable("ie", 2));
    betree_set_variable(event, 1, betree_make_integer_list_variable("sl", betree_make_integer_list(0)));
    betree_set_variable(event, 2, betree_make_integer_list_variable("seg", betree_make_integer_list(0)));
    betree_set_variable(
        event, 3, betree_make_integer_list_variable("frequency_caps", betree_make_integer_list(0)));

    const uint64_t expected[] = {1, 2, 3, 4};
    const uint64_t ids[] = {1, 3, 4};

    struct report_err* first_report = make_report_err(tree);
    mu_assert(betree_search_with_event_err(tree, event, first_report), "");
    mu_assert(assert_report_err_id_set(first_report, expected, 4) == 0, "");

    struct report_err* second_report = make_report_err(tree);
    mu_assert(betree_search_with_event_err(tree, event, second_report), "");
    mu_assert(assert_report_err_id_set(second_report, expected, 4) == 0, "");

    struct report_err* ids_report = make_report_err(tree);
    mu_assert(betree_search_with_event_ids_err(tree, event, ids_report, ids, 3), "");
    {
        const uint64_t expected_ids[] = {1, 3, 4};
        mu_assert(assert_report_err_id_set(ids_report, expected_ids, 3) == 0, "");
    }

    free_report_err(ids_report);
    free_report_err(second_report);
    free_report_err(first_report);
    betree_free_event(event);
    betree_free_err(tree);
    return 0;
}

int test_search_with_event_ids_err_invalid_event_reason()
{
    struct betree_err* tree = betree_make_err();

    make_attr_domains(tree, ATTR_CONFIG_2(ATTR_INT, ATTR_INT_LIST));

    const char* exprs[] = { "i = 1", "il is empty", "i = 2" };
    betree_bulk_insert(tree, exprs, 3);
    betree_make_sub_ids(tree);

    struct betree_event* invalid_event = betree_make_event_err(tree);
    betree_set_variable(invalid_event, 1, betree_make_integer_list_variable("il", betree_make_integer_list(0)));

    struct report_err* report = make_report_err(tree);
    mu_assert(!betree_search_with_event_err(tree, invalid_event, report), "searchFailsValidation");
    mu_assert(report->matched == 0, "noMatches");

    betree_var_t invalid_reason_id
        = ADDITIONAL_REASON(tree->config->attr_domain_count, REASON_INVALID_EVENT);
    dynamic_array_t* invalid_reason = betree_reason_map_get(report->reason_sub_id_list, invalid_reason_id);
    mu_assert(invalid_reason != NULL, "invalidEventReason");
    mu_assert(invalid_reason->size == 3, "allSubsTagged");

    free_report_err(report);
    betree_free_event(invalid_event);

    struct betree_event* invalid_event_ids = betree_make_event_err(tree);
    betree_set_variable(invalid_event_ids, 1, betree_make_integer_list_variable("il", betree_make_integer_list(0)));

    const uint64_t ids[] = {2, 3};
    struct report_err* ids_report = make_report_err(tree);
    mu_assert(!betree_search_with_event_ids_err(tree, invalid_event_ids, ids_report, ids, 2),
        "searchIdsFailsValidation");
    mu_assert(ids_report->matched == 0, "noMatchesIds");

    dynamic_array_t* invalid_reason_ids
        = betree_reason_map_get(ids_report->reason_sub_id_list, invalid_reason_id);
    mu_assert(invalid_reason_ids != NULL, "invalidEventReasonIds");
    mu_assert(invalid_reason_ids->size == 2, "filteredSubsTagged");
    mu_assert(
        ((betree_var_t)invalid_reason_ids->data[0] == (betree_var_t)2
            && (betree_var_t)invalid_reason_ids->data[1] == (betree_var_t)3)
            || ((betree_var_t)invalid_reason_ids->data[0] == (betree_var_t)3
                && (betree_var_t)invalid_reason_ids->data[1] == (betree_var_t)2),
        "goodFilteredReasonSubs");

    free_report_err(ids_report);
    betree_free_event(invalid_event_ids);
    betree_free_err(tree);
    return 0;
}

int test_search_with_event_err_type_mismatch_reason()
{
    struct betree_err* tree = betree_make_err();

    add_attr_domain_bounded_i(tree->config, "age", false, 0, 120);
    add_attr_domain_s(tree->config, "country", false);

    const char* exprs[] = { "age >= 21", "country = \"USA\"", "age >= 30 and country = \"USA\"" };
    betree_bulk_insert(tree, exprs, 3);
    betree_make_sub_ids(tree);

    struct betree_event* invalid_event = betree_make_event_err(tree);
    betree_set_variable(invalid_event, 0, betree_make_string_variable("age", "twenty five"));
    betree_set_variable(invalid_event, 1, betree_make_string_variable("country", "USA"));

    struct report_err* report = make_report_err(tree);
    mu_assert(!betree_search_with_event_err(tree, invalid_event, report), "searchFailsValidation");
    mu_assert(report->matched == 0, "noMatches");

    betree_var_t invalid_reason_id
        = ADDITIONAL_REASON(tree->config->attr_domain_count, REASON_INVALID_EVENT);
    dynamic_array_t* invalid_reason = betree_reason_map_get(report->reason_sub_id_list, invalid_reason_id);
    mu_assert(invalid_reason != NULL, "invalidEventReason");
    mu_assert(invalid_reason->size == 3, "allSubsTagged");

    free_report_err(report);
    betree_free_event(invalid_event);

    struct betree_event* invalid_event_ids = betree_make_event_err(tree);
    betree_set_variable(invalid_event_ids, 0, betree_make_string_variable("age", "twenty five"));
    betree_set_variable(invalid_event_ids, 1, betree_make_string_variable("country", "USA"));

    const uint64_t ids[] = {1, 3};
    struct report_err* ids_report = make_report_err(tree);
    mu_assert(!betree_search_with_event_ids_err(tree, invalid_event_ids, ids_report, ids, 2),
        "searchIdsFailsValidation");
    mu_assert(ids_report->matched == 0, "noMatchesIds");

    dynamic_array_t* invalid_reason_ids
        = betree_reason_map_get(ids_report->reason_sub_id_list, invalid_reason_id);
    mu_assert(invalid_reason_ids != NULL, "invalidEventReasonIds");
    mu_assert(invalid_reason_ids->size == 2, "filteredSubsTagged");

    free_report_err(ids_report);
    betree_free_event(invalid_event_ids);
    betree_free_err(tree);
    return 0;
}

int test_search_with_event_err_special_domain_type_mismatch_reason()
{
    struct betree_err* tree = betree_make_err();

    const struct betree_constant* constants[] = {
        betree_make_integer_constant("flight_id", 10),
    };

    add_attr_domain_segments(tree->config, "seg", false);
    add_attr_domain_frequency(tree->config, "frequency_caps", false);
    add_attr_domain_i(tree->config, "now", false);

    const char* exprs[] = {
        "segment_within(seg, 1, 20)",
        "within_frequency_cap(\"flight\", \"ns\", 100, 0)",
    };
    betree_bulk_insert_with_constants(tree, exprs, 2, constants, 1);
    betree_make_sub_ids(tree);
    betree_free_constant((struct betree_constant*)constants[0]);

    struct betree_event* invalid_event = betree_make_event_err(tree);
    struct betree_integer_list* wrong_seg = betree_make_integer_list(1);
    betree_add_integer(wrong_seg, 0, 1);
    betree_set_variable(invalid_event, 0, betree_make_integer_list_variable("seg", wrong_seg));
    struct betree_string_list* wrong_caps = betree_make_string_list(1);
    betree_add_string(wrong_caps, 0, "bad");
    betree_set_variable(
        invalid_event, 1, betree_make_string_list_variable("frequency_caps", wrong_caps));
    betree_set_variable(invalid_event, 2, betree_make_integer_variable("now", 0));

    struct report_err* report = make_report_err(tree);
    mu_assert(!betree_search_with_event_err(tree, invalid_event, report), "searchFailsValidation");
    mu_assert(report->matched == 0, "noMatches");

    betree_var_t invalid_reason_id
        = ADDITIONAL_REASON(tree->config->attr_domain_count, REASON_INVALID_EVENT);
    dynamic_array_t* invalid_reason = betree_reason_map_get(report->reason_sub_id_list, invalid_reason_id);
    mu_assert(invalid_reason != NULL, "invalidEventReason");
    mu_assert(invalid_reason->size == 2, "allSubsTagged");

    free_report_err(report);
    betree_free_event(invalid_event);
    betree_free_err(tree);
    return 0;
}


int test_malformed_json_invalid_event_reason()
{
    struct betree_err* tree = betree_make_err();
    add_attr_domain_bounded_i(tree->config, "age", false, 0, 120);
    mu_assert(betree_insert_err(tree, 1, "age >= 21"), "insert");
    mu_assert(betree_insert_err(tree, 2, "age < 21"), "insert");

    const char* invalid_events[] = {
        "{",
        "{\"age\":}",
        "{\"age\":25} trailing",
    };
    const uint64_t ids[] = {2};
    const betree_var_t reason_id
        = ADDITIONAL_REASON(tree->config->attr_domain_count, REASON_INVALID_EVENT);

    for(size_t i = 0; i < sizeof(invalid_events) / sizeof(invalid_events[0]); i++) {
        struct report_err* report = make_report_err(tree);
        mu_assert(!betree_search_err(tree, invalid_events[i], report), "malformedSearchRejected");
        dynamic_array_t* reason = betree_reason_map_get(report->reason_sub_id_list, reason_id);
        mu_assert(reason != NULL && reason->size == 2, "allSubsTaggedInvalidEvent");
        free_report_err(report);

        struct report_err* ids_report = make_report_err(tree);
        mu_assert(!betree_search_ids_err(tree, invalid_events[i], ids_report, ids, 1),
            "malformedFilteredSearchRejected");
        dynamic_array_t* ids_reason
            = betree_reason_map_get(ids_report->reason_sub_id_list, reason_id);
        mu_assert(ids_reason != NULL && ids_reason->size == 1, "filteredSubTaggedInvalidEvent");
        free_report_err(ids_report);
    }

    betree_free_err(tree);
    return 0;
}

void make_attr_domains(struct betree_err* tree, size_t config)
{
    make_attr_domains_undefined(tree, config, 0);
}


void make_attr_domains_undefined(struct betree_err* tree, size_t config, size_t config_undefined)
{
    if((1 << ATTR_BOOL) & config)
        add_attr_domain_b(tree->config, "b", ((1 << ATTR_BOOL) & config_undefined));
    if((1 << ATTR_INT) & config)
        add_attr_domain_i(tree->config, "i", ((1 << ATTR_INT) & config_undefined));
    if((1 << ATTR_FLOAT) & config)
        add_attr_domain_f(tree->config, "f", ((1 << ATTR_FLOAT) & config_undefined));
    if((1 << ATTR_STR) & config)
        add_attr_domain_s(tree->config, "s", ((1 << ATTR_STR) & config_undefined));
    if((1 << ATTR_INT_LIST) & config)
        add_attr_domain_il(tree->config, "il", ((1 << ATTR_INT_LIST) & config_undefined));
    if((1 << ATTR_STR_LIST) & config)
        add_attr_domain_sl(tree->config, "sl", ((1 << ATTR_STR_LIST) & config_undefined));
    if((1 << ATTR_SEGMENTS) & config) {
        add_attr_domain_segments(tree->config, "seg", ((1 << ATTR_SEGMENTS) & config_undefined));
    }
    if((1 << ATTR_GEO) & config) {
        add_attr_domain_f(tree->config, "latitude", ((1 << ATTR_GEO) & config_undefined));
        add_attr_domain_f(tree->config, "longitude", ((1 << ATTR_GEO) & config_undefined));
    }
    if((1 << ATTR_FCAP) & config)
        add_attr_domain_frequency(
            tree->config, "frequency_caps", ((1 << ATTR_FCAP) & config_undefined));
    if((1 << ATTR_INT64) & config)
        add_attr_domain_ie(tree->config, "now", ((1 << ATTR_INT64) & config_undefined));
}


void betree_bulk_insert(struct betree_err* tree, const char** exprs, const size_t count)
{
    for(size_t i = 0; i < count; i++) {
        size_t idx = i + 1;
#if defined(DEBUG)
        fprintf(stderr, "betree_insert exprs[%d] ... %s\n", idx, exprs[i]);
#endif
        betree_insert_err(tree, idx, exprs[i]);
    }
}

void betree_bulk_insert_with_constants(struct betree_err* tree,
    const char** exprs,
    size_t exprs_count,
    const struct betree_constant** constants,
    size_t constants_count)
{
    for(size_t i = 0; i < exprs_count; i++) {
        size_t idx = i + 1;
#if defined(DEBUG)
        fprintf(stderr, "betree_insert exprs[%d] ... %s\n", idx, exprs[i]);
#endif
        const struct betree_sub* sub
            = betree_make_sub_err(tree, idx, constants_count, constants, exprs[i]);
        betree_insert_sub_err(tree, sub);
    }
}


int all_tests()
{
    mu_run_test(test_bool_fail);
    mu_run_test(test_int_fail);
    mu_run_test(test_float_fail);
    mu_run_test(test_bin_fail);
    mu_run_test(test_int_list_fail);
    mu_run_test(test_bin_list_fail);
    mu_run_test(test_segments_fail);
    mu_run_test(test_frequency_cap_fail);
    mu_run_test(test_geo_fail);
    mu_run_test(test_int64_fail);

    mu_run_test(test_short_circuit_fail);

    mu_run_test(test_multiple_bool_exprs_fail);

    mu_run_test(test_memoize_fail);
    mu_run_test(test_memoize_fail_shared_reason);
    mu_run_test(test_memoize_fail_shared_reason_ids);

    mu_run_test(test_all_search_term);

    mu_run_test(test_event_search_reason);
    mu_run_test(test_excluded_branch_reason);
    mu_run_test(test_search_ids_err_excluded_and_evaluated_reasons);
    mu_run_test(test_allow_undefined_reason_search_by_domain);
    mu_run_test(test_disallow_undefined_reason_invalid_event);
    mu_run_test(test_search_err_match_set_parity);
    mu_run_test(test_search_with_event_err_parity);
    mu_run_test(test_search_with_event_err_special_domain_parity);
    mu_run_test(test_search_with_event_err_conversion_invariants);
    mu_run_test(test_search_with_event_err_reuse_after_normalization);
    mu_run_test(test_search_with_event_ids_err_invalid_event_reason);
    mu_run_test(test_search_with_event_err_type_mismatch_reason);
    mu_run_test(test_search_with_event_err_special_domain_type_mismatch_reason);
    mu_run_test(test_malformed_json_invalid_event_reason);

    return 0;
}


RUN_TESTS()
