#pragma once

#include <bit>
#include <cstddef>
#include <cstdint>

#include "tree.h"

static inline void add_sub_to_eval(struct betree_sub* sub, struct subs_to_eval* subs)
{
    if (subs->capacity == subs->count) {
        subs->capacity *= 2;
        subs->subs = static_cast<struct betree_sub**>(
            brealloc(subs->subs, sizeof(*subs->subs) * subs->capacity));
    }

    subs->subs[subs->count] = sub;
    subs->count++;
}

static inline bool is_id_in(std::uint64_t id, const std::uint64_t* ids, std::size_t sz)
{
    if (sz == 0) {
        return false;
    }
    std::size_t first = 0;
    std::size_t last = sz - 1;
    if (id < ids[first] || id > ids[last]) {
        return false;
    }
    std::size_t middle = (first + last) / 2;
    while (first <= last) {
        if (id == ids[middle]) {
            return true;
        }
        if (ids[middle] < id) {
            first = middle + 1;
        } else {
            last = middle - 1;
        }
        middle = (first + last) / 2;
    }
    return false;
}

static inline bool event_contains_variable(
    const struct betree_variable** preds, betree_var_t variable_id)
{
    return preds[variable_id] != nullptr;
}

enum short_circuit_e { SHORT_CIRCUIT_PASS, SHORT_CIRCUIT_FAIL, SHORT_CIRCUIT_NONE };

static inline enum short_circuit_e try_short_circuit(
    std::size_t attr_domains_count,
    const struct short_circuit* short_circuit,
    const std::uint64_t* undefined)
{
    std::size_t count = (attr_domains_count + 63) / 64;
    for (std::size_t i = 0; i < count; i++) {
        if (short_circuit->pass[i] & undefined[i]) {
            return SHORT_CIRCUIT_PASS;
        }
        if (short_circuit->fail[i] & undefined[i]) {
            return SHORT_CIRCUIT_FAIL;
        }
    }
    return SHORT_CIRCUIT_NONE;
}

static inline enum short_circuit_e try_short_circuit_(
    std::size_t attr_domains_count,
    const struct short_circuit* short_circuit,
    const std::uint64_t* undefined,
    betree_var_t* last_var)
{
    std::size_t count = (attr_domains_count + 63) / 64;
    for (std::size_t i = 0; i < count; i++) {
        std::uint64_t pass_mask = short_circuit->pass[i] & undefined[i];
        if (pass_mask) {
            *last_var = i * 64 + std::countr_zero(pass_mask);
            return SHORT_CIRCUIT_PASS;
        }
        std::uint64_t fail_mask = short_circuit->fail[i] & undefined[i];
        if (fail_mask) {
            *last_var = i * 64 + std::countr_zero(fail_mask);
            return SHORT_CIRCUIT_FAIL;
        }
    }
    return SHORT_CIRCUIT_NONE;
}

static inline enum short_circuit_e try_short_circuit_err(
    std::size_t attr_domains_count,
    const struct short_circuit* short_circuit,
    const std::uint64_t* undefined,
    betree_var_t* last_reason)
{
    std::size_t count = (attr_domains_count + 63) / 64;
    for (std::size_t i = 0; i < count; i++) {
        if (short_circuit->pass[i] & undefined[i]) {
            return SHORT_CIRCUIT_PASS;
        }
        std::uint64_t fail_mask = short_circuit->fail[i] & undefined[i];
        if (fail_mask) {
            *last_reason = i * 64 + std::countr_zero(fail_mask);
            return SHORT_CIRCUIT_FAIL;
        }
    }
    return SHORT_CIRCUIT_NONE;
}
