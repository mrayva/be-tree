#pragma once

#include <cstddef>
#include <cstdint>

#include "value.h"  // For betree_sub_t

struct betree_err;
struct betree_sub;
struct betree_event;
struct report_err;
struct betree_reason_t;
struct betree_reason_map_t;

#ifdef __cplusplus
extern "C" {
#endif

void betree_init_err(struct betree_err* betree);
struct betree_err* betree_make_err();
struct betree_err* betree_make_with_parameters_err(
    std::uint64_t lnode_max_cap, std::uint64_t min_partition_size);

bool betree_make_sub_ids(struct betree_err* tree);

void betree_add_boolean_variable_err(
    struct betree_err* betree, const char* name, bool allow_undefined);
void betree_add_integer_variable_err(
    struct betree_err* betree, const char* name, bool allow_undefined, std::int64_t min, std::int64_t max);
void betree_add_float_variable_err(
    struct betree_err* betree, const char* name, bool allow_undefined, double min, double max);
void betree_add_string_variable_err(
    struct betree_err* betree, const char* name, bool allow_undefined, std::size_t count);
void betree_add_integer_list_variable_err(
    struct betree_err* betree, const char* name, bool allow_undefined, std::int64_t min, std::int64_t max);
void betree_add_integer_enum_variable_err(
    struct betree_err* betree, const char* name, bool allow_undefined, std::size_t count);
void betree_add_string_list_variable_err(
    struct betree_err* betree, const char* name, bool allow_undefined, std::size_t count);
void betree_add_segments_variable_err(
    struct betree_err* betree, const char* name, bool allow_undefined);
void betree_add_frequency_caps_variable_err(
    struct betree_err* betree, const char* name, bool allow_undefined);
struct betree_variable_definition betree_get_variable_definition_err(
    struct betree_err* betree, std::size_t index);

bool betree_change_boundaries_err(struct betree_err* tree, const char* expr);

const struct betree_sub* betree_make_sub_err(struct betree_err* tree,
    betree_sub_t id,
    std::size_t constant_count,
    const struct betree_constant** constants,
    const char* expr);
bool betree_insert_sub_err(struct betree_err* tree, const struct betree_sub* sub);
struct betree_event* betree_make_event_err(const struct betree_err* betree);

bool betree_insert_err(struct betree_err* tree, betree_sub_t id, const char* expr);
bool betree_insert_with_constants_err(struct betree_err* tree,
    betree_sub_t id,
    std::size_t constant_count,
    const struct betree_constant** constants,
    const char* expr);

bool betree_search_err(
    const struct betree_err* tree,
    const char* event_str,
    struct report_err* report);
bool betree_search_ids_err(const struct betree_err* tree,
    const char* event_str,
    struct report_err* report,
    const std::uint64_t* ids,
    std::size_t sz);
bool betree_search_with_event_err(
    const struct betree_err* betree,
    struct betree_event* event,
    struct report_err* report);
bool betree_search_with_event_ids_err(const struct betree_err* betree,
    struct betree_event* event,
    struct report_err* report,
    const std::uint64_t* ids,
    std::size_t sz);

struct report_err* make_report_err(const struct betree_err* betree);
void free_report_err(struct report_err* report);

void betree_deinit_err(struct betree_err* betree);
void betree_free_err(struct betree_err* betree);

struct betree_reason_t* betree_reason_create(const char* reason_name);
void betree_reason_destroy(struct betree_reason_t* reason);

struct betree_reason_map_t* betree_reason_map_create(const struct betree_err* betree);
unsigned int betree_reason_map_size(struct betree_reason_map_t* m);
dynamic_array_t* betree_reason_map_get(struct betree_reason_map_t* l, betree_var_t reason);
void betree_reason_map_additem(
    struct betree_reason_map_t* l, betree_var_t reason, betree_sub_t value);
void betree_reason_map_join(
    struct betree_reason_map_t* l, betree_var_t reason, dynamic_array_t* sub_ids);
void betree_reason_map_destroy(struct betree_reason_map_t* reason);

#ifdef __cplusplus
}
#endif
