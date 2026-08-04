#include <cctype>
#include <cmath>
#include <string_view>

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "alloc.h"
#include "ast.h"
#include "ast_eval_shared.hpp"
#include "ast_match_shared.hpp"
#include "betree.h"
#include "error.h"
#include "hashmap.h"
#include "memoize.h"
#include "printer.h"
#include "special.h"
#include "tree.h"
#include "utils.h"
#include "value.h"
#include "var.h"

extern "C" {

struct ast_node* ast_node_create(void)
{
    struct ast_node* node = static_cast<struct ast_node*>(bcalloc(sizeof(struct ast_node)));
    if(node == nullptr) {
        std::fprintf(stderr, "%s bcalloc failed", __func__);
        std::abort();
    }
    node->global_id = INVALID_PRED;
    node->memoize_id = INVALID_PRED;
    return node;
}

struct ast_node* ast_is_null_expr_create(enum ast_is_null_e type, const char* name)
{
    struct ast_node* node = ast_node_create();
    node->type = AST_TYPE_IS_NULL_EXPR;
    node->is_null_expr.type = type;
    node->is_null_expr.attr_var = make_attr_var(name, nullptr);
    return node;
}

struct ast_node* ast_compare_expr_create(
    enum ast_compare_e op, const char* name, struct compare_value value)
{
    struct ast_node* node = ast_node_create();
    node->type = AST_TYPE_COMPARE_EXPR;
    node->compare_expr.op = op;
    node->compare_expr.attr_var = make_attr_var(name, nullptr);
    node->compare_expr.value = value;
    return node;
}

struct ast_node* ast_equality_expr_create(
    enum ast_equality_e op, const char* name, struct equality_value value)
{
    struct ast_node* node = ast_node_create();
    node->type = AST_TYPE_EQUALITY_EXPR;
    node->equality_expr.op = op;
    node->equality_expr.attr_var = make_attr_var(name, nullptr);
    node->equality_expr.value = value;
    return node;
}

static struct ast_node* ast_bool_expr_create(enum ast_bool_e op)
{
    struct ast_node* node = ast_node_create();
    node->type = AST_TYPE_BOOL_EXPR;
    node->bool_expr.op = op;
    return node;
}

struct ast_node* ast_bool_expr_literal_create(bool value)
{
    struct ast_node* node = ast_bool_expr_create(AST_BOOL_LITERAL);
    node->bool_expr.literal = value;
    return node;
}

struct ast_node* ast_bool_expr_variable_create(const char* name)
{
    struct ast_node* node = ast_bool_expr_create(AST_BOOL_VARIABLE);
    node->bool_expr.variable = make_attr_var(name, nullptr);
    return node;
}

struct ast_node* ast_bool_expr_unary_create(struct ast_node* expr)
{
    struct ast_node* node = ast_bool_expr_create(AST_BOOL_NOT);
    node->bool_expr.unary.expr = expr;
    return node;
}

// Instant here means simply fetching a variable
enum scores {
    instant_cost = 0,
    variable_fetch_cost = 1,
    single_length_cost = 5,
    double_length_cost = 10,
    complex_cost = 20,
};

static std::uint64_t score_node(struct ast_node* node)
{
    switch(node->type) {
        case AST_TYPE_IS_NULL_EXPR: return instant_cost;
        case AST_TYPE_COMPARE_EXPR: return variable_fetch_cost;
        case AST_TYPE_EQUALITY_EXPR: return variable_fetch_cost;
        case AST_TYPE_BOOL_EXPR:
            switch(node->bool_expr.op) {
                case AST_BOOL_VARIABLE: return variable_fetch_cost;
                case AST_BOOL_LITERAL: return instant_cost;
                case AST_BOOL_OR:
                case AST_BOOL_AND:
                    // lhs + rhs
                    return score_node(node->bool_expr.binary.lhs)
                         + score_node(node->bool_expr.binary.rhs);
                case AST_BOOL_NOT:
                    // expr
                    return score_node(node->bool_expr.unary.expr);
                default:
                    std::abort();
            }
        case AST_TYPE_SET_EXPR: return single_length_cost;
        case AST_TYPE_LIST_EXPR: return double_length_cost;
        case AST_TYPE_SPECIAL_EXPR:
            switch(node->special_expr.type) {
                case AST_SPECIAL_FREQUENCY: return complex_cost;
                case AST_SPECIAL_SEGMENT: return complex_cost;
                case AST_SPECIAL_GEO: return instant_cost;
                case AST_SPECIAL_STRING: return complex_cost;
                default: std::abort();
            }
        default: std::abort();
    }
}

struct ast_node* ast_bool_expr_binary_create(
    enum ast_bool_e op, struct ast_node* lhs, struct ast_node* rhs)
{
    struct ast_node* node = ast_bool_expr_create(op);
    std::uint64_t lhs_score = score_node(lhs);
    std::uint64_t rhs_score = score_node(rhs);
    if(rhs_score < lhs_score) {
        node->bool_expr.binary.lhs = rhs;
        node->bool_expr.binary.rhs = lhs;
    }
    else {
        node->bool_expr.binary.lhs = lhs;
        node->bool_expr.binary.rhs = rhs;
    }
    return node;
}

struct ast_node* ast_set_expr_create(
    enum ast_set_e op, struct set_left_value left_value, struct set_right_value right_value)
{
    struct ast_node* node = ast_node_create();
    node->type = AST_TYPE_SET_EXPR;
    node->set_expr.op = op;
    node->set_expr.left_value = left_value;
    node->set_expr.right_value = right_value;
    return node;
}

struct ast_node* ast_list_expr_create(
    enum ast_list_e op, const char* name, struct list_value list_value)
{
    struct ast_node* node = ast_node_create();
    node->type = AST_TYPE_LIST_EXPR;
    node->list_expr.op = op;
    node->list_expr.attr_var = make_attr_var(name, nullptr);
    node->list_expr.value = list_value;
    return node;
}

struct ast_node* ast_special_expr_create(void)
{
    struct ast_node* node = ast_node_create();
    node->type = AST_TYPE_SPECIAL_EXPR;
    return node;
}

struct ast_node* ast_special_frequency_create(const enum ast_special_frequency_e op,
    const char* stype,
    struct string_value ns,
    std::int64_t value,
    std::size_t length)
{
    enum frequency_type_e type = get_type_from_string(stype);
    struct ast_node* node = ast_special_expr_create();
    struct ast_special_frequency frequency = {};
    frequency.op = op;
    frequency.attr_var = make_attr_var("frequency_caps", nullptr);
    frequency.type = type;
    frequency.ns = ns;
    frequency.value = value;
    frequency.length = length;
    frequency.id = UINT32_MAX;
    node->special_expr.type = AST_SPECIAL_FREQUENCY;
    node->special_expr.frequency = frequency;
    node->special_expr.frequency.now = make_attr_var("now", nullptr);
    return node;
}

struct ast_node* ast_special_segment_create(
    const enum ast_special_segment_e op, const char* name, betree_seg_t segment_id, std::int64_t seconds)
{
    struct ast_node* node = ast_special_expr_create();
    struct ast_special_segment segment = {};
    segment.op = op;
    segment.segment_id = segment_id;
    segment.seconds = seconds;
    if(name == nullptr) {
        segment.has_variable = false;
        segment.attr_var = make_attr_var("segments_with_timestamp", nullptr);
    }
    else {
        segment.has_variable = true;
        segment.attr_var = make_attr_var(name, nullptr);
    }
    node->special_expr.type = AST_SPECIAL_SEGMENT;
    node->special_expr.segment = segment;
    node->special_expr.segment.now = make_attr_var("now", nullptr);
    return node;
}

struct ast_node* ast_special_geo_create(const enum ast_special_geo_e op,
    double latitude,
    double longitude,
    bool has_radius,
    double radius)
{
    struct ast_node* node = ast_special_expr_create();
    struct ast_special_geo geo = { .op = op,
        .has_radius = has_radius,
        .latitude = latitude,
        .longitude = longitude,
        .radius = radius,
        .latitude_var = make_attr_var("latitude", nullptr),
        .longitude_var = make_attr_var("longitude", nullptr) };
    node->special_expr.type = AST_SPECIAL_GEO;
    node->special_expr.geo = geo;
    return node;
}

struct ast_node* ast_special_string_create(
    const enum ast_special_string_e op, const char* name, const char* pattern)
{
    struct ast_node* node = ast_special_expr_create();
    struct ast_special_string string
        = { .op = op, .attr_var = make_attr_var(name, nullptr), .pattern = bstrdup(pattern) };
    node->special_expr.type = AST_SPECIAL_STRING;
    node->special_expr.string = string;
    return node;
}

static void free_special_expr(struct ast_special_expr special_expr)
{
    switch(special_expr.type) {
        case AST_SPECIAL_FREQUENCY:
            free_attr_var(special_expr.frequency.attr_var);
            free_attr_var(special_expr.frequency.now);
            bfree((char*)special_expr.frequency.ns.string);
            break;
        case AST_SPECIAL_SEGMENT:
            free_attr_var(special_expr.segment.attr_var);
            free_attr_var(special_expr.segment.now);
            break;
        case AST_SPECIAL_GEO:
            free_attr_var(special_expr.geo.latitude_var);
            free_attr_var(special_expr.geo.longitude_var);
            break;
        case AST_SPECIAL_STRING:
            free_attr_var(special_expr.string.attr_var);
            bfree((char*)special_expr.string.pattern);
            break;
        default: std::abort();
    }
}

static void free_set_expr(struct ast_set_expr set_expr)
{
    switch(set_expr.left_value.value_type) {
        case AST_SET_LEFT_VALUE_INTEGER: {
            break;
        }
        case AST_SET_LEFT_VALUE_STRING: {
            bfree((char*)set_expr.left_value.string_value.string);
            break;
        }
        case AST_SET_LEFT_VALUE_VARIABLE: {
            free_attr_var(set_expr.left_value.variable_value);
            break;
        }
        default: std::abort();
    }
    switch(set_expr.right_value.value_type) {
        case AST_SET_RIGHT_VALUE_INTEGER_LIST: {
            free_integer_list(set_expr.right_value.integer_list_value);
            break;
        }
        case AST_SET_RIGHT_VALUE_STRING_LIST: {
            free_string_list(set_expr.right_value.string_list_value);
            break;
        }
        case AST_SET_RIGHT_VALUE_VARIABLE: {
            free_attr_var(set_expr.right_value.variable_value);
            break;
        }
        default: std::abort();
    }
}

static void free_list_expr(struct ast_list_expr list_expr)
{
    switch(list_expr.value.value_type) {
        case AST_LIST_VALUE_INTEGER_LIST: {
            free_integer_list(list_expr.value.integer_list_value);
            break;
        }
        case AST_LIST_VALUE_STRING_LIST: {
            free_string_list(list_expr.value.string_list_value);
            break;
        }
        default: std::abort();
    }
    free_attr_var(list_expr.attr_var);
}

void free_ast_node(struct ast_node* node)
{
    if(node == nullptr) {
        return;
    }
    switch(node->type) {
        case AST_TYPE_IS_NULL_EXPR:
            free_attr_var(node->is_null_expr.attr_var);
            break;
        case AST_TYPE_SPECIAL_EXPR:
            free_special_expr(node->special_expr);
            break;
        case AST_TYPE_COMPARE_EXPR:
            free_attr_var(node->compare_expr.attr_var);
            break;
        case AST_TYPE_EQUALITY_EXPR:
            free_attr_var(node->equality_expr.attr_var);
            if(node->equality_expr.value.value_type == AST_EQUALITY_VALUE_STRING) {
                bfree((char*)node->equality_expr.value.string_value.string);
            }
            break;
        case AST_TYPE_BOOL_EXPR:
            switch(node->bool_expr.op) {
                case AST_BOOL_NOT:
                    free_ast_node(node->bool_expr.unary.expr);
                    break;
                case AST_BOOL_OR:
                case AST_BOOL_AND:
                    free_ast_node(node->bool_expr.binary.lhs);
                    free_ast_node(node->bool_expr.binary.rhs);
                    break;
                case AST_BOOL_VARIABLE:
                    free_attr_var(node->bool_expr.variable);
                    break;
                case AST_BOOL_LITERAL:
                    break;
                default: std::abort();
            }
            break;
        case AST_TYPE_SET_EXPR:
            free_set_expr(node->set_expr);
            break;
        case AST_TYPE_LIST_EXPR:
            free_list_expr(node->list_expr);
            break;
        default: std::abort();
    }
    bfree(node);
}

static void invalid_expr(const char* msg)
{
    std::fprintf(stderr, "%s", msg);
    std::abort();
}

static bool integer_in_integer_list_counting(std::int64_t integer, struct betree_integer_list* list,
    int* ops_count)
{
    return ast_match_integer_in_integer_list_counting(integer, list, ops_count);
}

static bool string_in_string_list_counting(struct string_value string, struct betree_string_list* list,
    int* ops_count)
{
    return ast_match_string_in_string_list_counting(string, list, ops_count);
}

static bool compare_value_matches(enum ast_compare_value_e a, enum betree_value_type_e b)
{
    return (a == AST_COMPARE_VALUE_INTEGER && b == BETREE_INTEGER)
        || (a == AST_COMPARE_VALUE_FLOAT && b == BETREE_FLOAT);
}

static bool equality_value_matches(enum ast_equality_value_e a, enum betree_value_type_e b)
{
    return (a == AST_EQUALITY_VALUE_INTEGER && b == BETREE_INTEGER)
        || (a == AST_EQUALITY_VALUE_FLOAT && b == BETREE_FLOAT)
        || (a == AST_EQUALITY_VALUE_STRING && b == BETREE_STRING)
        || (a == AST_EQUALITY_VALUE_INTEGER_ENUM && b == BETREE_INTEGER_ENUM);
}

static bool list_value_matches(enum ast_list_value_e a, enum betree_value_type_e b)
{
    return (a == AST_LIST_VALUE_INTEGER_LIST && b == BETREE_INTEGER_LIST)
        || (a == AST_LIST_VALUE_STRING_LIST && b == BETREE_STRING_LIST);
}

const char* frequency_type_to_string(enum frequency_type_e type)
{
    const char* string;
    switch(type) {
        case FREQUENCY_TYPE_ADVERTISER: {
            string = "advertiser";
            break;
        }
        case FREQUENCY_TYPE_ADVERTISERIP: {
            string = "advertiser:ip";
            break;
        }
        case FREQUENCY_TYPE_CAMPAIGN: {
            string = "campaign";
            break;
        }
        case FREQUENCY_TYPE_CAMPAIGNIP: {
            string = "campaign:ip";
            break;
        }
        case FREQUENCY_TYPE_CAMPAIGN_GROUP: {
            string = "campaign_group";
            break;
        }
        case FREQUENCY_TYPE_CAMPAIGN_GROUPIP: {
            string = "campaign_group:ip";
            break;
        }
        case FREQUENCY_TYPE_FLIGHT: {
            string = "flight";
            break;
        }
        case FREQUENCY_TYPE_FLIGHTIP: {
            string = "flight:ip";
            break;
        }
        case FREQUENCY_TYPE_PRODUCT: {
            string = "product";
            break;
        }
        case FREQUENCY_TYPE_PRODUCTIP: {
            string = "product:ip";
            break;
        }
        case FREQUENCY_TYPE_INVALID:
        default: 
            std::abort();
    }
    return string;
}

static bool match_special_expr(
    const struct betree_variable** preds, const struct ast_special_expr special_expr)
{
    switch(special_expr.type) {
        case AST_SPECIAL_FREQUENCY: {
            switch(special_expr.frequency.op) {
                case AST_SPECIAL_WITHINFREQUENCYCAP: {
                    const struct ast_special_frequency* f = &special_expr.frequency;
                    struct betree_frequency_caps* caps;
                    bool is_caps_defined = get_frequency_var(f->attr_var.var, preds, &caps);
                    if(is_caps_defined == false) {
                        return false;
                    }
                    if(caps->size == 0) {
                        // Optimization from looking at what within_frequency_caps does
                        return true;
                    }
                    std::int64_t now;
                    bool is_now_defined = get_integer_var(f->now.var, preds, &now);
                    if(is_now_defined == false) {
                        return false;
                    }
                    return within_frequency_caps(
                        caps, f->type, f->id, f->ns, f->value, f->length, now);
                }
                default: std::abort();
            }
        }
        case AST_SPECIAL_SEGMENT: {
            const struct ast_special_segment* s = &special_expr.segment;
            struct betree_segments* segments;
            bool is_segment_defined = get_segments_var(s->attr_var.var, preds, &segments);
            if(is_segment_defined == false) {
                return false;
            }
            std::int64_t now;
            bool is_now_defined = get_integer_var(s->now.var, preds, &now);
            if(is_now_defined == false) {
                return false;
            }
            switch(special_expr.segment.op) {
                case AST_SPECIAL_SEGMENTWITHIN:
                    return segment_within(s->segment_id, s->seconds, segments, now);
                case AST_SPECIAL_SEGMENTBEFORE:
                    return segment_before(s->segment_id, s->seconds, segments, now);
                default: std::abort();
            }
        }
        case AST_SPECIAL_GEO: {
            switch(special_expr.geo.op) {
                case AST_SPECIAL_GEOWITHINRADIUS: {
                    const struct ast_special_geo* g = &special_expr.geo;
                    double latitude_var, longitude_var;
                    bool is_latitude_defined
                        = get_float_var(g->latitude_var.var, preds, &latitude_var);
                    bool is_longitude_defined
                        = get_float_var(g->longitude_var.var, preds, &longitude_var);
                    if(is_latitude_defined == false || is_longitude_defined == false) {
                        return false;
                    }

                    return geo_within_radius(
                        g->latitude, g->longitude, latitude_var, longitude_var, g->radius);
                }
                default: std::abort();
            }
            return false;
        }
        case AST_SPECIAL_STRING: {
            const struct ast_special_string* s = &special_expr.string;
            struct string_value value;
            bool is_string_defined = get_string_var(s->attr_var.var, preds, &value);
            if(is_string_defined == false) {
                return false;
            }
            switch(s->op) {
                case AST_SPECIAL_CONTAINS:
                    return contains(value.string, s->pattern);
                case AST_SPECIAL_STARTSWITH:
                    return starts_with(value.string, s->pattern);
                case AST_SPECIAL_ENDSWITH:
                    return ends_with(value.string, s->pattern);
                default: std::abort();
            }
            return false;
        }
        default: std::abort();
    }
}

static bool match_special_expr_counting(
    const struct betree_variable** preds, const struct ast_special_expr special_expr,
    int* ops_count)
{
    (*ops_count)++;
    switch(special_expr.type) {
        case AST_SPECIAL_FREQUENCY: {
            switch(special_expr.frequency.op) {
                case AST_SPECIAL_WITHINFREQUENCYCAP: {
                    const struct ast_special_frequency* f = &special_expr.frequency;
                    struct betree_frequency_caps* caps;
                    bool is_caps_defined = get_frequency_var(f->attr_var.var, preds, &caps);
                    if(is_caps_defined == false) {
                        return false;
                    }
                    if(caps->size == 0) {
                        // Optimization from looking at what within_frequency_caps does
                        return true;
                    }
                    std::int64_t now;
                    bool is_now_defined = get_integer_var(f->now.var, preds, &now);
                    if(is_now_defined == false) {
                        return false;
                    }
                    return within_frequency_caps_counting(
                        caps, f->type, f->id, f->ns, f->value, f->length, now, ops_count);
                }
                default: std::abort();
            }
        }
        case AST_SPECIAL_SEGMENT: {
            const struct ast_special_segment* s = &special_expr.segment;
            struct betree_segments* segments;
            bool is_segment_defined = get_segments_var(s->attr_var.var, preds, &segments);
            if(is_segment_defined == false) {
                return false;
            }
            std::int64_t now;
            bool is_now_defined = get_integer_var(s->now.var, preds, &now);
            if(is_now_defined == false) {
                return false;
            }
            switch(special_expr.segment.op) {
                case AST_SPECIAL_SEGMENTWITHIN:
                    return segment_within_counting(s->segment_id, s->seconds, segments, now,
                        ops_count);
                case AST_SPECIAL_SEGMENTBEFORE:
                    return segment_before_counting(s->segment_id, s->seconds, segments, now,
                        ops_count);
                default: std::abort();
            }
        }
        case AST_SPECIAL_GEO: {
            switch(special_expr.geo.op) {
                case AST_SPECIAL_GEOWITHINRADIUS: {
                    const struct ast_special_geo* g = &special_expr.geo;
                    double latitude_var, longitude_var;
                    bool is_latitude_defined
                        = get_float_var(g->latitude_var.var, preds, &latitude_var);
                    bool is_longitude_defined
                        = get_float_var(g->longitude_var.var, preds, &longitude_var);
                    if(is_latitude_defined == false || is_longitude_defined == false) {
                        return false;
                    }

                    (*ops_count)++;
                    return geo_within_radius(
                        g->latitude, g->longitude, latitude_var, longitude_var, g->radius);
                }
                default: std::abort();
            }
            return false;
        }
        case AST_SPECIAL_STRING: {
            const struct ast_special_string* s = &special_expr.string;
            struct string_value value;
            bool is_string_defined = get_string_var(s->attr_var.var, preds, &value);
            if(is_string_defined == false) {
                return false;
            }
            (*ops_count)++;
            switch(s->op) {
                case AST_SPECIAL_CONTAINS:
                    return contains(value.string, s->pattern);
                case AST_SPECIAL_STARTSWITH:
                    return starts_with(value.string, s->pattern);
                case AST_SPECIAL_ENDSWITH:
                    return ends_with(value.string, s->pattern);
                default: std::abort();
            }
            return false;
        }
        default: std::abort();
    }
}

// Returns index i; 0 <= i <= count. 
// If x is among arr values returns index of element in arr with value equal to x
// If there is not such element returns index i such that:
//   if all elements in arr are less then x returns count (i.e the array's length)
//   else returns i such that then arr[i-1] < x < arr[i]
std::size_t next_low(const std::int64_t arr[], std::size_t low, std::size_t count, std::int64_t x)
{
    return ast_match_next_low(arr, low, count, x);
}

static bool match_not_all_of_int_counting(struct value variable, struct ast_list_expr list_expr,
    int* ops_count)
{
    return ast_match_not_all_of_int_counting(variable, list_expr, ops_count);
}

static bool match_not_all_of_string_counting(struct value variable, struct ast_list_expr list_expr,
    int* ops_count)
{
    return ast_match_not_all_of_string_counting(variable, list_expr, ops_count);
}

static bool match_all_of_int_counting(struct value variable, struct ast_list_expr list_expr,
    int* ops_count)
{
    return ast_match_all_of_int_counting(variable, list_expr, ops_count);
}

static bool match_all_of_string_counting(struct value variable, struct ast_list_expr list_expr,
    int* ops_count)
{
    return ast_match_all_of_string_counting(variable, list_expr, ops_count);
}

static bool match_list_expr(
    const struct betree_variable** preds, const struct ast_list_expr list_expr)
{
    return ast_eval_shared::match_list_expr(preds, list_expr, [](betree_var_t) {});
}

static bool match_list_expr_counting(
    const struct betree_variable** preds, const struct ast_list_expr list_expr,
    int* ops_count)
{
    (*ops_count)++;
    struct value variable;
    bool is_variable_defined = get_variable(list_expr.attr_var.var, preds, &variable);
    if(is_variable_defined == false) {
        return false;
    }
    switch(list_expr.op) {
        case AST_LIST_ONE_OF:
        case AST_LIST_NONE_OF: {
            bool result = false;
            switch(list_expr.value.value_type) {
                case AST_LIST_VALUE_INTEGER_LIST:
                    result = match_not_all_of_int_counting(variable, list_expr, ops_count);
                    break;
                case AST_LIST_VALUE_STRING_LIST: {
                    result = match_not_all_of_string_counting(variable, list_expr, ops_count);
                    break;
                }
                default: std::abort();
            }
            switch(list_expr.op) {
                case AST_LIST_ONE_OF:
                    return result;
                case AST_LIST_NONE_OF:
                    return !result;
                case AST_LIST_ALL_OF:
                    invalid_expr("Should never happen");
                    return false;
                default: std::abort();
            }
        }
        case AST_LIST_ALL_OF: {
            switch(list_expr.value.value_type) {
                case AST_LIST_VALUE_INTEGER_LIST:
                    return match_all_of_int_counting(variable, list_expr, ops_count);
                case AST_LIST_VALUE_STRING_LIST:
                    return match_all_of_string_counting(variable, list_expr, ops_count);
                default: std::abort();
            }
        }
        default: std::abort();
    }
}

static bool match_set_expr(
    const struct betree_variable** preds, const struct ast_set_expr set_expr, struct report* report)
{
    return ast_eval_shared::match_set_expr(preds, set_expr, [report](betree_var_t var) {
        if(report != nullptr && report->cb) {
            report->last_var = var;
        }
    });
}

static bool match_set_expr_counting(const struct betree_variable** preds, const struct ast_set_expr set_expr,
    int* ops_count)
{
    (*ops_count)++;
    struct set_left_value left = set_expr.left_value;
    struct set_right_value right = set_expr.right_value;
    bool is_in;
    if(left.value_type == AST_SET_LEFT_VALUE_INTEGER
        && right.value_type == AST_SET_RIGHT_VALUE_VARIABLE) {
        struct betree_integer_list* variable;
        bool is_variable_defined = get_integer_list_var(right.variable_value.var, preds, &variable);
        if(is_variable_defined == false) {
            return false;
        }
        is_in = integer_in_integer_list_counting(left.integer_value, variable, ops_count);
    }
    else if(left.value_type == AST_SET_LEFT_VALUE_STRING
        && right.value_type == AST_SET_RIGHT_VALUE_VARIABLE) {
        struct betree_string_list* variable;
        bool is_variable_defined = get_string_list_var(right.variable_value.var, preds, &variable);
        if(is_variable_defined == false) {
            return false;
        }
        is_in = string_in_string_list_counting(left.string_value, variable, ops_count);
    }
    else if(left.value_type == AST_SET_LEFT_VALUE_VARIABLE
        && right.value_type == AST_SET_RIGHT_VALUE_INTEGER_LIST) {
        std::int64_t variable;
        bool is_variable_defined = get_integer_var(left.variable_value.var, preds, &variable);
        if(is_variable_defined == false) {
            return false;
        }
        is_in = integer_in_integer_list_counting(variable, right.integer_list_value, ops_count);
    }
    else if(left.value_type == AST_SET_LEFT_VALUE_VARIABLE
        && right.value_type == AST_SET_RIGHT_VALUE_STRING_LIST) {
        struct string_value variable;
        bool is_variable_defined = get_string_var(left.variable_value.var, preds, &variable);
        if(is_variable_defined == false) {
            return false;
        }
        is_in = string_in_string_list_counting(variable, right.string_list_value, ops_count);
    }
    else {
        invalid_expr("invalid set expression");
        return false;
    }
    switch(set_expr.op) {
        case AST_SET_NOT_IN: {
            return !is_in;
        }
        case AST_SET_IN: {
            return is_in;
        }
        default: std::abort();
    }
}

static bool match_compare_expr(
    const struct betree_variable** preds, const struct ast_compare_expr compare_expr)
{
    return ast_eval_shared::match_compare_expr(preds, compare_expr, [](betree_var_t) {});
}

static bool match_equality_expr(
    const struct betree_variable** preds, const struct ast_equality_expr equality_expr)
{
    return ast_eval_shared::match_equality_expr(preds, equality_expr, [](betree_var_t) {});
}

static bool match_node_inner(const struct betree_variable** preds,
    const struct ast_node* node,
    struct memoize* memoize,
    struct report* report);

static bool match_bool_expr(const struct betree_variable** preds,
    const struct ast_bool_expr bool_expr,
    struct memoize* memoize,
    struct report* report)
{
    switch(bool_expr.op) {
        case AST_BOOL_LITERAL:
            return bool_expr.literal;
        case AST_BOOL_AND: {
            bool lhs = match_node_inner(preds, bool_expr.binary.lhs, memoize, report);
            if(lhs == false) {
                return false;
            }
            bool rhs = match_node_inner(preds, bool_expr.binary.rhs, memoize, report);
            return rhs;
        }
        case AST_BOOL_OR: {
            bool lhs = match_node_inner(preds, bool_expr.binary.lhs, memoize, report);
            if(lhs == true) {
                return true;
            }
            bool rhs = match_node_inner(preds, bool_expr.binary.rhs, memoize, report);
            return rhs;
        }
        case AST_BOOL_NOT: {
            bool result = match_node_inner(preds, bool_expr.unary.expr, memoize, report);
            return !result;
        }
        case AST_BOOL_VARIABLE: {
            bool value;
            bool is_variable_defined = get_bool_var(bool_expr.variable.var, preds, &value);
            if (report != nullptr && report->cb) {
                report->last_var = bool_expr.variable.var;
            }
            if(is_variable_defined == false) {
                return false;
            }
            return value;
        }
        default: std::abort();
    }
}

static bool match_bool_expr_counting(const struct betree_variable** preds,
    const struct ast_bool_expr bool_expr,
    struct memoize* memoize,
    struct report_counting* report)
{
    report->ops_count++;
    switch(bool_expr.op) {
        case AST_BOOL_LITERAL:
            return bool_expr.literal;
        case AST_BOOL_AND: {
            bool lhs = match_node_counting(preds, bool_expr.binary.lhs, memoize, report);
            if(lhs == false) {
                return false;
            }
            bool rhs = match_node_counting(preds, bool_expr.binary.rhs, memoize, report);
            return rhs;
        }
        case AST_BOOL_OR: {
            bool lhs = match_node_counting(preds, bool_expr.binary.lhs, memoize, report);
            if(lhs == true) {
                return true;
            }
            bool rhs = match_node_counting(preds, bool_expr.binary.rhs, memoize, report);
            return rhs;
        }
        case AST_BOOL_NOT: {
            bool result = match_node_counting(preds, bool_expr.unary.expr, memoize, report);
            return !result;
        }
        case AST_BOOL_VARIABLE: {
            bool value;
            bool is_variable_defined = get_bool_var(bool_expr.variable.var, preds, &value);
            if(is_variable_defined == false) {
                return false;
            }
            return value;
        }
        default: std::abort();
    }
}

static bool match_is_null_expr(const struct betree_variable** preds,
    const struct ast_is_null_expr is_null_expr)
{
    return ast_eval_shared::match_is_null_expr(preds, is_null_expr, [](betree_var_t) {});
}

static betree_var_t special_expr_var(const struct ast_special_expr *special_expr) {
    switch(special_expr->type) {
        case AST_SPECIAL_FREQUENCY:
            return special_expr->frequency.attr_var.var;
        case AST_SPECIAL_SEGMENT:
            return special_expr->segment.attr_var.var;
        case AST_SPECIAL_GEO:
            return GEO_VAR;
        case AST_SPECIAL_STRING:
            return special_expr->string.attr_var.var;
        default:
            std::abort();
    }
}

static bool match_node_inner(const struct betree_variable** preds,
    const struct ast_node* node,
    struct memoize* memoize,
    struct report* report)
{
    if(node->memoize_id != INVALID_PRED) {
        if(test_bit(memoize->pass, node->memoize_id)) {
            if(report != nullptr) {
                report->memoized++;
                if(report->cb != nullptr && report->memoize_vars != nullptr) {
                    report->last_var = report->memoize_vars[node->memoize_id];
                }
            }
            return true;
        }
        if(test_bit(memoize->fail, node->memoize_id)) {
            if(report != nullptr) {
                report->memoized++;
                if(report->cb != nullptr && report->memoize_vars != nullptr) {
                    report->last_var = report->memoize_vars[node->memoize_id];
                }
            }
            return false;
        }
    }
    bool result;
    switch(node->type) {
        case AST_TYPE_IS_NULL_EXPR:
            result = match_is_null_expr(preds, node->is_null_expr);
            if (report != nullptr && report->cb) {
                report->last_var = node->is_null_expr.attr_var.var;
            }
            break;
        case AST_TYPE_SPECIAL_EXPR: {
            result = match_special_expr(preds, node->special_expr);
            if (report != nullptr && report->cb) {
                report->last_var = special_expr_var(&node->special_expr);
            }
            break;
        }
        case AST_TYPE_BOOL_EXPR: {
            result = match_bool_expr(preds, node->bool_expr, memoize, report);
            break;
        }
        case AST_TYPE_LIST_EXPR: {
            result = match_list_expr(preds, node->list_expr);
            if (report != nullptr && report->cb) {
                report->last_var = node->list_expr.attr_var.var;
            }
            break;
        }
        case AST_TYPE_SET_EXPR: {
            result = match_set_expr(preds, node->set_expr, report);
            break;
        }
        case AST_TYPE_COMPARE_EXPR: {
            result = match_compare_expr(preds, node->compare_expr);
            if (report != nullptr && report->cb) {
                report->last_var = node->compare_expr.attr_var.var;
            }
            break;
        }
        case AST_TYPE_EQUALITY_EXPR: {
            result = match_equality_expr(preds, node->equality_expr);
            if (report != nullptr && report->cb) {
                report->last_var = node->equality_expr.attr_var.var;
            }
            break;
        }
        default: std::abort();
    }
    if(node->memoize_id != INVALID_PRED) {
        if(report != nullptr && report->cb != nullptr && report->memoize_vars != nullptr) {
            report->memoize_vars[node->memoize_id] = report->last_var;
        }
        if(result) {
            set_bit(memoize->pass, node->memoize_id);
        }
        else {
            set_bit(memoize->fail, node->memoize_id);
        }
    }
    return result;
}

bool match_node_counting(const struct betree_variable** preds,
    const struct ast_node* node,
    struct memoize* memoize,
    struct report_counting* report)
{
    report->node_count++;
    if(node->memoize_id != INVALID_PRED) {
        if(test_bit(memoize->pass, node->memoize_id)) {
            if(report != nullptr) {
                report->memoized++;
            }
            return true;
        }
        if(test_bit(memoize->fail, node->memoize_id)) {
            if(report != nullptr) {
                report->memoized++;
            }
            return false;
        }
    }
    bool result;
    switch(node->type) {
        case AST_TYPE_IS_NULL_EXPR:
            report->ops_count++;
            result = match_is_null_expr(preds, node->is_null_expr);
            break;
        case AST_TYPE_SPECIAL_EXPR: {
            result = match_special_expr_counting(preds, node->special_expr, &report->ops_count);
            break;
        }
        case AST_TYPE_BOOL_EXPR: {
            result = match_bool_expr_counting(preds, node->bool_expr, memoize, report);
            break;
        }
        case AST_TYPE_LIST_EXPR: {
            result = match_list_expr_counting(preds, node->list_expr, &report->ops_count);
            break;
        }
        case AST_TYPE_SET_EXPR: {
            result = match_set_expr_counting(preds, node->set_expr, &report->ops_count);
            break;
        }
        case AST_TYPE_COMPARE_EXPR: {
            report->ops_count++;
            result = match_compare_expr(preds, node->compare_expr);
            break;
        }
        case AST_TYPE_EQUALITY_EXPR: {
            report->ops_count++;
            result = match_equality_expr(preds, node->equality_expr);
            break;
        }
        default: std::abort();
    }
    if(node->memoize_id != INVALID_PRED) {
        if(result) {
            set_bit(memoize->pass, node->memoize_id);
        }
        else {
            set_bit(memoize->fail, node->memoize_id);
        }
    }
    return result;
}

bool match_node(const struct betree_variable** preds,
    const struct ast_node* node,
    struct memoize* memoize,
    struct report* report)
{
    return match_node_inner(preds, node, memoize, report);
}

// --- Tri-state evaluation for the flat/continuation search path ---
//
// Reuses the existing match_*_expr helpers above for the actual matching
// logic (they're already correct for the "value known" case); this layer
// only adds the extra check for whether the variable(s) an expression
// touches are BETREE_PRED_UNFETCHED before deferring to them, and threads
// MATCH_UNKNOWN through AND/OR/NOT short-circuit the same way the plain
// evaluator threads bool.

static enum match_result match_node_tri_inner(const struct betree_variable** preds,
    const struct ast_node* node,
    struct memoize* memoize,
    struct report* report);

static inline enum match_result to_result(bool v)
{
    return v ? MATCH_TRUE : MATCH_FALSE;
}

static inline enum match_result check_pred(const struct betree_variable** preds, betree_var_t var)
{
    if(preds[var] == &BETREE_PRED_UNFETCHED) {
        return MATCH_UNKNOWN;
    }
    if(preds[var] == nullptr) {
        return MATCH_FALSE;
    }
    return MATCH_TRUE;
}

static enum match_result match_bool_expr_tri(const struct betree_variable** preds,
    const struct ast_bool_expr bool_expr,
    struct memoize* memoize,
    struct report* report)
{
    switch(bool_expr.op) {
        case AST_BOOL_LITERAL:
            return to_result(bool_expr.literal);
        case AST_BOOL_AND: {
            enum match_result lhs = match_node_tri_inner(preds, bool_expr.binary.lhs, memoize, report);
            if(lhs == MATCH_FALSE) {
                return MATCH_FALSE;
            }
            enum match_result rhs = match_node_tri_inner(preds, bool_expr.binary.rhs, memoize, report);
            if(lhs == MATCH_TRUE) {
                return rhs;
            }
            return rhs == MATCH_FALSE ? MATCH_FALSE : MATCH_UNKNOWN;
        }
        case AST_BOOL_OR: {
            enum match_result lhs = match_node_tri_inner(preds, bool_expr.binary.lhs, memoize, report);
            if(lhs == MATCH_TRUE) {
                return MATCH_TRUE;
            }
            enum match_result rhs = match_node_tri_inner(preds, bool_expr.binary.rhs, memoize, report);
            if(lhs == MATCH_FALSE) {
                return rhs;
            }
            return rhs == MATCH_TRUE ? MATCH_TRUE : MATCH_UNKNOWN;
        }
        case AST_BOOL_NOT: {
            enum match_result result = match_node_tri_inner(preds, bool_expr.unary.expr, memoize, report);
            if(result == MATCH_UNKNOWN) {
                return MATCH_UNKNOWN;
            }
            return result == MATCH_TRUE ? MATCH_FALSE : MATCH_TRUE;
        }
        case AST_BOOL_VARIABLE: {
            betree_var_t var = bool_expr.variable.var;
            if(report != nullptr && report->cb) {
                report->last_var = var;
            }
            enum match_result cp = check_pred(preds, var);
            if(cp != MATCH_TRUE) {
                return cp;
            }
            bool value;
            get_bool_var(var, preds, &value);
            return to_result(value);
        }
        default: std::abort();
    }
}

static enum match_result match_compare_expr_tri(
    const struct betree_variable** preds, const struct ast_compare_expr compare_expr)
{
    enum match_result cp = check_pred(preds, compare_expr.attr_var.var);
    if(cp != MATCH_TRUE) {
        return cp;
    }
    return to_result(match_compare_expr(preds, compare_expr));
}

static enum match_result match_equality_expr_tri(
    const struct betree_variable** preds, const struct ast_equality_expr equality_expr)
{
    enum match_result cp = check_pred(preds, equality_expr.attr_var.var);
    if(cp != MATCH_TRUE) {
        return cp;
    }
    return to_result(match_equality_expr(preds, equality_expr));
}

static enum match_result match_list_expr_tri(
    const struct betree_variable** preds, const struct ast_list_expr list_expr)
{
    enum match_result cp = check_pred(preds, list_expr.attr_var.var);
    if(cp != MATCH_TRUE) {
        return cp;
    }
    return to_result(match_list_expr(preds, list_expr));
}

static enum match_result match_set_expr_tri(
    const struct betree_variable** preds, const struct ast_set_expr set_expr, struct report* report)
{
    struct set_left_value left = set_expr.left_value;
    struct set_right_value right = set_expr.right_value;
    betree_var_t var;
    if(left.value_type == AST_SET_LEFT_VALUE_INTEGER
        && right.value_type == AST_SET_RIGHT_VALUE_VARIABLE) {
        var = right.variable_value.var;
    }
    else if(left.value_type == AST_SET_LEFT_VALUE_STRING
        && right.value_type == AST_SET_RIGHT_VALUE_VARIABLE) {
        var = right.variable_value.var;
    }
    else if(left.value_type == AST_SET_LEFT_VALUE_VARIABLE
        && right.value_type == AST_SET_RIGHT_VALUE_INTEGER_LIST) {
        var = left.variable_value.var;
    }
    else if(left.value_type == AST_SET_LEFT_VALUE_VARIABLE
        && right.value_type == AST_SET_RIGHT_VALUE_STRING_LIST) {
        var = left.variable_value.var;
    }
    else {
        return MATCH_FALSE;
    }
    enum match_result cp = check_pred(preds, var);
    if(cp != MATCH_TRUE) {
        return cp;
    }
    return to_result(match_set_expr(preds, set_expr, report));
}

static enum match_result match_special_expr_tri(
    const struct betree_variable** preds, const struct ast_special_expr special_expr)
{
    switch(special_expr.type) {
        case AST_SPECIAL_FREQUENCY: {
            enum match_result cp = check_pred(preds, special_expr.frequency.attr_var.var);
            if(cp != MATCH_TRUE) {
                return cp;
            }
            cp = check_pred(preds, special_expr.frequency.now.var);
            if(cp != MATCH_TRUE) {
                return cp;
            }
            break;
        }
        case AST_SPECIAL_SEGMENT: {
            enum match_result cp = check_pred(preds, special_expr.segment.attr_var.var);
            if(cp != MATCH_TRUE) {
                return cp;
            }
            cp = check_pred(preds, special_expr.segment.now.var);
            if(cp != MATCH_TRUE) {
                return cp;
            }
            break;
        }
        case AST_SPECIAL_GEO: {
            enum match_result cp = check_pred(preds, special_expr.geo.latitude_var.var);
            if(cp != MATCH_TRUE) {
                return cp;
            }
            cp = check_pred(preds, special_expr.geo.longitude_var.var);
            if(cp != MATCH_TRUE) {
                return cp;
            }
            break;
        }
        case AST_SPECIAL_STRING: {
            enum match_result cp = check_pred(preds, special_expr.string.attr_var.var);
            if(cp != MATCH_TRUE) {
                return cp;
            }
            break;
        }
        default: std::abort();
    }
    return to_result(match_special_expr(preds, special_expr));
}

static enum match_result match_is_null_expr_tri(
    const struct betree_variable** preds, const struct ast_is_null_expr is_null_expr)
{
    if(preds[is_null_expr.attr_var.var] == &BETREE_PRED_UNFETCHED) {
        return MATCH_UNKNOWN;
    }
    return to_result(match_is_null_expr(preds, is_null_expr));
}

static enum match_result match_node_tri_inner(const struct betree_variable** preds,
    const struct ast_node* node,
    struct memoize* memoize,
    struct report* report)
{
    if(node->memoize_id != INVALID_PRED) {
        if(test_bit(memoize->pass, node->memoize_id)) {
            if(report != nullptr) {
                report->memoized++;
                if(report->cb != nullptr && report->memoize_vars != nullptr) {
                    report->last_var = report->memoize_vars[node->memoize_id];
                }
            }
            return MATCH_TRUE;
        }
        if(test_bit(memoize->fail, node->memoize_id)) {
            if(report != nullptr) {
                report->memoized++;
                if(report->cb != nullptr && report->memoize_vars != nullptr) {
                    report->last_var = report->memoize_vars[node->memoize_id];
                }
            }
            return MATCH_FALSE;
        }
    }
    enum match_result result;
    switch(node->type) {
        case AST_TYPE_IS_NULL_EXPR:
            result = match_is_null_expr_tri(preds, node->is_null_expr);
            if(report != nullptr && report->cb) {
                report->last_var = node->is_null_expr.attr_var.var;
            }
            break;
        case AST_TYPE_SPECIAL_EXPR: {
            result = match_special_expr_tri(preds, node->special_expr);
            if(report != nullptr && report->cb) {
                report->last_var = special_expr_var(&node->special_expr);
            }
            break;
        }
        case AST_TYPE_BOOL_EXPR: {
            result = match_bool_expr_tri(preds, node->bool_expr, memoize, report);
            break;
        }
        case AST_TYPE_LIST_EXPR: {
            result = match_list_expr_tri(preds, node->list_expr);
            if(report != nullptr && report->cb) {
                report->last_var = node->list_expr.attr_var.var;
            }
            break;
        }
        case AST_TYPE_SET_EXPR: {
            result = match_set_expr_tri(preds, node->set_expr, report);
            break;
        }
        case AST_TYPE_COMPARE_EXPR: {
            result = match_compare_expr_tri(preds, node->compare_expr);
            if(report != nullptr && report->cb) {
                report->last_var = node->compare_expr.attr_var.var;
            }
            break;
        }
        case AST_TYPE_EQUALITY_EXPR: {
            result = match_equality_expr_tri(preds, node->equality_expr);
            if(report != nullptr && report->cb) {
                report->last_var = node->equality_expr.attr_var.var;
            }
            break;
        }
        default: std::abort();
    }
    // Memoization only caches settled (true/false) results: an unknown
    // result may become settled once more variables are fetched, and
    // caching it would incorrectly freeze that outcome across yields.
    if(node->memoize_id != INVALID_PRED && result != MATCH_UNKNOWN) {
        if(report != nullptr && report->cb != nullptr && report->memoize_vars != nullptr) {
            report->memoize_vars[node->memoize_id] = report->last_var;
        }
        if(result == MATCH_TRUE) {
            set_bit(memoize->pass, node->memoize_id);
        }
        else {
            set_bit(memoize->fail, node->memoize_id);
        }
    }
    return result;
}

enum match_result match_node_tri(const struct betree_variable** preds,
    const struct ast_node* node,
    struct memoize* memoize,
    struct report* report)
{
    return match_node_tri_inner(preds, node, memoize, report);
}

struct bound_dirty {
    bool min_dirty;
    bool max_dirty;
};

static void get_variable_bound_inner(const struct attr_domain* domain,
    const struct ast_node* node,
    struct value_bound* bound,
    bool is_reversed,
    struct bound_dirty* dirty);

static void get_variable_bound_ast_bool_or(const struct attr_domain* domain,
    const struct ast_node* node,
    struct value_bound* bound,
    bool is_reversed,
    struct bound_dirty* dirty)
{
    struct value_bound lbound = {};
    lbound.value_type = bound->value_type;
    struct bound_dirty ldirty = { .min_dirty = false, .max_dirty = false };
    struct value_bound rbound = {};
    rbound.value_type = bound->value_type;
    struct bound_dirty rdirty = { .min_dirty = false, .max_dirty = false };
    get_variable_bound_inner(
        domain, node->bool_expr.binary.lhs, &lbound, is_reversed, &ldirty);
    get_variable_bound_inner(
        domain, node->bool_expr.binary.rhs, &rbound, is_reversed, &rdirty);
    if(ldirty.min_dirty == true && rdirty.min_dirty == true) {
        dirty->min_dirty = true;
        switch(bound->value_type) {
            case BETREE_BOOLEAN:
                bound->bmin = lbound.bmin && rbound.bmin;
                break;
            case BETREE_INTEGER:
            case BETREE_INTEGER_LIST:
                bound->imin = d64min(lbound.imin, rbound.imin);
                break;
            case BETREE_FLOAT:
                bound->fmin = fmin(lbound.fmin, rbound.fmin);
                break;
            case BETREE_STRING:
            case BETREE_STRING_LIST:
            case BETREE_INTEGER_ENUM:
                bound->smin = smin(lbound.smin, rbound.smin);
                break;
            case BETREE_SEGMENTS:
            case BETREE_FREQUENCY_CAPS:
            default:
                break;
        }
    }
    if(ldirty.max_dirty == true && rdirty.max_dirty == true) {
        dirty->max_dirty = true;
        switch(bound->value_type) {
            case BETREE_BOOLEAN:
                bound->bmax = lbound.bmax || rbound.bmax;
                break;
            case BETREE_INTEGER:
            case BETREE_INTEGER_LIST:
                bound->imax = d64max(lbound.imax, rbound.imax);
                break;
            case BETREE_FLOAT:
                bound->fmax = fmax(lbound.fmax, rbound.fmax);
                break;
            case BETREE_STRING:
            case BETREE_STRING_LIST:
            case BETREE_INTEGER_ENUM:
                bound->smax = smax(lbound.smax, rbound.smax);
                break;
            case BETREE_SEGMENTS:
            case BETREE_FREQUENCY_CAPS:
            default:
                break;
        }
    }
    return;
}

static void get_variable_bound_ast_bool_and(const struct attr_domain* domain,
    const struct ast_node* node,
    struct value_bound* bound,
    bool is_reversed,
    struct bound_dirty* dirty)
{
    struct value_bound lbound = {};
    lbound.value_type = bound->value_type;
    struct bound_dirty ldirty = { .min_dirty = false, .max_dirty = false };
    struct value_bound rbound = {};
    rbound.value_type = bound->value_type;
    struct bound_dirty rdirty = { .min_dirty = false, .max_dirty = false };
    get_variable_bound_inner(
        domain, node->bool_expr.binary.lhs, &lbound, is_reversed, &ldirty);
    get_variable_bound_inner(
        domain, node->bool_expr.binary.rhs, &rbound, is_reversed, &rdirty);
    if(ldirty.min_dirty == true || rdirty.min_dirty == true) {
        dirty->min_dirty = true;
        switch(bound->value_type) {
            case BETREE_BOOLEAN:
                if(ldirty.min_dirty == true && rdirty.min_dirty == true) {
                    bound->bmin = lbound.bmin && rbound.bmin;
                }
                else if(ldirty.min_dirty == true) {
                    bound->bmin = lbound.bmin;
                }
                else {
                    bound->bmin = rbound.bmin;
                }
                break;
            case BETREE_INTEGER:
            case BETREE_INTEGER_LIST:
                if(ldirty.min_dirty == true && rdirty.min_dirty == true) {
                    bound->imin = d64min(lbound.imin, rbound.imin);
                }
                else if(ldirty.min_dirty == true) {
                    bound->imin = lbound.imin;
                }
                else {
                    bound->imin = rbound.imin;
                }
                break;
            case BETREE_FLOAT:
                if(ldirty.min_dirty == true && rdirty.min_dirty == true) {
                    bound->fmin = fmin(lbound.fmin, rbound.fmin);
                }
                else if(ldirty.min_dirty == true) {
                    bound->fmin = lbound.fmin;
                }
                else {
                    bound->fmin = rbound.fmin;
                }
                break;
            case BETREE_STRING:
            case BETREE_STRING_LIST:
            case BETREE_INTEGER_ENUM:
                if(ldirty.min_dirty == true && rdirty.min_dirty == true) {
                    bound->smin = smin(lbound.smin, rbound.smin);
                }
                else if(ldirty.min_dirty == true) {
                    bound->smin = lbound.smin;
                }
                else {
                    bound->smin = rbound.smin;
                }
                break;
            case BETREE_SEGMENTS:
            case BETREE_FREQUENCY_CAPS:
            default:
                break;
        }
    }
    if(ldirty.max_dirty == true || rdirty.max_dirty == true) {
        dirty->max_dirty = true;
        switch(bound->value_type) {
            case BETREE_BOOLEAN:
                if(ldirty.max_dirty == true && rdirty.max_dirty == true) {
                    bound->bmax = lbound.bmax || rbound.bmax;
                }
                else if(ldirty.max_dirty == true) {
                    bound->bmax = lbound.bmax;
                }
                else {
                    bound->bmax = rbound.bmax;
                }
                break;
            case BETREE_INTEGER:
            case BETREE_INTEGER_LIST:
                if(ldirty.max_dirty == true && rdirty.max_dirty == true) {
                    bound->imax = d64max(lbound.imax, rbound.imax);
                }
                else if(ldirty.max_dirty == true) {
                    bound->imax = lbound.imax;
                }
                else {
                    bound->imax = rbound.imax;
                }
                break;
            case BETREE_FLOAT:
                if(ldirty.max_dirty == true && rdirty.max_dirty == true) {
                    bound->fmax = fmax(lbound.fmax, rbound.fmax);
                }
                else if(ldirty.max_dirty == true) {
                    bound->fmax = lbound.fmax;
                }
                else {
                    bound->fmax = rbound.fmax;
                }
                break;
            case BETREE_STRING:
            case BETREE_STRING_LIST:
            case BETREE_INTEGER_ENUM:
                if(ldirty.max_dirty == true && rdirty.max_dirty == true) {
                    bound->smax = smax(lbound.smax, rbound.smax);
                }
                else if(ldirty.max_dirty == true) {
                    bound->smax = lbound.smax;
                }
                else {
                    bound->smax = rbound.smax;
                }
                break;
            case BETREE_SEGMENTS:
            case BETREE_FREQUENCY_CAPS:
            default:
                break;
        }
    }
    return;
}

static void get_variable_bound_inner(const struct attr_domain* domain,
    const struct ast_node* node,
    struct value_bound* bound,
    bool is_reversed,
    struct bound_dirty* dirty)
{
    if(node == nullptr) {
        return;
    }
    switch(node->type) {
        case AST_TYPE_IS_NULL_EXPR:
            return;
        case AST_TYPE_SPECIAL_EXPR:
            return;
        case AST_TYPE_LIST_EXPR:
            if(domain->attr_var.var != node->list_expr.attr_var.var) {
                return;
            }
            if(!list_value_matches(node->list_expr.value.value_type, domain->bound.value_type)) {
                invalid_expr("Domain and expr type mismatch");
                return;
            }
            switch(node->list_expr.op) {
                case AST_LIST_ONE_OF:
                case AST_LIST_ALL_OF:
                    if(domain->attr_var.var != node->list_expr.attr_var.var) {
                        return;
                    }
                    if(domain->bound.value_type == BETREE_INTEGER_LIST
                        && node->list_expr.value.value_type == AST_LIST_VALUE_INTEGER_LIST) {
                        if(is_reversed) {
                        }
                        else {
                            if(node->list_expr.value.integer_list_value->count != 0) {
                                bound->imin = node->list_expr.value.integer_list_value->integers[0];
                                dirty->min_dirty = true;
                                bound->imax
                                    = node->list_expr.value.integer_list_value
                                          ->integers[node->list_expr.value.integer_list_value->count
                                              - 1];
                                dirty->max_dirty = true;
                            }
                            else {
                                return;
                            }
                        }
                        return;
                    }
                    if(domain->bound.value_type == BETREE_STRING_LIST
                        && node->list_expr.value.value_type == AST_LIST_VALUE_STRING_LIST) {
                        if(is_reversed) {
                        }
                        else {
                            if(node->list_expr.value.string_list_value->count != 0) {
                                bound->smin
                                    = node->list_expr.value.string_list_value->strings[0].str;
                                dirty->min_dirty = true;
                                bound->smax
                                    = node->list_expr.value.string_list_value
                                          ->strings[node->list_expr.value.string_list_value->count
                                              - 1]
                                          .str;
                                dirty->max_dirty = true;
                            }
                            else {
                                return;
                            }
                        }
                        return;
                    }
                    invalid_expr("Domain and expr type mismatch");
                    return;
                case AST_LIST_NONE_OF:
                    if(domain->bound.value_type == BETREE_INTEGER_LIST
                        && node->list_expr.value.value_type == AST_LIST_VALUE_INTEGER_LIST) {
                        if(is_reversed) {
                            if(node->list_expr.value.integer_list_value->count != 0) {
                                bound->imin = node->list_expr.value.integer_list_value->integers[0];
                                dirty->min_dirty = true;
                                bound->imax
                                    = node->list_expr.value.integer_list_value
                                          ->integers[node->list_expr.value.integer_list_value->count
                                              - 1];
                                dirty->max_dirty = true;
                            }
                            else {
                                return;
                            }
                        }
                        else {
                        }
                        return;
                    }
                    else if(domain->bound.value_type == BETREE_STRING_LIST
                        && node->list_expr.value.value_type == AST_LIST_VALUE_STRING_LIST) {
                        if(is_reversed) {
                            if(node->list_expr.value.string_list_value->count != 0) {
                                bound->smin
                                    = node->list_expr.value.string_list_value->strings[0].str;
                                dirty->min_dirty = true;
                                bound->smax
                                    = node->list_expr.value.string_list_value
                                          ->strings[node->list_expr.value.string_list_value->count
                                              - 1]
                                          .str;
                                dirty->max_dirty = true;
                            }
                            else {
                                return;
                            }
                        }
                        else {
                        }
                        return;
                    }
                    else {
                        invalid_expr("Domain and expr type mismatch");
                        return;
                    }
                default: std::abort();
            }
        case AST_TYPE_SET_EXPR:
            switch(node->set_expr.op) {
                case AST_SET_IN:
                    if(node->set_expr.left_value.value_type == AST_SET_LEFT_VALUE_VARIABLE) {
                        if(domain->attr_var.var != node->set_expr.left_value.variable_value.var) {
                            return;
                        }
                        if(domain->bound.value_type == BETREE_INTEGER
                            && node->set_expr.right_value.value_type
                                == AST_SET_RIGHT_VALUE_INTEGER_LIST) {
                            if(is_reversed) {
                            }
                            else {
                                if(node->set_expr.right_value.integer_list_value->count != 0) {
                                    bound->imin = node->set_expr.right_value.integer_list_value
                                                      ->integers[0];
                                    dirty->min_dirty = true;
                                    bound->imax = node->set_expr.right_value.integer_list_value
                                                      ->integers[node->set_expr.right_value
                                                                     .integer_list_value->count
                                                          - 1];
                                    dirty->max_dirty = true;
                                }
                                else {
                                    return;
                                }
                            }
                            return;
                        }
                        if(domain->bound.value_type == BETREE_STRING
                            && node->set_expr.right_value.value_type
                                == AST_SET_RIGHT_VALUE_STRING_LIST) {
                            if(is_reversed) {
                            }
                            else {
                                if(node->set_expr.right_value.string_list_value->count != 0) {
                                    bound->smin
                                        = node->set_expr.right_value.string_list_value->strings[0]
                                              .str;
                                    dirty->min_dirty = true;
                                    bound->smax = node->set_expr.right_value.string_list_value
                                                      ->strings[node->set_expr.right_value
                                                                    .string_list_value->count
                                                          - 1]
                                                      .str;
                                    dirty->max_dirty = true;
                                }
                                else {
                                    return;
                                }
                            }
                            return;
                        }
                        invalid_expr("Domain and expr type mismatch");
                        return;
                    }
                    else if(node->set_expr.right_value.value_type == AST_SET_RIGHT_VALUE_VARIABLE) {
                        if(domain->attr_var.var != node->set_expr.right_value.variable_value.var) {
                            return;
                        }
                        if(domain->bound.value_type == BETREE_INTEGER_LIST
                            && node->set_expr.left_value.value_type
                                == AST_SET_LEFT_VALUE_INTEGER) {
                            if(is_reversed) {
                            }
                            else {
                                bound->imin = node->set_expr.left_value.integer_value;
                                dirty->min_dirty = true;
                                bound->imax = node->set_expr.left_value.integer_value;
                                dirty->max_dirty = true;
                            }
                            return;
                        }
                        if(domain->bound.value_type == BETREE_STRING_LIST
                            && node->set_expr.left_value.value_type
                                == AST_SET_LEFT_VALUE_STRING) {
                            if(is_reversed) {
                            }
                            else {
                                bound->smin = node->set_expr.left_value.string_value.str;
                                dirty->min_dirty = true;
                                bound->smax = node->set_expr.left_value.string_value.str;
                                dirty->max_dirty = true;
                            }
                            return;
                        }
                        invalid_expr("Domain and expr type mismatch");
                        return;
                    }
                    invalid_expr("Domain and expr type mismatch");
                    return;
                case AST_SET_NOT_IN:
                    if(node->set_expr.left_value.value_type == AST_SET_LEFT_VALUE_VARIABLE) {
                        if(domain->attr_var.var != node->set_expr.left_value.variable_value.var) {
                            return;
                        }
                        if(domain->bound.value_type == BETREE_INTEGER
                            && node->set_expr.right_value.value_type
                                == AST_SET_RIGHT_VALUE_INTEGER_LIST) {
                            if(is_reversed) {
                                if(node->set_expr.right_value.integer_list_value->count != 0) {
                                    bound->imin = node->set_expr.right_value.integer_list_value
                                                      ->integers[0];
                                    dirty->min_dirty = true;
                                    bound->imax = node->set_expr.right_value.integer_list_value
                                                      ->integers[node->set_expr.right_value
                                                                     .integer_list_value->count
                                                          - 1];
                                    dirty->max_dirty = true;
                                }
                                else {
                                    return;
                                }
                            }
                            else {
                            }
                            return;
                        }
                        if(domain->bound.value_type == BETREE_STRING
                            && node->set_expr.right_value.value_type
                                == AST_SET_RIGHT_VALUE_STRING_LIST) {
                            if(is_reversed) {
                                if(node->set_expr.right_value.string_list_value->count != 0) {
                                    bound->smin
                                        = node->set_expr.right_value.string_list_value->strings[0]
                                              .str;
                                    dirty->min_dirty = true;
                                    bound->smax = node->set_expr.right_value.string_list_value
                                                      ->strings[node->set_expr.right_value
                                                                    .string_list_value->count
                                                          - 1]
                                                      .str;
                                    dirty->max_dirty = true;
                                }
                                else {
                                    return;
                                }
                            }
                            else {
                            }
                            return;
                        }
                        invalid_expr("Domain and expr type mismatch");
                        return;
                    }
                    if(node->set_expr.right_value.value_type == AST_SET_RIGHT_VALUE_VARIABLE) {
                        if(domain->attr_var.var != node->set_expr.right_value.variable_value.var) {
                            return;
                        }
                        if(domain->bound.value_type == BETREE_INTEGER_LIST
                            && node->set_expr.left_value.value_type
                                == AST_SET_LEFT_VALUE_INTEGER) {
                            if(is_reversed) {
                                bound->imin = node->set_expr.left_value.integer_value;
                                dirty->min_dirty = true;
                                bound->imax = node->set_expr.left_value.integer_value;
                                dirty->max_dirty = true;
                            }
                            else {
                            }
                            return;
                        }
                        if(domain->bound.value_type == BETREE_STRING_LIST
                            && node->set_expr.left_value.value_type
                                == AST_SET_LEFT_VALUE_STRING) {
                            if(is_reversed) {
                                bound->smin = node->set_expr.left_value.string_value.str;
                                dirty->min_dirty = true;
                                bound->smax = node->set_expr.left_value.string_value.str;
                                dirty->max_dirty = true;
                            }
                            else {
                            }
                            return;
                        }
                        invalid_expr("Domain and expr type mismatch");
                        return;
                    }
                    invalid_expr("Domain and expr type mismatch");
                    return;
                default: std::abort();
            }
        case AST_TYPE_BOOL_EXPR:
            switch(node->bool_expr.op) {
                case AST_BOOL_LITERAL:
                    return;
                case AST_BOOL_VARIABLE:
                    if(domain->attr_var.var != node->bool_expr.variable.var) {
                        return;
                    }
                    if(domain->bound.value_type != BETREE_BOOLEAN) {
                        invalid_expr("Domain and expr type mismatch");
                        return;
                    }
                    if(is_reversed) {
                        bound->bmin = false;
                        dirty->min_dirty = true;
                        bound->bmax = false;
                        dirty->max_dirty = true;
                    }
                    else {
                        bound->bmin = true;
                        dirty->min_dirty = true;
                        bound->bmax = true;
                        dirty->max_dirty = true;
                    }
                    return;
                case AST_BOOL_NOT:
                    get_variable_bound_inner(
                        domain, node->bool_expr.unary.expr, bound, !is_reversed, dirty);
                    return;
                case AST_BOOL_OR: {
                    if (is_reversed) {
                        get_variable_bound_ast_bool_and(domain, node, bound, is_reversed, dirty);
                    } else {
                        get_variable_bound_ast_bool_or(domain, node, bound, is_reversed, dirty);
                    }
                    return;
                }
                case AST_BOOL_AND: {
                    if (is_reversed) {
                        get_variable_bound_ast_bool_or(domain, node, bound, is_reversed, dirty);
                    } else {
                        get_variable_bound_ast_bool_and(domain, node, bound, is_reversed, dirty);
                    }
                    return;
                }
                default: std::abort();
            }
        case AST_TYPE_EQUALITY_EXPR:
            if(domain->attr_var.var != node->equality_expr.attr_var.var) {
                return;
            }
            if(!equality_value_matches(
                   node->equality_expr.value.value_type, domain->bound.value_type)) {
                invalid_expr("Domain and expr type mismatch");
                return;
            }
            switch(node->equality_expr.op) {
                case AST_EQUALITY_EQ:
                    switch(node->equality_expr.value.value_type) {
                        case AST_EQUALITY_VALUE_INTEGER:
                            if(is_reversed) {
                            }
                            else {
                                bound->imin = node->equality_expr.value.integer_value;
                                dirty->min_dirty = true;
                                bound->imax = node->equality_expr.value.integer_value;
                                dirty->max_dirty = true;
                            }
                            break;
                        case AST_EQUALITY_VALUE_FLOAT:
                            if(is_reversed) {
                            }
                            else {
                                bound->fmin = node->equality_expr.value.float_value;
                                dirty->min_dirty = true;
                                bound->fmax = node->equality_expr.value.float_value;
                                dirty->max_dirty = true;
                            }
                            break;
                        case AST_EQUALITY_VALUE_STRING:
                            if(is_reversed) {
                            }
                            else {
                                bound->smin = node->equality_expr.value.string_value.str;
                                dirty->min_dirty = true;
                                bound->smax = node->equality_expr.value.string_value.str;
                                dirty->max_dirty = true;
                            }
                            break;
                        case AST_EQUALITY_VALUE_INTEGER_ENUM:
                            if(is_reversed) {
                            }
                            else {
                                bound->smin = node->equality_expr.value.integer_enum_value.ienum;
                                dirty->min_dirty = true;
                                bound->smax = node->equality_expr.value.integer_enum_value.ienum;
                                dirty->max_dirty = true;
                            }
                            break;
                        default: std::abort();
                    }
                    break;
                case AST_EQUALITY_NE:
                    switch(node->equality_expr.value.value_type) {
                        case AST_EQUALITY_VALUE_INTEGER:
                            if(is_reversed) {
                                bound->imin = node->equality_expr.value.integer_value;
                                dirty->min_dirty = true;
                                bound->imax = node->equality_expr.value.integer_value;
                                dirty->max_dirty = true;
                            }
                            else {
                            }
                            break;
                        case AST_EQUALITY_VALUE_FLOAT:
                            if(is_reversed) {
                                bound->fmin = node->equality_expr.value.float_value;
                                dirty->min_dirty = true;
                                bound->fmax = node->equality_expr.value.float_value;
                                dirty->max_dirty = true;
                            }
                            else {
                            }
                            break;
                        case AST_EQUALITY_VALUE_STRING:
                            if(is_reversed) {
                                bound->smin = node->equality_expr.value.string_value.str;
                                dirty->min_dirty = true;
                                bound->smax = node->equality_expr.value.string_value.str;
                                dirty->max_dirty = true;
                            }
                            else {
                            }
                            break;
                        case AST_EQUALITY_VALUE_INTEGER_ENUM:
                            if(is_reversed) {
                                bound->smin = node->equality_expr.value.integer_enum_value.ienum;
                                dirty->min_dirty = true;
                                bound->smax = node->equality_expr.value.integer_enum_value.ienum;
                                dirty->max_dirty = true;
                            }
                            else {
                            }
                            break;
                        default: std::abort();
                    }
                    break;
                default: std::abort();
            }
            return;
        case AST_TYPE_COMPARE_EXPR:
            if(domain->attr_var.var != node->compare_expr.attr_var.var) {
                return;
            }
            if(!compare_value_matches(
                   node->compare_expr.value.value_type, domain->bound.value_type)) {
                invalid_expr("Domain and expr type mismatch");
                return;
            }
            switch(node->compare_expr.op) {
                case AST_COMPARE_LT:
                    switch(node->compare_expr.value.value_type) {
                        case AST_COMPARE_VALUE_INTEGER:
                            if(is_reversed) {
                                bound->imin = node->compare_expr.value.integer_value;
                                dirty->min_dirty = true;
                            }
                            else {
                                bound->imax = node->compare_expr.value.integer_value - 1;
                                dirty->max_dirty = true;
                            }
                            break;
                        case AST_COMPARE_VALUE_FLOAT:
                            if(is_reversed) {
                                bound->fmin = node->compare_expr.value.float_value;
                                dirty->min_dirty = true;
                            }
                            else {
                                bound->fmax
                                    = node->compare_expr.value.float_value - __DBL_EPSILON__;
                                dirty->max_dirty = true;
                            }
                            break;
                        default: std::abort();
                    }
                    break;
                case AST_COMPARE_LE:
                    switch(node->compare_expr.value.value_type) {
                        case AST_COMPARE_VALUE_INTEGER:
                            if(is_reversed) {
                                bound->imin = node->compare_expr.value.integer_value + 1;
                                dirty->min_dirty = true;
                            }
                            else {
                                bound->imax = node->compare_expr.value.integer_value;
                                dirty->max_dirty = true;
                            }
                            break;
                        case AST_COMPARE_VALUE_FLOAT:
                            if(is_reversed) {
                                bound->fmin
                                    = node->compare_expr.value.float_value + __DBL_EPSILON__;
                                dirty->min_dirty = true;
                            }
                            else {
                                bound->fmax = node->compare_expr.value.float_value;
                                dirty->max_dirty = true;
                            }
                            break;
                        default: std::abort();
                    }
                    break;
                case AST_COMPARE_GT:
                    switch(node->compare_expr.value.value_type) {
                        case AST_COMPARE_VALUE_INTEGER:
                            if(is_reversed) {
                                bound->imax = node->compare_expr.value.integer_value;
                                dirty->max_dirty = true;
                            }
                            else {
                                bound->imin = node->compare_expr.value.integer_value + 1;
                                dirty->min_dirty = true;
                            }
                            break;
                        case AST_COMPARE_VALUE_FLOAT:
                            if(is_reversed) {
                                bound->fmax = node->compare_expr.value.float_value;
                                dirty->max_dirty = true;
                            }
                            else {
                                bound->fmin
                                    = node->compare_expr.value.float_value + __DBL_EPSILON__;
                                dirty->min_dirty = true;
                            }
                            break;
                        default: std::abort();
                    }
                    break;
                case AST_COMPARE_GE:
                    switch(node->compare_expr.value.value_type) {
                        case AST_COMPARE_VALUE_INTEGER:
                            if(is_reversed) {
                                bound->imax = node->compare_expr.value.integer_value - 1;
                                dirty->max_dirty = true;
                            }
                            else {
                                bound->imin = node->compare_expr.value.integer_value;
                                dirty->min_dirty = true;
                            }
                            break;
                        case AST_COMPARE_VALUE_FLOAT:
                            if(is_reversed) {
                                bound->fmax
                                    = node->compare_expr.value.float_value - __DBL_EPSILON__;
                                dirty->max_dirty = true;
                            }
                            else {
                                bound->fmin = node->compare_expr.value.float_value;
                                dirty->min_dirty = true;
                            }
                            break;
                        default: std::abort();
                    }
                    break;
                default: std::abort();
            }
            return;
        default: std::abort();
    }
}

struct value_bound get_variable_bound(const struct attr_domain* domain, const struct ast_node* node)
{
    struct value_bound bound = {};
    bound.value_type = domain->bound.value_type;
    struct bound_dirty dirty = { .min_dirty = false, .max_dirty = false };
    get_variable_bound_inner(domain, node, &bound, false, &dirty);
    switch(domain->bound.value_type) {
        case BETREE_BOOLEAN:
            if(dirty.min_dirty == false) {
                bound.bmin = domain->bound.bmin;
            }
            if(dirty.max_dirty == false) {
                bound.bmax = domain->bound.bmax;
            }
            break;
        case BETREE_INTEGER:
        case BETREE_INTEGER_LIST:
            if(dirty.min_dirty == false) {
                bound.imin = domain->bound.imin;
            }
            if(dirty.max_dirty == false) {
                bound.imax = domain->bound.imax;
            }
            break;
        case BETREE_FLOAT:
            if(dirty.min_dirty == false) {
                bound.fmin = domain->bound.fmin;
            }
            if(dirty.max_dirty == false) {
                bound.fmax = domain->bound.fmax;
            }
            break;
        case BETREE_STRING:
        case BETREE_STRING_LIST:
        case BETREE_INTEGER_ENUM:
            if(dirty.min_dirty == false) {
                bound.smin = domain->bound.smin;
            }
            if(dirty.max_dirty == false) {
                bound.smax = domain->bound.smax;
            }
            break;
        case BETREE_SEGMENTS:
        case BETREE_FREQUENCY_CAPS:
        default:
            std::fprintf(stderr, "Invalid domain type to get a bound\n");
            std::abort();
    }
    return bound;
}

static bool get_integer_constant(
    const char* name, std::size_t count, const struct betree_constant** constants, std::int64_t* ret)
{
    const struct betree_constant* constant = nullptr;
    for(std::size_t i = 0; i < count; i++) {
        if(strcmp(name, constants[i]->name) == 0) {
            constant = constants[i];
            break;
        }
    }
    if(constant == nullptr || constant->value.value_type != BETREE_INTEGER) {
        return false;
    }
    *ret = constant->value.integer_value;
    return true;
}

static const char* get_constant_name_for_type(enum frequency_type_e type)
{
    switch(type) {
        case FREQUENCY_TYPE_ADVERTISER:
        case FREQUENCY_TYPE_ADVERTISERIP:
            return "advertiser_id";
        case FREQUENCY_TYPE_CAMPAIGN:
        case FREQUENCY_TYPE_CAMPAIGNIP:
            return "campaign_id";
        case FREQUENCY_TYPE_CAMPAIGN_GROUP:
        case FREQUENCY_TYPE_CAMPAIGN_GROUPIP:
            return "campaign_group_id";
        case FREQUENCY_TYPE_FLIGHT:
        case FREQUENCY_TYPE_FLIGHTIP:
            return "flight_id";
        case FREQUENCY_TYPE_PRODUCT:
        case FREQUENCY_TYPE_PRODUCTIP:
            return "product_id";
        case FREQUENCY_TYPE_INVALID:
        default:
            return nullptr;
    };
}

bool assign_constants(
    std::size_t constant_count, const struct betree_constant** constants, struct ast_node* node)
{
    switch(node->type) {
        case AST_TYPE_IS_NULL_EXPR:
        case AST_TYPE_COMPARE_EXPR:
        case AST_TYPE_EQUALITY_EXPR:
        case AST_TYPE_SET_EXPR:
        case AST_TYPE_LIST_EXPR:
            return true;
        case AST_TYPE_BOOL_EXPR:
            switch(node->bool_expr.op) {
                case AST_BOOL_OR:
                case AST_BOOL_AND: {
                    bool lhs
                        = assign_constants(constant_count, constants, node->bool_expr.binary.lhs);
                    bool rhs
                        = assign_constants(constant_count, constants, node->bool_expr.binary.rhs);
                    return lhs && rhs;
                }
                case AST_BOOL_NOT:
                    return assign_constants(constant_count, constants, node->bool_expr.unary.expr);
                case AST_BOOL_VARIABLE:
                case AST_BOOL_LITERAL:
                default:
                    return true;
            }
        case AST_TYPE_SPECIAL_EXPR:
            switch(node->special_expr.type) {
                case AST_SPECIAL_FREQUENCY: {
                    const char* name
                        = get_constant_name_for_type(node->special_expr.frequency.type);
                    if(name == nullptr) {
                        return false;
                    }
                    std::int64_t constant = INT64_MAX;
                    bool success = get_integer_constant(name, constant_count, constants, &constant);
                    if(success == false) {
                        return false;
                    }
                    node->special_expr.frequency.id = (std::uint32_t)constant;
                    return true;
                }
                case AST_SPECIAL_SEGMENT:
                case AST_SPECIAL_GEO:
                case AST_SPECIAL_STRING:
                    return true;
                default:
                    std::fprintf(stderr, "Invalid special node type\n");
                    std::abort();
            }
        default:
            std::fprintf(stderr, "Invalid node type\n");
            std::abort();
    }
}

void assign_variable_id(struct config* config, struct ast_node* node)
{
    switch(node->type) {
        case AST_TYPE_IS_NULL_EXPR: {
            betree_var_t variable_id
                = try_get_id_for_attr(config, node->is_null_expr.attr_var.attr);
            node->is_null_expr.attr_var.var = variable_id;
            return;
        }
        case(AST_TYPE_SPECIAL_EXPR): {
            switch(node->special_expr.type) {
                case AST_SPECIAL_FREQUENCY: {
                    betree_var_t variable_id
                        = try_get_id_for_attr(config, node->special_expr.frequency.attr_var.attr);
                    node->special_expr.frequency.attr_var.var = variable_id;
                    betree_var_t now_id
                        = try_get_id_for_attr(config, node->special_expr.frequency.now.attr);
                    node->special_expr.frequency.now.var = now_id;
                    return;
                }
                case AST_SPECIAL_SEGMENT: {
                    betree_var_t variable_id
                        = try_get_id_for_attr(config, node->special_expr.segment.attr_var.attr);
                    node->special_expr.segment.attr_var.var = variable_id;
                    betree_var_t now_id
                        = try_get_id_for_attr(config, node->special_expr.segment.now.attr);
                    node->special_expr.segment.now.var = now_id;
                    return;
                }
                case AST_SPECIAL_GEO: {
                    betree_var_t latitude_id
                        = try_get_id_for_attr(config, node->special_expr.geo.latitude_var.attr);
                    node->special_expr.geo.latitude_var.var = latitude_id;
                    betree_var_t longitude_id
                        = try_get_id_for_attr(config, node->special_expr.geo.longitude_var.attr);
                    node->special_expr.geo.longitude_var.var = longitude_id;
                    return;
                }
                case AST_SPECIAL_STRING: {
                    betree_var_t variable_id
                        = try_get_id_for_attr(config, node->special_expr.string.attr_var.attr);
                    node->special_expr.string.attr_var.var = variable_id;
                    return;
                }
                default: std::abort();
            }
            return;
        }
        case(AST_TYPE_COMPARE_EXPR): {
            betree_var_t variable_id
                = try_get_id_for_attr(config, node->compare_expr.attr_var.attr);
            node->compare_expr.attr_var.var = variable_id;
            return;
        }
        case(AST_TYPE_EQUALITY_EXPR): {
            betree_var_t variable_id
                = try_get_id_for_attr(config, node->equality_expr.attr_var.attr);
            node->equality_expr.attr_var.var = variable_id;
            return;
        }
        case(AST_TYPE_BOOL_EXPR): {
            switch(node->bool_expr.op) {
                case AST_BOOL_LITERAL:
                    return;
                case AST_BOOL_NOT: {
                    assign_variable_id(config, node->bool_expr.unary.expr);
                    return;
                }
                case AST_BOOL_OR:
                case AST_BOOL_AND: {
                    assign_variable_id(config, node->bool_expr.binary.lhs);
                    assign_variable_id(config, node->bool_expr.binary.rhs);
                    return;
                }
                case AST_BOOL_VARIABLE: {
                    betree_var_t variable_id
                        = try_get_id_for_attr(config, node->bool_expr.variable.attr);
                    node->bool_expr.variable.var = variable_id;
                    return;
                }
                default: std::abort();
            }
        }
        case(AST_TYPE_LIST_EXPR): {
            betree_var_t variable_id = try_get_id_for_attr(config, node->list_expr.attr_var.attr);
            node->list_expr.attr_var.var = variable_id;
            return;
        }
        case(AST_TYPE_SET_EXPR): {
            switch(node->set_expr.left_value.value_type) {
                case AST_SET_LEFT_VALUE_INTEGER: {
                    break;
                }
                case AST_SET_LEFT_VALUE_STRING: {
                    break;
                }
                case AST_SET_LEFT_VALUE_VARIABLE: {
                    betree_var_t variable_id = try_get_id_for_attr(
                        config, node->set_expr.left_value.variable_value.attr);
                    node->set_expr.left_value.variable_value.var = variable_id;
                    break;
                }
                default: std::abort();
            }
            switch(node->set_expr.right_value.value_type) {
                case AST_SET_RIGHT_VALUE_INTEGER_LIST: {
                    break;
                }
                case AST_SET_RIGHT_VALUE_STRING_LIST: {
                    break;
                }
                case AST_SET_RIGHT_VALUE_VARIABLE: {
                    betree_var_t variable_id = try_get_id_for_attr(
                        config, node->set_expr.right_value.variable_value.attr);
                    node->set_expr.right_value.variable_value.var = variable_id;
                    break;
                }
                default: std::abort();
            }
            return;
        }
        default: std::abort();
    }
}

void assign_ienum_id(struct config* config, struct ast_node* node, bool always_assign)
{
    switch(node->type) {
        case AST_TYPE_IS_NULL_EXPR:
        case AST_TYPE_SPECIAL_EXPR: 
        case AST_TYPE_COMPARE_EXPR: 
        case AST_TYPE_LIST_EXPR:
        case AST_TYPE_SET_EXPR:
            return;
        case AST_TYPE_EQUALITY_EXPR:
            if(node->equality_expr.value.value_type == AST_EQUALITY_VALUE_INTEGER
              && config->attr_domains[node->equality_expr.attr_var.var]->bound.value_type == BETREE_INTEGER_ENUM) {
                betree_ienum_t ienum_id = get_id_for_ienum(config,
                    node->equality_expr.attr_var,
                    node->equality_expr.value.integer_value, always_assign);
                node->equality_expr.value.value_type = AST_EQUALITY_VALUE_INTEGER_ENUM;
                node->equality_expr.value.integer_enum_value.integer = node->equality_expr.value.integer_value;
                node->equality_expr.value.integer_enum_value.var = node->equality_expr.attr_var.var;
                node->equality_expr.value.integer_enum_value.ienum = ienum_id;
            }
            return;
        case AST_TYPE_BOOL_EXPR:
            switch(node->bool_expr.op) {
                case AST_BOOL_LITERAL:
                    return;
                case AST_BOOL_NOT: {
                    assign_ienum_id(config, node->bool_expr.unary.expr, always_assign);
                    return;
                }
                case AST_BOOL_OR:
                case AST_BOOL_AND: {
                    assign_ienum_id(config, node->bool_expr.binary.lhs, always_assign);
                    assign_ienum_id(config, node->bool_expr.binary.rhs, always_assign);
                    return;
                }
                case AST_BOOL_VARIABLE: {
                    return;
                }
                default: std::abort();
            }
        default: std::abort();
    }
}

void assign_str_id(struct config* config, struct ast_node* node, bool always_assign)
{
    switch(node->type) {
        case AST_TYPE_IS_NULL_EXPR:
            return;
        case(AST_TYPE_SPECIAL_EXPR): {
            switch(node->special_expr.type) {
                case AST_SPECIAL_FREQUENCY: {
                    betree_str_t str_id = get_id_for_string(config,
                        node->special_expr.frequency.attr_var,
                        node->special_expr.frequency.ns.string, always_assign);
                    node->special_expr.frequency.ns.var = node->special_expr.frequency.attr_var.var;
                    node->special_expr.frequency.ns.str = str_id;
                    return;
                }
                case AST_SPECIAL_SEGMENT: {
                    return;
                }
                case AST_SPECIAL_GEO: {
                    return;
                }
                case AST_SPECIAL_STRING: {
                    return;
                }
                default: std::abort();
            }
            return;
        }
        case(AST_TYPE_COMPARE_EXPR): {
            return;
        }
        case(AST_TYPE_EQUALITY_EXPR): {
            if(node->equality_expr.value.value_type == AST_EQUALITY_VALUE_STRING) {
                betree_str_t str_id = get_id_for_string(config,
                    node->equality_expr.attr_var,
                    node->equality_expr.value.string_value.string, always_assign);
                node->equality_expr.value.string_value.var = node->equality_expr.attr_var.var;
                node->equality_expr.value.string_value.str = str_id;
            }
            return;
        }
        case(AST_TYPE_BOOL_EXPR): {
            switch(node->bool_expr.op) {
                case AST_BOOL_LITERAL:
                    return;
                case AST_BOOL_NOT: {
                    assign_str_id(config, node->bool_expr.unary.expr, always_assign);
                    return;
                }
                case AST_BOOL_OR:
                case AST_BOOL_AND: {
                    assign_str_id(config, node->bool_expr.binary.lhs, always_assign);
                    assign_str_id(config, node->bool_expr.binary.rhs, always_assign);
                    return;
                }
                case AST_BOOL_VARIABLE: {
                    return;
                }
                default: std::abort();
            }
        }
        case(AST_TYPE_LIST_EXPR): {
            if(node->list_expr.value.value_type == AST_LIST_VALUE_STRING_LIST) {
                for(std::size_t i = 0; i < node->list_expr.value.string_list_value->count; i++) {
                    betree_str_t str_id = get_id_for_string(config,
                        node->list_expr.attr_var,
                        node->list_expr.value.string_list_value->strings[i].string, always_assign);
                    node->list_expr.value.string_list_value->strings[i].var
                        = node->list_expr.attr_var.var;
                    node->list_expr.value.string_list_value->strings[i].str = str_id;
                }
            }
            return;
        }
        case(AST_TYPE_SET_EXPR): {
            if(node->set_expr.left_value.value_type == AST_SET_LEFT_VALUE_STRING) {
                betree_str_t str_id = get_id_for_string(config,
                    node->set_expr.right_value.variable_value,
                    node->set_expr.left_value.string_value.string, always_assign);
                node->set_expr.left_value.string_value.var
                    = node->set_expr.right_value.variable_value.var;
                node->set_expr.left_value.string_value.str = str_id;
            }
            if(node->set_expr.right_value.value_type == AST_SET_RIGHT_VALUE_STRING_LIST) {
                for(std::size_t i = 0; i < node->set_expr.right_value.string_list_value->count; i++) {
                    betree_str_t str_id = get_id_for_string(config,
                        node->set_expr.left_value.variable_value,
                        node->set_expr.right_value.string_list_value->strings[i].string, always_assign);
                    node->set_expr.right_value.string_list_value->strings[i].var
                        = node->set_expr.left_value.variable_value.var;
                    node->set_expr.right_value.string_list_value->strings[i].str = str_id;
                }
            }
            return;
        }
        default: std::abort();
    }
}

static bool eq_compare_value(struct compare_value a, struct compare_value b)
{
    if(a.value_type != b.value_type) {
        return false;
    }
    switch(a.value_type) {
        case AST_COMPARE_VALUE_INTEGER:
            return a.integer_value == b.integer_value;
        case AST_COMPARE_VALUE_FLOAT:
            return feq(a.float_value, b.float_value);
        default: std::abort();
    }
}

static bool eq_equality_value(struct equality_value a, struct equality_value b)
{
    if(a.value_type != b.value_type) {
        return false;
    }
    switch(a.value_type) {
        case AST_EQUALITY_VALUE_INTEGER:
            return a.integer_value == b.integer_value;
        case AST_EQUALITY_VALUE_FLOAT:
            return feq(a.float_value, b.float_value);
        case AST_EQUALITY_VALUE_STRING:
            return a.string_value.var == b.string_value.var
                && a.string_value.str == b.string_value.str;
        case AST_EQUALITY_VALUE_INTEGER_ENUM:
            return a.integer_enum_value.var == b.integer_enum_value.var
                && a.integer_enum_value.ienum == b.integer_enum_value.ienum;
        default: std::abort();
    }
}

static bool eq_bool_expr(struct ast_bool_expr a, struct ast_bool_expr b)
{
    if(a.op != b.op) {
        return false;
    }
    switch(a.op) {
        case AST_BOOL_OR:
        case AST_BOOL_AND:
            return eq_expr(a.binary.lhs, b.binary.lhs) && eq_expr(a.binary.rhs, b.binary.rhs);
        case AST_BOOL_NOT:
            return eq_expr(a.unary.expr, b.unary.expr);
        case AST_BOOL_VARIABLE:
            return a.variable.var == b.variable.var;
        case AST_BOOL_LITERAL:
            return a.literal == b.literal;
        default: std::abort();
    }
}

static bool eq_set_left_value(struct set_left_value a, struct set_left_value b)
{
    if(a.value_type != b.value_type) {
        return false;
    }
    switch(a.value_type) {
        case AST_SET_LEFT_VALUE_INTEGER:
            return a.integer_value == b.integer_value;
        case AST_SET_LEFT_VALUE_STRING:
            return a.string_value.var == b.string_value.var
                && a.string_value.str == b.string_value.str;
        case AST_SET_LEFT_VALUE_VARIABLE:
            return a.variable_value.var == b.variable_value.var;
        default: std::abort();
    }
}

static bool eq_integer_list(struct betree_integer_list* a, struct betree_integer_list* b)
{
    if(a->count != b->count) {
        return false;
    }
    for(std::size_t i = 0; i < a->count; i++) {
        if(a->integers[i] != b->integers[i]) {
            return false;
        }
    }
    return true;
}

static bool eq_string_list(struct betree_string_list* a, struct betree_string_list* b)
{
    if(a->count != b->count) {
        return false;
    }
    for(std::size_t i = 0; i < a->count; i++) {
        if(a->strings[i].var == b->strings[i].var && a->strings[i].str != b->strings[i].str) {
            return false;
        }
    }
    return true;
}

static bool eq_set_right_value(struct set_right_value a, struct set_right_value b)
{
    if(a.value_type != b.value_type) {
        return false;
    }
    switch(a.value_type) {
        case AST_SET_RIGHT_VALUE_INTEGER_LIST:
            return eq_integer_list(a.integer_list_value, b.integer_list_value);
        case AST_SET_RIGHT_VALUE_STRING_LIST:
            return eq_string_list(a.string_list_value, b.string_list_value);
        case AST_SET_RIGHT_VALUE_VARIABLE:
            return a.variable_value.var == b.variable_value.var;
        default: std::abort();
    }
}

static bool eq_set_expr(struct ast_set_expr a, struct ast_set_expr b)
{
    if(a.op != b.op) {
        return false;
    }
    return eq_set_left_value(a.left_value, b.left_value)
        && eq_set_right_value(a.right_value, b.right_value);
}

static bool eq_list_value(struct list_value a, struct list_value b)
{
    if(a.value_type != b.value_type) {
        return false;
    }
    switch(a.value_type) {
        case AST_LIST_VALUE_INTEGER_LIST:
            return eq_integer_list(a.integer_list_value, b.integer_list_value);
        case AST_LIST_VALUE_STRING_LIST:
            return eq_string_list(a.string_list_value, b.string_list_value);
        default: std::abort();
    }
}

static bool eq_list_expr(struct ast_list_expr a, struct ast_list_expr b)
{
    if(a.op != b.op) {
        return false;
    }
    return a.attr_var.var == b.attr_var.var && eq_list_value(a.value, b.value);
}

static bool eq_special_expr(struct ast_special_expr a, struct ast_special_expr b)
{
    if(a.type != b.type) {
        return false;
    }
    switch(a.type) {
        case AST_SPECIAL_FREQUENCY:
            return a.frequency.attr_var.var == b.frequency.attr_var.var
                && a.frequency.length == b.frequency.length
                && a.frequency.ns.var == b.frequency.ns.var
                && a.frequency.ns.str == b.frequency.ns.str && a.frequency.op == b.frequency.op
                && a.frequency.type == b.frequency.type && a.frequency.value == b.frequency.value;
        case AST_SPECIAL_SEGMENT:
            return a.segment.attr_var.var == b.segment.attr_var.var
                && a.segment.has_variable == b.segment.has_variable && a.segment.op == b.segment.op
                && a.segment.seconds == b.segment.seconds
                && a.segment.segment_id == b.segment.segment_id;
        case AST_SPECIAL_GEO:
            return a.geo.has_radius == b.geo.has_radius && feq(a.geo.latitude, b.geo.latitude)
                && feq(a.geo.longitude, b.geo.longitude) && a.geo.op == b.geo.op
                && feq(a.geo.radius, b.geo.radius);
        case AST_SPECIAL_STRING:
            return a.string.attr_var.var == b.string.attr_var.var && a.string.op == b.string.op
                && strcmp(a.string.pattern, b.string.pattern) == 0;
        default: std::abort();
    }
}

bool eq_expr(const struct ast_node* a, const struct ast_node* b)
{
    if(a == nullptr || b == nullptr) {
        return false;
    }
    if(a->type != b->type) {
        return false;
    }
    switch(a->type) {
        case AST_TYPE_IS_NULL_EXPR:
            return a->is_null_expr.attr_var.var == b->is_null_expr.attr_var.var;
        case AST_TYPE_COMPARE_EXPR: {
            return a->compare_expr.attr_var.var == b->compare_expr.attr_var.var
                && a->compare_expr.op == b->compare_expr.op
                && eq_compare_value(a->compare_expr.value, b->compare_expr.value);
        }
        case AST_TYPE_EQUALITY_EXPR: {
            return a->equality_expr.attr_var.var == b->equality_expr.attr_var.var
                && a->equality_expr.op == b->equality_expr.op
                && eq_equality_value(a->equality_expr.value, b->equality_expr.value);
        }
        case AST_TYPE_BOOL_EXPR: {
            return eq_bool_expr(a->bool_expr, b->bool_expr);
        }
        case AST_TYPE_SET_EXPR: {
            return eq_set_expr(a->set_expr, b->set_expr);
        }
        case AST_TYPE_LIST_EXPR: {
            return eq_list_expr(a->list_expr, b->list_expr);
        }
        case AST_TYPE_SPECIAL_EXPR: {
            return eq_special_expr(a->special_expr, b->special_expr);
        }
        default: std::abort();
    }
}

bool fast_eq_expr(const struct ast_node* a, const struct ast_node* b)
{
    if(a == nullptr || b == nullptr) {
        return false;
    }
    if(a->type != b->type) {
        return false;
    }
    if(a->type == AST_TYPE_BOOL_EXPR) {
        if(a->bool_expr.op == AST_BOOL_NOT) {
            return a->bool_expr.unary.expr->global_id == b->bool_expr.unary.expr->global_id;
        }
        if(a->bool_expr.op == AST_BOOL_AND || a->bool_expr.op == AST_BOOL_OR) {
            return a->bool_expr.binary.lhs->global_id == b->bool_expr.binary.lhs->global_id
                && a->bool_expr.binary.rhs->global_id == b->bool_expr.binary.rhs->global_id;
        }
    }
    return a->global_id == b->global_id;
}

void assign_pred_id(struct config* config, struct ast_node* node)
{
    assign_pred(config->pred_map, node);
}


void sort_lists(struct ast_node* node)
{
    switch(node->type) {
        case AST_TYPE_IS_NULL_EXPR:
        case AST_TYPE_COMPARE_EXPR:
        case AST_TYPE_EQUALITY_EXPR:
            return;
        case AST_TYPE_BOOL_EXPR:
            switch(node->bool_expr.op) {
                case AST_BOOL_OR:
                case AST_BOOL_AND:
                    sort_lists(node->bool_expr.binary.lhs);
                    sort_lists(node->bool_expr.binary.rhs);
                    return;
                case AST_BOOL_NOT:
                    return sort_lists(node->bool_expr.unary.expr);
                case AST_BOOL_VARIABLE:
                case AST_BOOL_LITERAL:
                    return;
                default: std::abort();
            }
        case AST_TYPE_SET_EXPR:
            if(node->set_expr.left_value.value_type == AST_SET_LEFT_VALUE_VARIABLE) {
                switch(node->set_expr.right_value.value_type) {
                    case AST_SET_RIGHT_VALUE_INTEGER_LIST:
                        sort_integer_list(node->set_expr.right_value.integer_list_value);
                        return;
                    case AST_SET_RIGHT_VALUE_STRING_LIST:
                        sort_string_list(node->set_expr.right_value.string_list_value);
                        return;
                    case AST_SET_RIGHT_VALUE_VARIABLE:
                        return;
                    default:
                        std::fprintf(stderr, "Invalid set expr");
                        std::abort();
                        return;
                }
            }
            return;
        case AST_TYPE_LIST_EXPR:
            switch(node->list_expr.value.value_type) {
                case AST_LIST_VALUE_INTEGER_LIST:
                    sort_integer_list(node->list_expr.value.integer_list_value);
                    return;
                case AST_LIST_VALUE_STRING_LIST:
                    sort_string_list(node->list_expr.value.string_list_value);
                    return;
                default: std::abort();
            }
        case AST_TYPE_SPECIAL_EXPR:
            switch(node->special_expr.type) {
                case AST_SPECIAL_FREQUENCY:
                case AST_SPECIAL_SEGMENT:
                case AST_SPECIAL_GEO:
                case AST_SPECIAL_STRING:
                    return;
                default: std::abort();
            }
        default: std::abort();
    }
}

bool var_exists(const struct config* config, const char* attr)
{
    for(std::size_t i = 0; i < config->attr_domain_count; i++) {
        if(strcmp(attr, config->attr_domains[i]->attr_var.attr) == 0) {
            return true;
        }
    }
    return false;
}

bool is_variable_valid(struct attr_var attr_var)
{
    if(attr_var.var == INVALID_VAR) {
        std::fprintf(stderr, "Invalid variable: %s\n", attr_var.attr);
        return false;
    }
    return true;
}

bool is_list_type(enum betree_value_type_e type) {
    return type == BETREE_INTEGER_LIST || type == BETREE_STRING_LIST;
}

bool all_exprs_valid(const struct config* config, const struct ast_node* node)
{
    switch(node->type) {
        case AST_TYPE_IS_NULL_EXPR: {
            const struct attr_domain* attr_domain = config->attr_domains[node->is_null_expr.attr_var.var];
            enum betree_value_type_e var_type = attr_domain->bound.value_type;
            return (node->is_null_expr.type == AST_IS_NULL && attr_domain->allow_undefined)
                || node->is_null_expr.type == AST_IS_NOT_NULL
                || (node->is_null_expr.type == AST_IS_EMPTY && is_list_type(var_type));
        }
        case AST_TYPE_COMPARE_EXPR: {
            enum betree_value_type_e var_type = config->attr_domains[node->compare_expr.attr_var.var]->bound.value_type;
            enum ast_compare_value_e val_type = node->compare_expr.value.value_type;
            return (var_type == BETREE_INTEGER && val_type == AST_COMPARE_VALUE_INTEGER)
                || (var_type == BETREE_FLOAT && val_type == AST_COMPARE_VALUE_FLOAT);
        }
        case AST_TYPE_EQUALITY_EXPR: {
            enum betree_value_type_e var_type = config->attr_domains[node->equality_expr.attr_var.var]->bound.value_type;
            enum ast_equality_value_e val_type = node->equality_expr.value.value_type;
            return (var_type == BETREE_STRING && val_type == AST_EQUALITY_VALUE_STRING)
                || (var_type == BETREE_FLOAT && val_type == AST_EQUALITY_VALUE_FLOAT)
                || (var_type == BETREE_INTEGER && val_type == AST_EQUALITY_VALUE_INTEGER)
                || (var_type == BETREE_INTEGER_ENUM && val_type == AST_EQUALITY_VALUE_INTEGER_ENUM);
        }
        case AST_TYPE_LIST_EXPR: {
            enum betree_value_type_e var_type = config->attr_domains[node->list_expr.attr_var.var]->bound.value_type;
            enum ast_list_value_e val_type = node->list_expr.value.value_type;
            return (var_type == BETREE_STRING_LIST && val_type == AST_LIST_VALUE_STRING_LIST)
                || (var_type == BETREE_INTEGER_LIST && val_type == AST_LIST_VALUE_INTEGER_LIST);
        }
        case AST_TYPE_BOOL_EXPR: {
            switch(node->bool_expr.op) {
                case AST_BOOL_VARIABLE: {
                    enum betree_value_type_e var_type = config->attr_domains[node->bool_expr.variable.var]->bound.value_type;
                    return var_type == BETREE_BOOLEAN;
                }
                case AST_BOOL_LITERAL: 
                    return true;
                case AST_BOOL_OR:
                case AST_BOOL_AND:
                    return all_exprs_valid(config, node->bool_expr.binary.lhs)
                        && all_exprs_valid(config, node->bool_expr.binary.rhs);
                case AST_BOOL_NOT:
                    return all_exprs_valid(config, node->bool_expr.unary.expr);
                default: std::abort();
            }
        }
        case AST_TYPE_SET_EXPR: {
            if(node->set_expr.left_value.value_type == AST_SET_LEFT_VALUE_VARIABLE) {
                enum betree_value_type_e var_type = config->attr_domains[node->set_expr.left_value.variable_value.var]->bound.value_type;
                enum set_right_value_e val_type = node->set_expr.right_value.value_type;
                return (var_type == BETREE_STRING && val_type == AST_SET_RIGHT_VALUE_STRING_LIST)
                    || (var_type == BETREE_INTEGER && val_type == AST_SET_RIGHT_VALUE_INTEGER_LIST);
            }
            else if(node->set_expr.right_value.value_type == AST_SET_RIGHT_VALUE_VARIABLE) {
                enum betree_value_type_e var_type = config->attr_domains[node->set_expr.right_value.variable_value.var]->bound.value_type;
                enum set_left_value_e val_type = node->set_expr.left_value.value_type;
                return (var_type == BETREE_STRING_LIST && val_type == AST_SET_LEFT_VALUE_STRING)
                    || (var_type == BETREE_INTEGER_LIST && val_type == AST_SET_LEFT_VALUE_INTEGER); 
            }
            return false;
        }
        case AST_TYPE_SPECIAL_EXPR:
            switch(node->special_expr.type) {
                case AST_SPECIAL_FREQUENCY:
                    return node->special_expr.frequency.type != FREQUENCY_TYPE_INVALID;
                case AST_SPECIAL_SEGMENT:
                case AST_SPECIAL_GEO:
                case AST_SPECIAL_STRING:
                    return true;
                default: std::abort();
            }
        default: std::abort();
    }
}

bool all_variables_in_config(const struct config* config, const struct ast_node* node)
{
    switch(node->type) {
        case AST_TYPE_IS_NULL_EXPR: return is_variable_valid(node->is_null_expr.attr_var);
        case AST_TYPE_COMPARE_EXPR: return is_variable_valid(node->compare_expr.attr_var);
        case AST_TYPE_EQUALITY_EXPR: return is_variable_valid(node->equality_expr.attr_var);
        case AST_TYPE_LIST_EXPR: return is_variable_valid(node->list_expr.attr_var);
        case AST_TYPE_BOOL_EXPR:
            switch(node->bool_expr.op) {
                case AST_BOOL_VARIABLE: return is_variable_valid(node->bool_expr.variable);
                case AST_BOOL_LITERAL: return true;
                case AST_BOOL_OR:
                case AST_BOOL_AND:
                    return all_variables_in_config(config, node->bool_expr.binary.lhs)
                        && all_variables_in_config(config, node->bool_expr.binary.rhs);
                case AST_BOOL_NOT:
                    return all_variables_in_config(config, node->bool_expr.unary.expr);
                default: std::abort();
            }
        case AST_TYPE_SET_EXPR:
            if(node->set_expr.left_value.value_type == AST_SET_LEFT_VALUE_VARIABLE) {
                return is_variable_valid(node->set_expr.left_value.variable_value);
            }
            else if(node->set_expr.right_value.value_type == AST_SET_RIGHT_VALUE_VARIABLE) {
                return is_variable_valid(node->set_expr.right_value.variable_value);
            }
            std::fprintf(stderr, "Invalid set expr");
            std::abort();
            return false;
        case AST_TYPE_SPECIAL_EXPR:
            switch(node->special_expr.type) {
                case AST_SPECIAL_GEO: return true;
                case AST_SPECIAL_STRING: return is_variable_valid(node->special_expr.string.attr_var);
                case AST_SPECIAL_FREQUENCY:
                    return is_variable_valid(node->special_expr.frequency.attr_var)
                        && is_variable_valid(node->special_expr.frequency.now);
                case AST_SPECIAL_SEGMENT:
                    return is_variable_valid(node->special_expr.segment.attr_var)
                        && is_variable_valid(node->special_expr.segment.now);
                default: std::abort();
            }
        default: std::abort();
    }
}

static std::size_t get_attr_string_bound(const struct config* config, std::string_view attr)
{
    for(std::size_t i = 0; i < config->attr_domain_count; i++) {
        if(attr == config->attr_domains[i]->attr_var.attr) {
            std::size_t count = config->attr_domains[i]->bound.smax;
            if(count == SIZE_MAX) {
                return count;
            }
            return count + 1;
        }
    }
    return SIZE_MAX;
}

static struct string_map* get_string_map_for_attr(const struct config* config, std::string_view attr)
{
    for(std::size_t i = 0; i < config->string_map_count; i++) {
        struct string_map* string_map = &config->string_maps[i];
        if(attr == string_map->attr_var.attr) {
            return string_map;
        }
    }
    return nullptr;
}

static bool str_valid(const struct config* config, std::string_view attr, std::string_view string)
{
    std::size_t bound = get_attr_string_bound(config, attr);
    struct string_map* string_map = get_string_map_for_attr(config, attr);
    std::size_t space_left = string_map == nullptr ? bound : bound - string_map->string_value_count;
    if(string_map != nullptr) {
        auto str = static_cast<betree_str_t*>(map_get_(&string_map->m.base, string.data()));
        if(str != nullptr) {
            return true;
        }
    }
    bool within_bound = space_left > 0;
    if(within_bound == false) {
        std::fprintf(stderr,
            "For attr '%s', string '%s' could not fit within the bound %zu\n",
            attr.data(),
            string.data(),
            bound);
    }
    return within_bound;
}

static bool strs_valid(
    const struct config* config, const char* attr, const struct betree_string_list* strings)
{
    std::size_t bound = get_attr_string_bound(config, attr);
    if(bound == SIZE_MAX) {
        return true;
    }
    struct string_map* string_map = get_string_map_for_attr(config, attr);
    std::size_t space_left = string_map == nullptr ? bound : bound - string_map->string_value_count;
    std::size_t found = 0;
    if(string_map != nullptr) {
        for(std::size_t i = 0; i < strings->count; i++) {
            const char* string = strings->strings[i].string;
            auto str = static_cast<betree_str_t*>(map_get_(&string_map->m.base, string));
            if(str != nullptr) {
                found++;
            }
        }
    }
    std::size_t needed = strings->count - found;
    bool within_bound = space_left >= needed;
    if(within_bound == false) {
        std::fprintf(stderr,
            "For attr '%s', not all strings could not fit within the bound %zu\n",
            attr,
            bound);
    }
    return within_bound;
}

bool all_bounded_strings_valid(const struct config* config, const struct ast_node* node)
{
    switch(node->type) {
        case AST_TYPE_IS_NULL_EXPR:
        case AST_TYPE_COMPARE_EXPR:
            return true;
        case AST_TYPE_EQUALITY_EXPR:
            if(node->equality_expr.value.value_type == AST_EQUALITY_VALUE_STRING) {
                return str_valid(config,
                    node->equality_expr.attr_var.attr,
                    node->equality_expr.value.string_value.string);
            }
            return true;
        case AST_TYPE_BOOL_EXPR:
            switch(node->bool_expr.op) {
                case AST_BOOL_OR:
                case AST_BOOL_AND: {
                    bool lhs = all_bounded_strings_valid(config, node->bool_expr.binary.lhs);
                    if(lhs == false) {
                        return false;
                    }
                    return all_bounded_strings_valid(config, node->bool_expr.binary.rhs);
                }
                case AST_BOOL_NOT:
                    return all_bounded_strings_valid(config, node->bool_expr.unary.expr);
                case AST_BOOL_VARIABLE:
                case AST_BOOL_LITERAL:
                    return true;
                default: std::abort();
            }
        case AST_TYPE_SET_EXPR:
            if(node->set_expr.left_value.value_type == AST_SET_LEFT_VALUE_VARIABLE
                && node->set_expr.right_value.value_type == AST_SET_RIGHT_VALUE_STRING_LIST) {
                return strs_valid(config,
                    node->set_expr.left_value.variable_value.attr,
                    node->set_expr.right_value.string_list_value);
            }
            if(node->set_expr.right_value.value_type == AST_SET_RIGHT_VALUE_VARIABLE
                && node->set_expr.left_value.value_type == AST_SET_LEFT_VALUE_STRING) {
                return str_valid(config,
                    node->set_expr.right_value.variable_value.attr,
                    node->set_expr.left_value.string_value.string);
            }
            return true;
        case AST_TYPE_LIST_EXPR:
            if(node->list_expr.value.value_type == AST_LIST_VALUE_STRING_LIST) {
                return strs_valid(
                    config, node->list_expr.attr_var.attr, node->list_expr.value.string_list_value);
            }
            return true;
        case AST_TYPE_SPECIAL_EXPR:
            return true;
        default: std::abort();
    }
}

} // extern "C"
