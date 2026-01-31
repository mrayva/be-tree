#pragma once

#include <cstddef>
#include <cstdint>

// Note: betree_sub_t, betree_var_t, and struct definitions are in betree.h
// which is included before this file when compiling as C++

struct betree;
struct betree_sub;
struct betree_constant;
struct betree_variable;
struct betree_event;
struct report;
struct report_counting;
struct betree_integer_list;
struct betree_string_list;
struct betree_segments;
struct betree_segment;
struct betree_frequency_caps;
struct betree_frequency_cap;
struct betree_variable_definition;

#ifdef __cplusplus
extern "C" {
#endif

struct betree_integer_list* betree_make_integer_list(std::size_t count);
void betree_add_integer(struct betree_integer_list* list, std::size_t index, std::int64_t value);

struct betree_string_list* betree_make_string_list(std::size_t count);
void betree_add_string(struct betree_string_list* list, std::size_t index, const char* value);

struct betree_segments* betree_make_segments(std::size_t count);
struct betree_segment* betree_make_segment(std::int64_t id, std::int64_t timestamp);
void betree_add_segment(struct betree_segments* segments, std::size_t index, struct betree_segment* segment);

struct betree_frequency_caps* betree_make_frequency_caps(std::size_t count);
struct betree_frequency_cap* betree_make_frequency_cap(const char* stype, std::uint32_t id, const char* ns, bool timestamp_defined, std::int64_t timestamp, std::uint32_t value);
void betree_add_frequency_cap(struct betree_frequency_caps* frequency_caps, std::size_t index, struct betree_frequency_cap* frequency_cap);

const struct betree_variable** make_environment(std::size_t attr_domain_count, const struct betree_event* event);

void betree_init(struct betree* betree);
struct betree* betree_make();
struct betree* betree_make_with_parameters(std::uint64_t lnode_max_cap, std::uint64_t min_partition_size);

void betree_add_boolean_variable(struct betree* betree, const char* name, bool allow_undefined);
void betree_add_integer_variable(struct betree* betree, const char* name, bool allow_undefined, std::int64_t min, std::int64_t max);
void betree_add_float_variable(struct betree* betree, const char* name, bool allow_undefined, double min, double max);
void betree_add_string_variable(struct betree* betree, const char* name, bool allow_undefined, std::size_t count);
void betree_add_integer_list_variable(struct betree* betree, const char* name, bool allow_undefined, std::int64_t min, std::int64_t max);
void betree_add_integer_enum_variable(struct betree* betree, const char* name, bool allow_undefined, std::size_t count);
void betree_add_string_list_variable(struct betree* betree, const char* name, bool allow_undefined, std::size_t count);
void betree_add_segments_variable(struct betree* betree, const char* name, bool allow_undefined);
void betree_add_frequency_caps_variable(struct betree* betree, const char* name, bool allow_undefined);

void betree_add_ranked_boolean_variable(
    struct betree* betree, const char* name, bool allow_undefined, int ranked);
void betree_add_ranked_integer_variable(
    struct betree* betree, const char* name, bool allow_undefined, std::int64_t min, std::int64_t max, int rank);
void betree_add_ranked_float_variable(
    struct betree* betree, const char* name, bool allow_undefined, double min, double max, int rank);
void betree_add_ranked_string_variable(
    struct betree* betree, const char* name, bool allow_undefined, std::size_t count, int rank);
void betree_add_ranked_integer_list_variable(
    struct betree* betree, const char* name, bool allow_undefined, std::int64_t min, std::int64_t max, int rank);
void betree_add_ranked_integer_enum_variable(
    struct betree* betree, const char* name, bool allow_undefined, std::size_t count, int rank);
void betree_add_ranked_string_list_variable(
    struct betree* betree, const char* name, bool allow_undefined, std::size_t count, int rank);
void betree_add_ranked_segments_variable(
    struct betree* betree, const char* name, bool allow_undefined, int rank);
void betree_add_ranked_frequency_caps_variable(
    struct betree* betree, const char* name, bool allow_undefined, int rank);

bool betree_change_boundaries(struct betree* tree, const char* expr);

struct betree_sub* betree_make_sub(struct betree* tree, betree_sub_t id, std::size_t constant_count, const struct betree_constant** constants, const char* expr);
bool betree_insert_sub(struct betree* tree, const struct betree_sub* sub);

struct betree_variable_definition betree_get_variable_definition(struct betree* betree, std::size_t index);

struct betree_constant* betree_make_integer_constant(const char* name, std::int64_t integer_value);

struct betree_variable* betree_make_boolean_variable(const char* name, bool value);
struct betree_variable* betree_make_integer_variable(const char* name, std::int64_t value);
struct betree_variable* betree_make_float_variable(const char* name, double value);
struct betree_variable* betree_make_string_variable(const char* name, const char* value);
struct betree_variable* betree_make_integer_list_variable(const char* name, struct betree_integer_list* value);
struct betree_variable* betree_make_string_list_variable(const char* name, struct betree_string_list* value);
struct betree_variable* betree_make_segments_variable(const char* name, struct betree_segments* value);
struct betree_variable* betree_make_frequency_caps_variable(const char* name, struct betree_frequency_caps* value);

struct betree_event* betree_make_event(const struct betree* betree);
void betree_set_variable(struct betree_event* event, std::size_t index, struct betree_variable* variable);

bool betree_insert(struct betree* tree, betree_sub_t id, const char* expr);
bool betree_insert_with_constants(struct betree* tree, betree_sub_t id, std::size_t constant_count, const struct betree_constant** constants, const char* expr);

bool betree_search(const struct betree* tree, const char* event_str, struct report* report);
bool betree_search_ids(const struct betree* tree, const char* event_str, struct report* report, const std::uint64_t* ids, std::size_t sz);
bool betree_search_with_event(const struct betree* betree, struct betree_event* event, struct report* report);
bool betree_search_with_event_ids(const struct betree* betree, struct betree_event* event, struct report* report, const std::uint64_t* ids, std::size_t sz);

bool betree_exists(const struct betree* tree, const char* event_str);
bool betree_exists_with_event(const struct betree* betree, struct betree_event* event);

struct report* make_report();
void free_report(struct report* report);

struct report_counting* make_report_counting();
void free_report_counting(struct report_counting* report);

void betree_deinit(struct betree* betree);
void betree_free(struct betree* betree);

void betree_free_constant(struct betree_constant* constant);
void betree_free_constants(std::size_t count, struct betree_constant** constants);

void betree_free_variable(struct betree_variable* variable);
void betree_free_event(struct betree_event* event);

void betree_free_integer_list(struct betree_integer_list* value);
void betree_free_string_list(struct betree_string_list* value);
void betree_free_segment(struct betree_segment* value);
void betree_free_segments(struct betree_segments* value);
void betree_free_frequency_cap(struct betree_frequency_cap* value);
void betree_free_frequency_caps(struct betree_frequency_caps* value);

void betree_prepare_sub_data(struct betree* tree);

#ifdef __cplusplus
}
#endif
