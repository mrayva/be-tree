#include <cstdint>
#include <cstdio>
#include <cstring>
#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>

#include "alloc.h"
#include "ast.h"
#include "ast_eval_shared.hpp"
#include "ast_match_shared.hpp"
#include "betree.h"
#include "betree_err.h"
#include "error.h"
#include "hashmap.h"
#include "memoize.h"
#include "printer.h"
#include "special.h"
#include "tree.h"
#include "utils.h"
#include "value.h"
#include "var.h"

static bool match_node_inner_err(const struct betree_variable** preds,
    const struct ast_node* node,
    struct memoize* memoize,
    struct report_err* report,
    betree_var_t* last_reason,
    betree_var_t* memoize_reason,
    std::size_t attr_domain_count);

static bool match_special_expr_err(const struct betree_variable** preds,
    const struct ast_special_expr special_expr,
    betree_var_t* last_reason,
    std::size_t attr_domain_count)
{
    switch(special_expr.type) {
        case AST_SPECIAL_FREQUENCY: {
            switch(special_expr.frequency.op) {
                case AST_SPECIAL_WITHINFREQUENCYCAP: {
                    auto f = &special_expr.frequency;
                    struct betree_frequency_caps* caps;
                    auto is_caps_defined = get_frequency_var(f->attr_var.var, preds, &caps);
                    *last_reason = special_expr.frequency.attr_var.var;

                    if(is_caps_defined == false) {
                        return false;
                    }
                    if(caps->size == 0) {
                        // Optimization from looking at what within_frequency_caps does
                        return true;
                    }
                    std::int64_t now;
                    auto is_now_defined = get_integer_var(f->now.var, preds, &now);
                    if(is_now_defined == false) {
                        return false;
                    }
                    return within_frequency_caps(
                        caps, f->type, f->id, f->ns, f->value, f->length, now);
                }
                default:
                    std::abort();
            }
        }
        case AST_SPECIAL_SEGMENT: {
            *last_reason = special_expr.segment.attr_var.var;
            auto s = &special_expr.segment;
            struct betree_segments* segments;
            auto is_segment_defined = get_segments_var(s->attr_var.var, preds, &segments);
            if(is_segment_defined == false) {
                return false;
            }
            std::int64_t now;
            auto is_now_defined = get_integer_var(s->now.var, preds, &now);
            if(is_now_defined == false) {
                return false;
            }
            switch(special_expr.segment.op) {
                case AST_SPECIAL_SEGMENTWITHIN:
                    return segment_within(s->segment_id, s->seconds, segments, now);
                case AST_SPECIAL_SEGMENTBEFORE:
                    return segment_before(s->segment_id, s->seconds, segments, now);
                default:
                    std::abort();
            }
        }
        case AST_SPECIAL_GEO: {
            switch(special_expr.geo.op) {
                case AST_SPECIAL_GEOWITHINRADIUS: {
                    *last_reason = ADDITIONAL_REASON(attr_domain_count, REASON_GEO);
                    auto g = &special_expr.geo;
                    double latitude_var, longitude_var;
                    auto is_latitude_defined
                        = get_float_var(g->latitude_var.var, preds, &latitude_var);
                    auto is_longitude_defined
                        = get_float_var(g->longitude_var.var, preds, &longitude_var);
                    if(is_latitude_defined == false || is_longitude_defined == false) {
                        return false;
                    }
                    return geo_within_radius(
                        g->latitude, g->longitude, latitude_var, longitude_var, g->radius);
                }
                default:
                    std::abort();
            }
            return false;
        }
        case AST_SPECIAL_STRING: {
            *last_reason = special_expr.string.attr_var.var;
            auto s = &special_expr.string;
            struct string_value value;
            auto is_string_defined = get_string_var(s->attr_var.var, preds, &value);
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
                default:
                    std::abort();
            }
            return false;
        }
        default:
            std::abort();
    }
}

static bool match_list_expr_err(const struct betree_variable** preds,
    const struct ast_list_expr list_expr,
    betree_var_t* last_reason)
{
    return ast_eval_shared::match_list_expr(
        preds, list_expr, [last_reason](betree_var_t var) { *last_reason = var; });
}

static bool match_set_expr_err(const struct betree_variable** preds,
    const struct ast_set_expr set_expr,
    betree_var_t* last_reason)
{
    return ast_eval_shared::match_set_expr(
        preds, set_expr, [last_reason](betree_var_t var) { *last_reason = var; });
}

static bool match_compare_expr_err(const struct betree_variable** preds,
    const struct ast_compare_expr compare_expr,
    betree_var_t* last_reason)
{
    return ast_eval_shared::match_compare_expr(
        preds, compare_expr, [last_reason](betree_var_t var) { *last_reason = var; });
}

