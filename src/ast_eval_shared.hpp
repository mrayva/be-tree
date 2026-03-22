#pragma once

#include <cstdio>
#include <cstdlib>

#include "ast.h"
#include "ast_match_shared.hpp"
#include "utils.h"
#include "var.h"

namespace ast_eval_shared {

inline void invalid_expr(const char* msg)
{
    std::fprintf(stderr, "%s", msg);
    std::abort();
}

template <typename OnReason>
inline bool match_is_null_expr(const struct betree_variable** preds,
    const struct ast_is_null_expr is_null_expr,
    OnReason&& on_reason)
{
    on_reason(is_null_expr.attr_var.var);
    struct value variable;
    bool is_variable_defined = get_variable(is_null_expr.attr_var.var, preds, &variable);
    switch (is_null_expr.type) {
        case AST_IS_NULL:
            return !is_variable_defined;
        case AST_IS_NOT_NULL:
            return is_variable_defined;
        case AST_IS_EMPTY:
            return is_variable_defined && is_empty_list(variable);
        default:
            std::abort();
    }
}

template <typename OnReason>
inline bool match_list_expr(const struct betree_variable** preds,
    const struct ast_list_expr list_expr,
    OnReason&& on_reason)
{
    on_reason(list_expr.attr_var.var);
    struct value variable;
    bool is_variable_defined = get_variable(list_expr.attr_var.var, preds, &variable);
    if (!is_variable_defined) {
        return false;
    }
    switch (list_expr.op) {
        case AST_LIST_ONE_OF:
        case AST_LIST_NONE_OF: {
            bool result = false;
            switch (list_expr.value.value_type) {
                case AST_LIST_VALUE_INTEGER_LIST:
                    result = ast_match_not_all_of_int(variable, list_expr);
                    break;
                case AST_LIST_VALUE_STRING_LIST:
                    result = ast_match_not_all_of_string(variable, list_expr);
                    break;
                default:
                    std::abort();
            }
            switch (list_expr.op) {
                case AST_LIST_ONE_OF:
                    return result;
                case AST_LIST_NONE_OF:
                    return !result;
                case AST_LIST_ALL_OF:
                    invalid_expr("Should never happen");
                    return false;
                default:
                    std::abort();
            }
        }
        case AST_LIST_ALL_OF:
            switch (list_expr.value.value_type) {
                case AST_LIST_VALUE_INTEGER_LIST:
                    return ast_match_all_of_int(variable, list_expr);
                case AST_LIST_VALUE_STRING_LIST:
                    return ast_match_all_of_string(variable, list_expr);
                default:
                    std::abort();
            }
        default:
            std::abort();
    }
}

template <typename OnReason>
inline bool match_set_expr(const struct betree_variable** preds,
    const struct ast_set_expr set_expr,
    OnReason&& on_reason)
{
    struct set_left_value left = set_expr.left_value;
    struct set_right_value right = set_expr.right_value;
    bool is_in;
    if (left.value_type == AST_SET_LEFT_VALUE_INTEGER
        && right.value_type == AST_SET_RIGHT_VALUE_VARIABLE) {
        struct betree_integer_list* variable;
        on_reason(right.variable_value.var);
        bool is_variable_defined = get_integer_list_var(right.variable_value.var, preds, &variable);
        if (!is_variable_defined) {
            return false;
        }
        is_in = ast_match_integer_in_integer_list(left.integer_value, variable);
    } else if (left.value_type == AST_SET_LEFT_VALUE_STRING
        && right.value_type == AST_SET_RIGHT_VALUE_VARIABLE) {
        struct betree_string_list* variable;
        on_reason(right.variable_value.var);
        bool is_variable_defined = get_string_list_var(right.variable_value.var, preds, &variable);
        if (!is_variable_defined) {
            return false;
        }
        is_in = ast_match_string_in_string_list(left.string_value, variable);
    } else if (left.value_type == AST_SET_LEFT_VALUE_VARIABLE
        && right.value_type == AST_SET_RIGHT_VALUE_INTEGER_LIST) {
        std::int64_t variable;
        on_reason(left.variable_value.var);
        bool is_variable_defined = get_integer_var(left.variable_value.var, preds, &variable);
        if (!is_variable_defined) {
            return false;
        }
        is_in = ast_match_integer_in_integer_list(variable, right.integer_list_value);
    } else if (left.value_type == AST_SET_LEFT_VALUE_VARIABLE
        && right.value_type == AST_SET_RIGHT_VALUE_STRING_LIST) {
        struct string_value variable;
        on_reason(left.variable_value.var);
        bool is_variable_defined = get_string_var(left.variable_value.var, preds, &variable);
        if (!is_variable_defined) {
            return false;
        }
        is_in = ast_match_string_in_string_list(variable, right.string_list_value);
    } else {
        invalid_expr("invalid set expression");
        return false;
    }
    switch (set_expr.op) {
        case AST_SET_NOT_IN:
            return !is_in;
        case AST_SET_IN:
            return is_in;
        default:
            std::abort();
    }
}

template <typename OnReason>
inline bool match_compare_expr(const struct betree_variable** preds,
    const struct ast_compare_expr compare_expr,
    OnReason&& on_reason)
{
    on_reason(compare_expr.attr_var.var);
    struct value variable;
    bool is_variable_defined = get_variable(compare_expr.attr_var.var, preds, &variable);
    if (!is_variable_defined) {
        return false;
    }
    switch (compare_expr.op) {
        case AST_COMPARE_LT:
            switch (compare_expr.value.value_type) {
                case AST_COMPARE_VALUE_INTEGER:
                    return variable.integer_value < compare_expr.value.integer_value;
                case AST_COMPARE_VALUE_FLOAT:
                    return variable.float_value < compare_expr.value.float_value;
                default:
                    std::abort();
            }
        case AST_COMPARE_LE:
            switch (compare_expr.value.value_type) {
                case AST_COMPARE_VALUE_INTEGER:
                    return variable.integer_value <= compare_expr.value.integer_value;
                case AST_COMPARE_VALUE_FLOAT:
                    return variable.float_value <= compare_expr.value.float_value;
                default:
                    std::abort();
            }
        case AST_COMPARE_GT:
            switch (compare_expr.value.value_type) {
                case AST_COMPARE_VALUE_INTEGER:
                    return variable.integer_value > compare_expr.value.integer_value;
                case AST_COMPARE_VALUE_FLOAT:
                    return variable.float_value > compare_expr.value.float_value;
                default:
                    std::abort();
            }
        case AST_COMPARE_GE:
            switch (compare_expr.value.value_type) {
                case AST_COMPARE_VALUE_INTEGER:
                    return variable.integer_value >= compare_expr.value.integer_value;
                case AST_COMPARE_VALUE_FLOAT:
                    return variable.float_value >= compare_expr.value.float_value;
                default:
                    std::abort();
            }
        default:
            std::abort();
    }
}

template <typename OnReason>
inline bool match_equality_expr(const struct betree_variable** preds,
    const struct ast_equality_expr equality_expr,
    OnReason&& on_reason)
{
    on_reason(equality_expr.attr_var.var);
    struct value variable;
    bool is_variable_defined = get_variable(equality_expr.attr_var.var, preds, &variable);
    if (!is_variable_defined) {
        return false;
    }
    switch (equality_expr.op) {
        case AST_EQUALITY_EQ:
            switch (equality_expr.value.value_type) {
                case AST_EQUALITY_VALUE_INTEGER:
                    return variable.integer_value == equality_expr.value.integer_value;
                case AST_EQUALITY_VALUE_FLOAT:
                    return feq(variable.float_value, equality_expr.value.float_value);
                case AST_EQUALITY_VALUE_STRING:
                    return variable.string_value.str == equality_expr.value.string_value.str;
                case AST_EQUALITY_VALUE_INTEGER_ENUM:
                    return variable.integer_enum_value.ienum
                        == equality_expr.value.integer_enum_value.ienum;
                default:
                    std::abort();
            }
        case AST_EQUALITY_NE:
            switch (equality_expr.value.value_type) {
                case AST_EQUALITY_VALUE_INTEGER:
                    return variable.integer_value != equality_expr.value.integer_value;
                case AST_EQUALITY_VALUE_FLOAT:
                    return fne(variable.float_value, equality_expr.value.float_value);
                case AST_EQUALITY_VALUE_STRING:
                    return variable.string_value.str != equality_expr.value.string_value.str;
                case AST_EQUALITY_VALUE_INTEGER_ENUM:
                    return variable.integer_enum_value.ienum
                        != equality_expr.value.integer_enum_value.ienum;
                default:
                    std::abort();
            }
        default:
            std::abort();
    }
}

} // namespace ast_eval_shared
