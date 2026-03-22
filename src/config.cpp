#include "config.hpp"

#include <cfloat>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string_view>

#include "alloc.h"
#include "config.h"
#include "error.h"
#include "hashmap.h"
#include "memoize.h"
#include "utils.h"

static struct attr_domain* make_attr_domain(
    const char* attr, betree_var_t variable_id, struct value_bound bound, bool allow_undefined, int rank)
{
    auto* attr_domain = static_cast<struct attr_domain*>(bcalloc(sizeof(struct attr_domain)));
    if (attr_domain == nullptr) {
        std::fprintf(stderr, "%s bcalloc faild\n", __func__);
        std::abort();
    }
    attr_domain->attr_var.attr = bstrdup(attr);
    attr_domain->attr_var.var = variable_id;
    attr_domain->attr_var.data = nullptr;
    attr_domain->bound = bound;
    attr_domain->allow_undefined = allow_undefined;
    attr_domain->rank = rank;
    return attr_domain;
}

static void add_attr_domain(
    struct config* config, const char* attr, struct value_bound bound, bool allow_undefined, int rank)
{
    betree_var_t variable_id = config->attr_domain_count;
    auto* attr_domain = make_attr_domain(attr, variable_id, bound, allow_undefined, rank);
    if (config->attr_domain_count == 0) {
        config->attr_domains = static_cast<struct attr_domain**>(bcalloc(sizeof(struct attr_domain*)));
        if (config->attr_domains == nullptr) {
            std::fprintf(stderr, "%s bcalloc failed\n", __func__);
            std::abort();
        }
    }
    else {
        auto* attr_domains = static_cast<struct attr_domain**>(brealloc(
            config->attr_domains, sizeof(struct attr_domain*) * (config->attr_domain_count + 1)));
        if (attr_domains == nullptr) {
            std::fprintf(stderr, "%s brealloc failed\n", __func__);
            std::abort();
        }
        config->attr_domains = attr_domains;
    }
    config->attr_domains[config->attr_domain_count] = attr_domain;
    config->attr_domain_count++;
}

static void add_integer_map(struct attr_var attr_var, struct config* config)
{
    if (config->integer_map_count == 0) {
        config->integer_maps = static_cast<struct integer_map*>(bcalloc(sizeof(struct integer_map)));
        if (config->integer_maps == nullptr) {
            std::fprintf(stderr, "%s bcalloc failed\n", __func__);
            std::abort();
        }
    }
    else {
        auto* integer_maps
            = static_cast<struct integer_map*>(brealloc(config->integer_maps, sizeof(struct integer_map) * (config->integer_map_count + 1)));
        if (integer_maps == nullptr) {
            std::fprintf(stderr, "%s brealloc failed\n", __func__);
            std::abort();
        }
        config->integer_maps = integer_maps;
    }
    config->integer_maps[config->integer_map_count].attr_var.attr = bstrdup(attr_var.attr);
    config->integer_maps[config->integer_map_count].attr_var.var = attr_var.var;
    config->integer_maps[config->integer_map_count].integer_value_count = 0;
    config->integer_maps[config->integer_map_count].integer_values = nullptr;
    config->integer_map_count++;
}

static void add_string_map(struct attr_var attr_var, struct config* config)
{
    if (config->string_map_count == 0) {
        config->string_maps = static_cast<struct string_map*>(bcalloc(sizeof(struct string_map)));
        if (config->string_maps == nullptr) {
            std::fprintf(stderr, "%s bcalloc failed\n", __func__);
            std::abort();
        }
    }
    else {
        auto* string_maps
            = static_cast<struct string_map*>(brealloc(config->string_maps, sizeof(struct string_map) * (config->string_map_count + 1)));
        if (string_maps == nullptr) {
            std::fprintf(stderr, "%s brealloc failed\n", __func__);
            std::abort();
        }
        config->string_maps = string_maps;
    }
    config->string_maps[config->string_map_count].attr_var.attr = bstrdup(attr_var.attr);
    config->string_maps[config->string_map_count].attr_var.var = attr_var.var;
    config->string_maps[config->string_map_count].string_value_count = 0;
    config->string_map_count++;
}