static bool match_equality_expr_err(const struct betree_variable** preds,
    const struct ast_equality_expr equality_expr,
    betree_var_t* last_reason)
{
    return ast_eval_shared::match_equality_expr(
        preds, equality_expr, [last_reason](betree_var_t var) { *last_reason = var; });
}

static bool match_is_null_expr_err(const struct betree_variable** preds,
    const struct ast_is_null_expr is_null_expr,
    betree_var_t* last_reason)
{
    return ast_eval_shared::match_is_null_expr(
        preds, is_null_expr, [last_reason](betree_var_t var) { *last_reason = var; });
}

static bool match_bool_expr_err(const struct betree_variable** preds,
    const struct ast_bool_expr bool_expr,
    struct memoize* memoize,
    struct report_err* report,
    betree_var_t* last_reason,
    betree_var_t* memoize_reason,
    std::size_t attr_domain_count)
{
    switch(bool_expr.op) {
        case AST_BOOL_LITERAL:
            return bool_expr.literal;
        case AST_BOOL_AND: {
            auto lhs = match_node_inner_err(preds,
                bool_expr.binary.lhs,
                memoize,
                report,
                last_reason,
                memoize_reason,
                attr_domain_count);
            if(lhs == false) {
                return false;
            }
            auto rhs = match_node_inner_err(preds,
                bool_expr.binary.rhs,
                memoize,
                report,
                last_reason,
                memoize_reason,
                attr_domain_count);
            return rhs;
        }
        case AST_BOOL_OR: {
            auto lhs = match_node_inner_err(preds,
                bool_expr.binary.lhs,
                memoize,
                report,
                last_reason,
                memoize_reason,
                attr_domain_count);
            if(lhs == true) {
                return true;
            }
            auto rhs = match_node_inner_err(preds,
                bool_expr.binary.rhs,
                memoize,
                report,
                last_reason,
                memoize_reason,
                attr_domain_count);
            return rhs;
        }
        case AST_BOOL_NOT: {
            auto result = match_node_inner_err(preds,
                bool_expr.unary.expr,
                memoize,
                report,
                last_reason,
                memoize_reason,
                attr_domain_count);
            return !result;
        }
        case AST_BOOL_VARIABLE: {
            bool value;
            auto is_variable_defined = get_bool_var(bool_expr.variable.var, preds, &value);
            *last_reason = bool_expr.variable.var;

            if(is_variable_defined == false) {
                return false;
            }
            return value;
        }
        default:
            std::abort();
    }
}

static bool match_node_inner_err(const struct betree_variable** preds,
    const struct ast_node* node,
    struct memoize* memoize,
    struct report_err* report,
    betree_var_t* last_reason,
    betree_var_t* memoize_reason,
    std::size_t attr_domain_count)
{
    if(node->memoize_id != INVALID_PRED) {
        if(test_bit(memoize->pass, node->memoize_id)) {
            if(report != nullptr) {
                report->memoized++;
            }
            *last_reason = memoize_reason[node->memoize_id];
            return true;
        }
        if(test_bit(memoize->fail, node->memoize_id)) {
            if(report != nullptr) {
                report->memoized++;
            }
            *last_reason = memoize_reason[node->memoize_id];
            return false;
        }
    }
    bool result;
    switch(node->type) {
        case AST_TYPE_IS_NULL_EXPR:
            result = match_is_null_expr_err(preds, node->is_null_expr, last_reason);
            break;
        case AST_TYPE_SPECIAL_EXPR: {
            result
                = match_special_expr_err(preds, node->special_expr, last_reason, attr_domain_count);
            break;
        }
        case AST_TYPE_BOOL_EXPR: {
            result = match_bool_expr_err(preds,
                node->bool_expr,
                memoize,
                report,
                last_reason,
                memoize_reason,
                attr_domain_count);
            break;
        }
        case AST_TYPE_LIST_EXPR: {
            result = match_list_expr_err(preds, node->list_expr, last_reason);
            break;
        }
        case AST_TYPE_SET_EXPR: {
            result = match_set_expr_err(preds, node->set_expr, last_reason);
            break;
        }
        case AST_TYPE_COMPARE_EXPR: {
            result = match_compare_expr_err(preds, node->compare_expr, last_reason);
            break;
        }
        case AST_TYPE_EQUALITY_EXPR: {
            result = match_equality_expr_err(preds, node->equality_expr, last_reason);
            break;
        }
        default:
            std::abort();
    }
    if(node->memoize_id != INVALID_PRED) {
        if(result) {
            set_bit(memoize->pass, node->memoize_id);
            memoize_reason[node->memoize_id] = *last_reason;
        }
        else {
            set_bit(memoize->fail, node->memoize_id);
            memoize_reason[node->memoize_id] = *last_reason;
        }
    }
    return result;
}

