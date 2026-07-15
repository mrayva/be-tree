#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "map.h"
#include "var.h"

// When compiling as C++, use the modern C++ header for function declarations
#ifdef __cplusplus
#include "config.hpp"
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct attr_domain {
    struct attr_var attr_var;
    struct value_bound bound;
    bool allow_undefined;
    int rank;
};

struct ast_node;
struct pred_map;

typedef map_t(betree_str_t) str_map_t;

struct string_map {
    struct attr_var attr_var;
    size_t string_value_count;
    str_map_t m;
};

struct integer_map {
    struct attr_var attr_var;
    size_t integer_value_count;
    int64_t* integer_values;
};

struct config* make_config(uint8_t lnode_max_cap, uint8_t partition_min_size);
struct config* make_default_config(void);
void free_config(struct config* config);

struct config {
    uint8_t lnode_max_cap;
    uint8_t partition_min_size;
    uint32_t max_domain_for_split;
    size_t attr_domain_count;
    struct attr_domain** attr_domains;
    size_t string_map_count;
    struct string_map* string_maps;
    size_t integer_map_count;
    struct integer_map* integer_maps;
    struct pred_map* pred_map;
};

void add_attr_domain_i(struct config* config, const char* attr, bool allow_undefined);
void add_attr_domain_f(struct config* config, const char* attr, bool allow_undefined);
void add_attr_domain_b(struct config* config, const char* attr, bool allow_undefined);
void add_attr_domain_s(struct config* config, const char* attr, bool allow_undefined);
void add_attr_domain_il(struct config* config, const char* attr, bool allow_undefined);
void add_attr_domain_sl(struct config* config, const char* attr, bool allow_undefined);
void add_attr_domain_segments(struct config* config, const char* attr, bool allow_undefined);
void add_attr_domain_frequency(struct config* config, const char* attr, bool allow_undefined);
void add_attr_domain_ie(struct config* config, const char* attr, bool allow_undefined);

void add_attr_domain_bounded_i(struct config* config, const char* attr, bool allow_undefined, int64_t min, int64_t max);
void add_attr_domain_bounded_f(struct config* config, const char* attr, bool allow_undefined, double min, double max);
void add_attr_domain_bounded_s(struct config* config, const char* attr, bool allow_undefined, size_t max);
void add_attr_domain_bounded_il(struct config* config, const char* attr, bool allow_undefined, int64_t min, int64_t max);
void add_attr_domain_bounded_sl(struct config* config, const char* attr, bool allow_undefined, size_t max);
void add_attr_domain_bounded_ie(struct config* config, const char* attr, bool allow_undefined, size_t max);

void add_attr_domain_bounded_ranked_i(
    struct config* config, const char* attr, bool allow_undefined, int64_t min, int64_t max, int rank);
void add_attr_domain_bounded_ranked_f(
    struct config* config, const char* attr, bool allow_undefined, double min, double max, int rank);
void add_attr_domain_ranked_b(struct config* config, const char* attr, bool allow_undefined, int rank);
void add_attr_domain_bounded_ranked_s(
    struct config* config, const char* attr, bool allow_undefined, size_t max, int rank);
void add_attr_domain_bounded_ranked_ie(
    struct config* config, const char* attr, bool allow_undefined, size_t max, int rank);
void add_attr_domain_bounded_ranked_il(
    struct config* config, const char* attr, bool allow_undefined, int64_t min, int64_t max, int rank);
void add_attr_domain_bounded_ranked_sl(
    struct config* config, const char* attr, bool allow_undefined, size_t max, int rank);

void add_attr_domain_ranked_segments(struct config* config, const char* attr, bool allow_undefined, int rank);
void add_attr_domain_ranked_frequency(struct config* config, const char* attr, bool allow_undefined, int rank);
const struct attr_domain* get_attr_domain(const struct attr_domain** attr_domains, betree_var_t variable_id);
bool is_variable_allow_undefined(const struct config* config, const betree_var_t variable_id);

const char* get_attr_for_id(const struct config* config, betree_var_t variable_id);
betree_var_t try_get_id_for_attr(const struct config* config, const char* attr);
betree_ienum_t try_get_id_for_ienum(const struct config* config, struct attr_var attr_var, int64_t integer);
betree_str_t try_get_id_for_string(const struct config* config, struct attr_var attr_var, const char* string);
betree_ienum_t get_id_for_ienum(struct config* config, struct attr_var attr_var, int64_t integer, bool always_assign);
betree_str_t get_id_for_string(struct config* config, struct attr_var attr_var, const char* string, bool always_assign);

struct attr_var make_attr_var(const char* attr, struct config* config);
struct attr_var copy_attr_var(struct attr_var attr_var);
void free_attr_var(struct attr_var attr_var);


#ifdef __cplusplus
}
#endif
