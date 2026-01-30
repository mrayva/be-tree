#pragma once

#include <cstddef>
#include <cstdint>

#include "var.h"  // For betree_var_t, betree_ienum_t, betree_str_t

struct config;
struct attr_domain;

#ifdef __cplusplus
extern "C" {
#endif

struct config* make_config(std::uint8_t lnode_max_cap, std::uint8_t partition_min_size);
struct config* make_default_config();
void free_config(struct config* config);

void add_attr_domain_bounded_ranked_i(
    struct config* config, const char* attr, bool allow_undefined, std::int64_t min, std::int64_t max, int rank);
void add_attr_domain_bounded_i(
    struct config* config, const char* attr, bool allow_undefined, std::int64_t min, std::int64_t max);
void add_attr_domain_i(struct config* config, const char* attr, bool allow_undefined);
void add_attr_domain_ie(struct config* config, const char* attr, bool allow_undefined);

void add_attr_domain_bounded_ranked_f(
    struct config* config, const char* attr, bool allow_undefined, double min, double max, int rank);
void add_attr_domain_bounded_f(
    struct config* config, const char* attr, bool allow_undefined, double min, double max);
void add_attr_domain_f(struct config* config, const char* attr, bool allow_undefined);

void add_attr_domain_ranked_b(struct config* config, const char* attr, bool allow_undefined, int rank);
void add_attr_domain_b(struct config* config, const char* attr, bool allow_undefined);

void add_attr_domain_s(struct config* config, const char* attr, bool allow_undefined);
void add_attr_domain_bounded_ranked_s(
    struct config* config, const char* attr, bool allow_undefined, std::size_t max, int rank);
void add_attr_domain_bounded_s(
    struct config* config, const char* attr, bool allow_undefined, std::size_t max);

void add_attr_domain_bounded_ranked_ie(
    struct config* config, const char* attr, bool allow_undefined, std::size_t max, int rank);
void add_attr_domain_bounded_ie(
    struct config* config, const char* attr, bool allow_undefined, std::size_t max);

void add_attr_domain_il(struct config* config, const char* attr, bool allow_undefined);
void add_attr_domain_bounded_ranked_il(
    struct config* config, const char* attr, bool allow_undefined, std::int64_t min, std::int64_t max, int rank);
void add_attr_domain_bounded_il(
    struct config* config, const char* attr, bool allow_undefined, std::int64_t min, std::int64_t max);

void add_attr_domain_sl(struct config* config, const char* attr, bool allow_undefined);
void add_attr_domain_bounded_ranked_sl(
    struct config* config, const char* attr, bool allow_undefined, std::size_t max, int rank);
void add_attr_domain_bounded_sl(
    struct config* config, const char* attr, bool allow_undefined, std::size_t max);

void add_attr_domain_ranked_segments(struct config* config, const char* attr, bool allow_undefined, int rank);
void add_attr_domain_segments(struct config* config, const char* attr, bool allow_undefined);

void add_attr_domain_ranked_frequency(struct config* config, const char* attr, bool allow_undefined, int rank);
void add_attr_domain_frequency(struct config* config, const char* attr, bool allow_undefined);

const struct attr_domain* get_attr_domain(
    const struct attr_domain** attr_domains, betree_var_t variable_id);

betree_ienum_t try_get_id_for_ienum(
    const struct config* config, struct attr_var attr_var, std::int64_t integer);
betree_str_t try_get_id_for_string(
    const struct config* config, struct attr_var attr_var, const char* string);
betree_ienum_t get_id_for_ienum(struct config* config, struct attr_var attr_var, std::int64_t integer, bool always_assign);
betree_str_t get_id_for_string(struct config* config, struct attr_var attr_var, const char* string, bool always_assign);

bool is_variable_allow_undefined(const struct config* config, betree_var_t variable_id);

#ifdef __cplusplus
}
#endif