// --- Tri-state evaluation for the flat/continuation search path ---
//
// Mirrors match_node_tri_inner in ast.cpp, but threads the last_reason/
// memoize_reason out-parameters match_node_err uses instead of
// report->last_var (report_err has no such field -- reason tracking here
// is the whole point, not an optional callback). Reuses the existing
// match_*_expr_err helpers above for the actual matching logic, gated by
// the same check_pred pattern.

static enum match_result match_node_tri_inner_err(const struct betree_variable** preds,
    const struct ast_node* node,
    struct memoize* memoize,
    struct report_err* report,
    betree_var_t* last_reason,
    betree_var_t* memoize_reason,
    std::size_t attr_domain_count);

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

static enum match_result match_bool_expr_tri_err(const struct betree_variable** preds,
    const struct ast_bool_expr bool_expr,
    struct memoize* memoize,
    struct report_err* report,
    betree_var_t* last_reason,
    betree_var_t* memoize_reason,
    std::size_t attr_domain_count)
{
    switch(bool_expr.op) {
        case AST_BOOL_LITERAL:
            return to_result(bool_expr.literal);
        case AST_BOOL_AND: {
            auto lhs = match_node_tri_inner_err(preds, bool_expr.binary.lhs, memoize, report,
                last_reason, memoize_reason, attr_domain_count);
            if(lhs == MATCH_FALSE) {
                return MATCH_FALSE;
            }
            auto rhs = match_node_tri_inner_err(preds, bool_expr.binary.rhs, memoize, report,
                last_reason, memoize_reason, attr_domain_count);
            if(lhs == MATCH_TRUE) {
                return rhs;
            }
            return rhs == MATCH_FALSE ? MATCH_FALSE : MATCH_UNKNOWN;
        }
        case AST_BOOL_OR: {
            auto lhs = match_node_tri_inner_err(preds, bool_expr.binary.lhs, memoize, report,
                last_reason, memoize_reason, attr_domain_count);
            if(lhs == MATCH_TRUE) {
                return MATCH_TRUE;
            }
            auto rhs = match_node_tri_inner_err(preds, bool_expr.binary.rhs, memoize, report,
                last_reason, memoize_reason, attr_domain_count);
            if(lhs == MATCH_FALSE) {
                return rhs;
            }
            return rhs == MATCH_TRUE ? MATCH_TRUE : MATCH_UNKNOWN;
        }
        case AST_BOOL_NOT: {
            auto result = match_node_tri_inner_err(preds, bool_expr.unary.expr, memoize, report,
                last_reason, memoize_reason, attr_domain_count);
            if(result == MATCH_UNKNOWN) {
                return MATCH_UNKNOWN;
            }
            return result == MATCH_TRUE ? MATCH_FALSE : MATCH_TRUE;
        }
        case AST_BOOL_VARIABLE: {
            betree_var_t var = bool_expr.variable.var;
            *last_reason = var;
            enum match_result cp = check_pred(preds, var);
            if(cp != MATCH_TRUE) {
                return cp;
            }
            bool value;
            get_bool_var(var, preds, &value);
            return to_result(value);
        }
        default:
            std::abort();
    }
}

static enum match_result match_compare_expr_tri_err(
    const struct betree_variable** preds,
    const struct ast_compare_expr compare_expr,
    betree_var_t* last_reason)
{
    enum match_result cp = check_pred(preds, compare_expr.attr_var.var);
    if(cp != MATCH_TRUE) {
        *last_reason = compare_expr.attr_var.var;
        return cp;
    }
    return to_result(match_compare_expr_err(preds, compare_expr, last_reason));
}

static enum match_result match_equality_expr_tri_err(
    const struct betree_variable** preds,
    const struct ast_equality_expr equality_expr,
    betree_var_t* last_reason)
{
    enum match_result cp = check_pred(preds, equality_expr.attr_var.var);
    if(cp != MATCH_TRUE) {
        *last_reason = equality_expr.attr_var.var;
        return cp;
    }
    return to_result(match_equality_expr_err(preds, equality_expr, last_reason));
}

static enum match_result match_list_expr_tri_err(
    const struct betree_variable** preds,
    const struct ast_list_expr list_expr,
    betree_var_t* last_reason)
{
    enum match_result cp = check_pred(preds, list_expr.attr_var.var);
    if(cp != MATCH_TRUE) {
        *last_reason = list_expr.attr_var.var;
        return cp;
    }
    return to_result(match_list_expr_err(preds, list_expr, last_reason));
}

