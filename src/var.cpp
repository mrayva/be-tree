#include "var.hpp"

#include "betree.h"
#include "error.h"
#include "tree.h"
#include "utils.h"

extern "C" {

bool get_variable(betree_var_t var, const struct betree_variable** preds, struct value* value)
{
    const auto* pred = preds[var];
    if (pred != nullptr) {
        *value = pred->value;
        return true;
    }
    return false;
}

bool get_float_var(betree_var_t var, const struct betree_variable** preds, double* ret)
{
    const auto* pred = preds[var];
    if (pred != nullptr) {
        *ret = pred->value.float_value;
        return true;
    }
    return false;
}

bool get_string_var(
    betree_var_t var, const struct betree_variable** preds, struct string_value* ret)
{
    const auto* pred = preds[var];
    if (pred != nullptr) {
        *ret = pred->value.string_value;
        return true;
    }
    return false;
}

bool get_integer_enum_var(
    betree_var_t var, const struct betree_variable** preds, struct integer_enum_value* ret)
{
    const auto* pred = preds[var];
    if (pred != nullptr) {
        *ret = pred->value.integer_enum_value;
        return true;
    }
    return false;
}

bool get_integer_var(betree_var_t var, const struct betree_variable** preds, std::int64_t* ret)
{
    const auto* pred = preds[var];
    if (pred != nullptr) {
        *ret = pred->value.integer_value;
        return true;
    }
    return false;
}

bool get_bool_var(betree_var_t var, const struct betree_variable** preds, bool* ret)
{
    const auto* pred = preds[var];
    if (pred != nullptr) {
        *ret = pred->value.boolean_value;
        return true;
    }
    return false;
}

bool get_integer_list_var(
    betree_var_t var, const struct betree_variable** preds, struct betree_integer_list** ret)
{
    const auto* pred = preds[var];
    if (pred != nullptr) {
        *ret = pred->value.integer_list_value;
        return true;
    }
    return false;
}

bool get_string_list_var(
    betree_var_t var, const struct betree_variable** preds, struct betree_string_list** ret)
{
    const auto* pred = preds[var];
    if (pred != nullptr) {
        *ret = pred->value.string_list_value;
        return true;
    }
    return false;
}

bool get_segments_var(
    betree_var_t var, const struct betree_variable** preds, struct betree_segments** ret)
{
    const auto* pred = preds[var];
    if (pred != nullptr) {
        *ret = pred->value.segments_value;
        return true;
    }
    return false;
}

bool get_frequency_var(
    betree_var_t var, const struct betree_variable** preds, struct betree_frequency_caps** ret)
{
    const auto* pred = preds[var];
    if (pred != nullptr) {
        *ret = pred->value.frequency_caps_value;
        return true;
    }
    return false;
}

bool is_empty_list(struct value value)
{
    switch (value.value_type) {
        case BETREE_INTEGER_LIST:
            return value.integer_list_value != nullptr && value.integer_list_value->count == 0;
        case BETREE_STRING_LIST:
            return value.string_list_value != nullptr && value.string_list_value->count == 0;
        case BETREE_SEGMENTS:
            return value.segments_value != nullptr && value.segments_value->size == 0;
        case BETREE_FREQUENCY_CAPS:
            return value.frequency_caps_value != nullptr && value.frequency_caps_value->size == 0;
        default:
            return false;
    }
}

} // extern "C"
