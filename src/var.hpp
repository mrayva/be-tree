#pragma once

#include <cstdint>

#include "value.h"

struct betree_variable;

// Maintain C struct layout for compatibility
struct attr_var {
    const char* attr;
    betree_var_t var;
    void* data;
};

// C API compatibility
#ifdef __cplusplus
extern "C" {
#endif

bool get_variable(betree_var_t var, const struct betree_variable** preds, struct value* value);
bool get_float_var(betree_var_t var, const struct betree_variable** preds, double* ret);
bool get_string_var(betree_var_t var, const struct betree_variable** preds, struct string_value* ret);
bool get_integer_enum_var(betree_var_t var, const struct betree_variable** preds, struct integer_enum_value* ret);
bool get_integer_var(betree_var_t var, const struct betree_variable** preds, std::int64_t* ret);
bool get_bool_var(betree_var_t var, const struct betree_variable** preds, bool* ret);
bool get_integer_list_var(betree_var_t var, const struct betree_variable** preds, struct betree_integer_list** ret);
bool get_string_list_var(betree_var_t var, const struct betree_variable** preds, struct betree_string_list** ret);
bool get_segments_var(betree_var_t var, const struct betree_variable** preds, struct betree_segments** ret);
bool get_frequency_var(betree_var_t var, const struct betree_variable** preds, struct betree_frequency_caps** ret);

bool is_empty_list(struct value value);

#ifdef __cplusplus
}
#endif
