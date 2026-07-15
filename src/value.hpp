#pragma once

#include <cstddef>
#include <cstdint>

struct betree_integer_list;
struct betree_string_list;
struct betree_segments;
struct betree_frequency_caps;
struct betree_segment;
struct betree_frequency_cap;
struct string_value;
struct value;

#ifdef __cplusplus
extern "C" {
#endif

struct betree_integer_list* make_integer_list(void);
struct betree_string_list* make_string_list(void);
struct betree_segments* make_segments(void);
struct betree_frequency_caps* make_frequency_caps(void);

void add_integer_list_value(std::int64_t integer, struct betree_integer_list* list);
char* integer_list_value_to_string(struct betree_integer_list* list);

void add_string_list_value(struct string_value string, struct betree_string_list* list);
char* string_list_value_to_string(struct betree_string_list* list);

void add_segment(struct betree_segment* segment, struct betree_segments* list);
void add_frequency(struct betree_frequency_cap* frequency, struct betree_frequency_caps* list);

struct betree_segment* make_segment(std::int64_t id, std::int64_t timestamp);
struct betree_frequency_cap* make_frequency_cap(const char* stype,
    std::uint32_t id,
    struct string_value ns,
    bool timestamp_defined,
    std::int64_t timestamp,
    std::uint32_t value);
struct betree_frequency_cap* make_frequency_cap_with_type(enum frequency_type_e type,
    std::uint32_t id,
    struct string_value ns,
    bool timestamp_defined,
    std::int64_t timestamp,
    std::uint32_t value);

enum frequency_type_e get_type_from_string(const char* stype);

void free_integer_list(struct betree_integer_list* value);
void free_string_list(struct betree_string_list* value);
void free_segment(struct betree_segment* value);
void free_segments(struct betree_segments* value);
void free_frequency_cap(struct betree_frequency_cap* value);
void free_frequency_caps(struct betree_frequency_caps* value);
void free_value(struct value value);

char* segment_value_to_string(struct betree_segment* segment);
char* segments_value_to_string(struct betree_segments* list);
char* frequency_cap_to_string(struct betree_frequency_cap* cap);
char* frequency_caps_value_to_string(struct betree_frequency_caps* list);

void remove_duplicates_integer_list(struct betree_integer_list* list);
void sort_integer_list(struct betree_integer_list* list);
void sort_and_remove_duplicate_integer_list(struct betree_integer_list* list);

void remove_duplicates_string_list(struct betree_string_list* list);
void sort_string_list(struct betree_string_list* list);
void sort_and_remove_duplicate_string_list(struct betree_string_list* list);

#ifdef __cplusplus
}
#endif