static enum match_result match_set_expr_tri_err(
    const struct betree_variable** preds,
    const struct ast_set_expr set_expr,
    betree_var_t* last_reason)
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
        *last_reason = var;
        return cp;
    }
    return to_result(match_set_expr_err(preds, set_expr, last_reason));
}

static enum match_result match_special_expr_tri_err(
    const struct betree_variable** preds,
    const struct ast_special_expr special_expr,
    betree_var_t* last_reason,
    std::size_t attr_domain_count)
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
        default:
            std::abort();
    }
    return to_result(match_special_expr_err(preds, special_expr, last_reason, attr_domain_count));
}

static enum match_result match_is_null_expr_tri_err(
    const struct betree_variable** preds,
    const struct ast_is_null_expr is_null_expr,
    betree_var_t* last_reason)
{
    if(preds[is_null_expr.attr_var.var] == &BETREE_PRED_UNFETCHED) {
        return MATCH_UNKNOWN;
    }
    return to_result(match_is_null_expr_err(preds, is_null_expr, last_reason));
}

static enum match_result match_node_tri_inner_err(const struct betree_variable** preds,
    const struct ast_node* node,
    struct memoize* memoize,
    struct report_err* report,
    betree_var_t* last_reason,
    betree_var_t* memoize_reason,
    std::size_t attr_domain_count)
{
    if(node->memoize_id != INVALID_PRED) {
        if(test_bit(memoize->pass, node->memoize_id)) {
            if(report != nullptr) {
                report->memoized++;
            }
            *last_reason = memoize_reason[node->memoize_id];
            return MATCH_TRUE;
        }
        if(test_bit(memoize->fail, node->memoize_id)) {
            if(report != nullptr) {
                report->memoized++;
            }
            *last_reason = memoize_reason[node->memoize_id];
            return MATCH_FALSE;
        }
    }
    enum match_result result;
    switch(node->type) {
        case AST_TYPE_IS_NULL_EXPR:
            result = match_is_null_expr_tri_err(preds, node->is_null_expr, last_reason);
            break;
        case AST_TYPE_SPECIAL_EXPR: {
            result = match_special_expr_tri_err(
                preds, node->special_expr, last_reason, attr_domain_count);
            break;
        }
        case AST_TYPE_BOOL_EXPR: {
            result = match_bool_expr_tri_err(preds, node->bool_expr, memoize, report, last_reason,
                memoize_reason, attr_domain_count);
            break;
        }
        case AST_TYPE_LIST_EXPR: {
            result = match_list_expr_tri_err(preds, node->list_expr, last_reason);
            break;
        }
        case AST_TYPE_SET_EXPR: {
            result = match_set_expr_tri_err(preds, node->set_expr, last_reason);
            break;
        }
        case AST_TYPE_COMPARE_EXPR: {
            result = match_compare_expr_tri_err(preds, node->compare_expr, last_reason);
            break;
        }
        case AST_TYPE_EQUALITY_EXPR: {
            result = match_equality_expr_tri_err(preds, node->equality_expr, last_reason);
            break;
        }
        default:
            std::abort();
    }
    // Memoization only caches settled (true/false) results: an unknown
    // result may become settled once more variables are fetched, and
    // caching it would incorrectly freeze that outcome across yields.
    if(node->memoize_id != INVALID_PRED && result != MATCH_UNKNOWN) {
        if(result == MATCH_TRUE) {
            set_bit(memoize->pass, node->memoize_id);
            memoize_reason[node->memoize_id] = *last_reason;
        }
        else {
            set_bit(memoize->fail, node->memoize_id);
            memoize_reason[node->memoize_id] = *last_reason;
        }
    }
    return result;
}

extern "C" {

void set_reason_sub_id_list(char* last_reason, const char* variable_name)
{
    std::sprintf(last_reason, "%s", variable_name);
}

bool match_node_err(const struct betree_variable** preds,
    const struct ast_node* node,
    struct memoize* memoize,
    struct report_err* report,
    betree_var_t* last_reason,
    betree_var_t* memoize_reason,
    std::size_t attr_domain_count)
{
    return match_node_inner_err(
        preds, node, memoize, report, last_reason, memoize_reason, attr_domain_count);
}

enum match_result match_node_tri_err(const struct betree_variable** preds,
    const struct ast_node* node,
    struct memoize* memoize,
    struct report_err* report,
    betree_var_t* last_reason,
    betree_var_t* memoize_reason,
    std::size_t attr_domain_count)
{
    return match_node_tri_inner_err(
        preds, node, memoize, report, last_reason, memoize_reason, attr_domain_count);
}

}

#include "ast_err.h"
