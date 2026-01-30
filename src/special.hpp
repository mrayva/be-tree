#pragma once

#include <cstdint>
#include <cstdlib>

#include "tree.h"

// C API compatibility
#ifdef __cplusplus
extern "C" {
#endif

bool within_frequency_caps(const struct betree_frequency_caps* caps,
    enum frequency_type_e type,
    std::uint32_t id,
    const struct string_value ns,
    std::uint32_t value,
    std::size_t length,
    std::int64_t now);

bool within_frequency_caps_counting(const struct betree_frequency_caps* caps,
    enum frequency_type_e type,
    std::uint32_t id,
    const struct string_value ns,
    std::uint32_t value,
    std::size_t length,
    std::int64_t now,
    int* ops_count);

bool segment_within(
    std::int64_t segment_id,
    std::int32_t after_seconds,
    const struct betree_segments* segments,
    std::int64_t now);

bool segment_within_counting(
    std::int64_t segment_id,
    std::int32_t after_seconds,
    const struct betree_segments* segments,
    std::int64_t now,
    int* ops_count);

bool segment_before(
    std::int64_t segment_id,
    std::int32_t before_seconds,
    const struct betree_segments* segments,
    std::int64_t now);

bool segment_before_counting(
    std::int64_t segment_id,
    std::int32_t before_seconds,
    const struct betree_segments* segments,
    std::int64_t now,
    int* ops_count);

bool geo_within_radius(double lat1, double lon1, double lat2, double lon2, double distance);
bool contains(const char* value, const char* pattern);
bool starts_with(const char* value, const char* pattern);
bool ends_with(const char* value, const char* pattern);

#ifdef __cplusplus
}
#endif