static void add_to_integer_map(struct integer_map* integer_map, std::int64_t integer)
{
    if (integer_map->integer_value_count == 0) {
        integer_map->integer_values = static_cast<std::int64_t*>(bcalloc(sizeof(std::int64_t)));
        if (integer_map->integer_values == nullptr) {
            std::fprintf(stderr, "%s bcalloc failed\n", __func__);
            std::abort();
        }
    }
    else {
        auto* integer_values = static_cast<std::int64_t*>(brealloc(integer_map->integer_values,
            sizeof(std::int64_t) * (integer_map->integer_value_count + 1)));
        if (integer_values == nullptr) {
            std::fprintf(stderr, "%s brealloc failed\n", __func__);
            std::abort();
        }
        integer_map->integer_values = integer_values;
    }
    integer_map->integer_values[integer_map->integer_value_count] = integer;
    integer_map->integer_value_count++;
}

static void add_to_string_map(struct string_map* string_map, std::string_view string)
{
    if (string_map->string_value_count == 0) {
        map_init(&string_map->m);
    }
    map_set(&string_map->m, string.data(), string_map->string_value_count);
    string_map->string_value_count++;
}

extern "C" {

struct config* make_config(std::uint8_t lnode_max_cap, std::uint8_t partition_min_size)
{
    auto* config = static_cast<struct config*>(bcalloc(sizeof(struct config)));
    if (config == nullptr) {
        std::fprintf(stderr, "%s bcalloc failed\n", __func__);
        std::abort();
    }
    config->attr_domain_count = 0;
    config->attr_domains = nullptr;
    config->lnode_max_cap = lnode_max_cap;
    config->partition_min_size = partition_min_size;
    config->max_domain_for_split = 1000;
    config->string_map_count = 0;
    config->string_maps = nullptr;
    config->pred_map = make_pred_map();
    return config;
}

struct config* make_default_config()
{
    return make_config(3, 0);
}

void free_config(struct config* config)
{
    if (config == nullptr) {
        return;
    }
    if (config->attr_domains != nullptr) {
        for (std::size_t i = 0; i < config->attr_domain_count; i++) {
            bfree(const_cast<char*>(config->attr_domains[i]->attr_var.attr));
            bfree(config->attr_domains[i]);
        }
        bfree(config->attr_domains);
        config->attr_domains = nullptr;
    }
    if (config->integer_maps != nullptr) {
        for (std::size_t i = 0; i < config->integer_map_count; i++) {
            bfree(const_cast<char*>(config->integer_maps[i].attr_var.attr));
            bfree(config->integer_maps[i].integer_values);
        }
        bfree(config->integer_maps);
        config->integer_maps = nullptr;
    }
    if (config->string_maps != nullptr) {
        for (std::size_t i = 0; i < config->string_map_count; i++) {
            bfree(const_cast<char*>(config->string_maps[i].attr_var.attr));
            map_deinit(&config->string_maps[i].m);
        }
        bfree(config->string_maps);
        config->string_maps = nullptr;
    }
    if (config->pred_map != nullptr) {
        free_pred_map(config->pred_map);
        config->pred_map = nullptr;
    }
    bfree(config);
}

void add_attr_domain_bounded_ranked_i(
    struct config* config, const char* attr, bool allow_undefined, std::int64_t min, std::int64_t max, int rank)
{
    struct value_bound bound = {};
    bound.value_type = BETREE_INTEGER;
    bound.imin = min;
    bound.imax = max;
    add_attr_domain(config, attr, bound, allow_undefined, rank);
}

void add_attr_domain_bounded_i(
    struct config* config, const char* attr, bool allow_undefined, std::int64_t min, std::int64_t max)
{
    add_attr_domain_bounded_ranked_i(config, attr, allow_undefined, min, max, 0);
}

void add_attr_domain_i(struct config* config, const char* attr, bool allow_undefined)
{
    add_attr_domain_bounded_i(config, attr, allow_undefined, INT64_MIN, INT64_MAX);
}

void add_attr_domain_ie(struct config* config, const char* attr, bool allow_undefined)
{
    add_attr_domain_bounded_ie(config, attr, allow_undefined, SIZE_MAX);
}

void add_attr_domain_bounded_ranked_f(
    struct config* config, const char* attr, bool allow_undefined, double min, double max, int rank)
{
    struct value_bound bound = {};
    bound.value_type = BETREE_FLOAT;
    bound.fmin = min;
    bound.fmax = max;
    add_attr_domain(config, attr, bound, allow_undefined, rank);
}

void add_attr_domain_bounded_f(
    struct config* config, const char* attr, bool allow_undefined, double min, double max)
{
    add_attr_domain_bounded_ranked_f(config, attr, allow_undefined, min, max, 0);
}

void add_attr_domain_f(struct config* config, const char* attr, bool allow_undefined)
{
    add_attr_domain_bounded_f(config, attr, allow_undefined, -DBL_MAX, DBL_MAX);
}

void add_attr_domain_ranked_b(struct config* config, const char* attr, bool allow_undefined, int rank)
{
    struct value_bound bound = {};
    bound.value_type = BETREE_BOOLEAN;
    bound.bmin = false;
    bound.bmax = true;
    add_attr_domain(config, attr, bound, allow_undefined, rank);
}

void add_attr_domain_b(struct config* config, const char* attr, bool allow_undefined)
{
    add_attr_domain_ranked_b(config, attr, allow_undefined, 0);
}

void add_attr_domain_s(struct config* config, const char* attr, bool allow_undefined)
{
    add_attr_domain_bounded_s(config, attr, allow_undefined, SIZE_MAX);
}

void add_attr_domain_bounded_ranked_s(
    struct config* config, const char* attr, bool allow_undefined, std::size_t max, int rank)
{
    struct value_bound bound = {};
    bound.value_type = BETREE_STRING;
    bound.smin = 0;
    bound.smax = max - 1;
    add_attr_domain(config, attr, bound, allow_undefined, rank);
}

void add_attr_domain_bounded_s(
    struct config* config, const char* attr, bool allow_undefined, std::size_t max)
{
    add_attr_domain_bounded_ranked_s(config, attr, allow_undefined, max, 0);
}

void add_attr_domain_bounded_ranked_ie(
    struct config* config, const char* attr, bool allow_undefined, std::size_t max, int rank)
{
    struct value_bound bound = {};
    bound.value_type = BETREE_INTEGER_ENUM;
    bound.smin = 0;
    bound.smax = max - 1;
    add_attr_domain(config, attr, bound, allow_undefined, rank);
}

void add_attr_domain_bounded_ie(
    struct config* config, const char* attr, bool allow_undefined, std::size_t max)
{
    add_attr_domain_bounded_ranked_ie(config, attr, allow_undefined, max, 0);
}

void add_attr_domain_il(struct config* config, const char* attr, bool allow_undefined)
{
    add_attr_domain_bounded_il(config, attr, allow_undefined, INT64_MIN, INT64_MAX);
}

void add_attr_domain_bounded_ranked_il(
    struct config* config, const char* attr, bool allow_undefined, std::int64_t min, std::int64_t max, int rank)
{
    struct value_bound bound = {};
    bound.value_type = BETREE_INTEGER_LIST;
    bound.imin = min;
    bound.imax = max;
    add_attr_domain(config, attr, bound, allow_undefined, rank);
}

void add_attr_domain_bounded_il(
    struct config* config, const char* attr, bool allow_undefined, std::int64_t min, std::int64_t max)
{
    add_attr_domain_bounded_ranked_il(config, attr, allow_undefined, min, max, 0);
}

void add_attr_domain_sl(struct config* config, const char* attr, bool allow_undefined)
{
    add_attr_domain_bounded_sl(config, attr, allow_undefined, SIZE_MAX);
}

void add_attr_domain_bounded_ranked_sl(
    struct config* config, const char* attr, bool allow_undefined, std::size_t max, int rank)
{
    struct value_bound bound = {};
    bound.value_type = BETREE_STRING_LIST;
    bound.smin = 0;
    bound.smax = max - 1;
    add_attr_domain(config, attr, bound, allow_undefined, rank);
}

void add_attr_domain_bounded_sl(
    struct config* config, const char* attr, bool allow_undefined, std::size_t max)
{
    add_attr_domain_bounded_ranked_sl(config, attr, allow_undefined, max, 0);
}

void add_attr_domain_ranked_segments(struct config* config, const char* attr, bool allow_undefined, int rank)
{
    struct value_bound bound = {};
    bound.value_type = BETREE_SEGMENTS;
    add_attr_domain(config, attr, bound, allow_undefined, rank);
}

void add_attr_domain_segments(struct config* config, const char* attr, bool allow_undefined)
{
    add_attr_domain_ranked_segments(config, attr, allow_undefined, 0);
}

void add_attr_domain_ranked_frequency(struct config* config, const char* attr, bool allow_undefined, int rank)
{
    struct value_bound bound = {};
    bound.value_type = BETREE_FREQUENCY_CAPS;
    add_attr_domain(config, attr, bound, allow_undefined, rank);
}

void add_attr_domain_frequency(struct config* config, const char* attr, bool allow_undefined)
{
    add_attr_domain_ranked_frequency(config, attr, allow_undefined, 0);
}

const struct attr_domain* get_attr_domain(
    const struct attr_domain** attr_domains, betree_var_t variable_id)
{
    return attr_domains[variable_id];
}

betree_ienum_t try_get_id_for_ienum(
    const struct config* config, struct attr_var attr_var, std::int64_t integer)
{
    for (std::size_t i = 0; i < config->integer_map_count; i++) {
        if (config->integer_maps[i].attr_var.var == attr_var.var) {
            auto* integer_map = &config->integer_maps[i];
            for (std::size_t j = 0; j < integer_map->integer_value_count; j++) {
                if (integer_map->integer_values[j] == integer) {
                    return j;
                }
            }
            break;
        }
    }
    return INVALID_IENUM;
}

betree_str_t try_get_id_for_string(
    const struct config* config, struct attr_var attr_var, const char* string)
{
    for (std::size_t i = 0; i < config->string_map_count; i++) {
        if (config->string_maps[i].attr_var.var == attr_var.var) {
            auto* string_map = &config->string_maps[i];
            auto* str = static_cast<betree_str_t*>(map_get_(&string_map->m.base, string));
            if (str != nullptr) {
                return *str;
            }
            break;
        }
    }
    return INVALID_STR;
}

betree_ienum_t get_id_for_ienum(struct config* config, struct attr_var attr_var, std::int64_t integer, bool always_assign)
{
    struct integer_map* integer_map = nullptr;
    for (std::size_t i = 0; i < config->integer_map_count; i++) {
        if (config->integer_maps[i].attr_var.var == attr_var.var) {
            integer_map = &config->integer_maps[i];
            for (std::size_t j = 0; j < integer_map->integer_value_count; j++) {
                if (integer_map->integer_values[j] == integer) {
                    return j;
                }
            }
            break;
        }
    }
    if (integer_map == nullptr) {
        add_integer_map(attr_var, config);
        integer_map = &config->integer_maps[config->integer_map_count - 1];
    }
    auto* attr_domain
        = get_attr_domain(const_cast<const struct attr_domain**>(config->attr_domains), attr_var.var);
    if (!always_assign && attr_domain->bound.smax + 1 == integer_map->integer_value_count) {
        return INVALID_IENUM;
    }
    add_to_integer_map(integer_map, integer);
    return integer_map->integer_value_count - 1;
}

betree_str_t get_id_for_string(struct config* config, struct attr_var attr_var, const char* string, bool always_assign)
{
    struct string_map* string_map = nullptr;
    for (std::size_t i = 0; i < config->string_map_count; i++) {
        if (config->string_maps[i].attr_var.var == attr_var.var) {
            string_map = &config->string_maps[i];
            auto* str = static_cast<betree_str_t*>(map_get_(&string_map->m.base, string));
            if (str != nullptr) {
                return *str;
            }
            break;
        }
    }
    if (string_map == nullptr) {
        add_string_map(attr_var, config);
        string_map = &config->string_maps[config->string_map_count - 1];
    }
    auto* attr_domain
        = get_attr_domain(const_cast<const struct attr_domain**>(config->attr_domains), attr_var.var);
    if (!always_assign && attr_domain->bound.smax + 1 == string_map->string_value_count) {
        return INVALID_STR;
    }
    add_to_string_map(string_map, string);
    return string_map->string_value_count - 1;
}

bool is_variable_allow_undefined(const struct config* config, betree_var_t variable_id)
{
    return config->attr_domains[variable_id]->allow_undefined;
}

} // extern "C"
