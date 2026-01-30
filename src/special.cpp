#include "special.hpp"

#include <cmath>
#include <cstring>
#include <numbers>

#include "betree.h"
#include "error.h"
#include "utils.h"

extern "C" {

bool within_frequency_caps(const struct betree_frequency_caps* caps,
    enum frequency_type_e type,
    std::uint32_t id,
    const struct string_value ns,
    std::uint32_t value,
    std::size_t length,
    std::int64_t now)
{
    for (std::size_t i = 0; i < caps->size; i++) {
        const auto* content = caps->content[i];
        if (content->id == id &&
            content->ns.str == ns.str &&
            content->type == type) {
            if (length <= 0) {
                return value > content->value;
            }
            if (!content->timestamp_defined) {
                return true;
            }
            if ((now - (content->timestamp / 1000000)) > static_cast<std::int64_t>(length)) {
                return true;
            }
            if (value > content->value) {
                return true;
            }
            return false;
        }
    }
    return true;
}

bool within_frequency_caps_counting(const struct betree_frequency_caps* caps,
    enum frequency_type_e type,
    std::uint32_t id,
    const struct string_value ns,
    std::uint32_t value,
    std::size_t length,
    std::int64_t now,
    int* ops_count)
{
    for (std::size_t i = 0; i < caps->size; i++) {
        (*ops_count)++;
        const auto* content = caps->content[i];
        if (content->id == id &&
            content->ns.str == ns.str &&
            content->type == type) {
            if (length <= 0) {
                return value > content->value;
            }
            if (!content->timestamp_defined) {
                return true;
            }
            if ((now - (content->timestamp / 1000000)) > static_cast<std::int64_t>(length)) {
                return true;
            }
            if (value > content->value) {
                return true;
            }
            return false;
        }
    }
    return true;
}

bool segment_within(
    std::int64_t segment_id,
    std::int32_t after_seconds,
    const struct betree_segments* segments,
    std::int64_t now)
{
    for (std::size_t i = 0; i < segments->size; i++) {
        if (segments->content[i]->id < segment_id) {
            continue;
        }
        if (segments->content[i]->id == segment_id) {
            return (now - after_seconds) <= (segments->content[i]->timestamp / 1000000);
        }
        return false;
    }
    return false;
}

bool segment_within_counting(
    std::int64_t segment_id,
    std::int32_t after_seconds,
    const struct betree_segments* segments,
    std::int64_t now,
    int* ops_count)
{
    for (std::size_t i = 0; i < segments->size; i++) {
        (*ops_count)++;
        if (segments->content[i]->id < segment_id) {
            continue;
        }
        if (segments->content[i]->id == segment_id) {
            return (now - after_seconds) <= (segments->content[i]->timestamp / 1000000);
        }
        return false;
    }
    return false;
}

bool segment_before(
    std::int64_t segment_id,
    std::int32_t before_seconds,
    const struct betree_segments* segments,
    std::int64_t now)
{
    for (std::size_t i = 0; i < segments->size; i++) {
        if (segments->content[i]->id < segment_id) {
            continue;
        }
        if (segments->content[i]->id == segment_id) {
            return (now - before_seconds) > (segments->content[i]->timestamp / 1000000);
        }
        return false;
    }
    return false;
}

bool segment_before_counting(
    std::int64_t segment_id,
    std::int32_t before_seconds,
    const struct betree_segments* segments,
    std::int64_t now,
    int* ops_count)
{
    for (std::size_t i = 0; i < segments->size; i++) {
        (*ops_count)++;
        if (segments->content[i]->id < segment_id) {
            continue;
        }
        if (segments->content[i]->id == segment_id) {
            return (now - before_seconds) > (segments->content[i]->timestamp / 1000000);
        }
        return false;
    }
    return false;
}

// Earth radius in kilometers
constexpr double EARTH_RADIUS = 6372.8;
constexpr double TO_RAD = std::numbers::pi / 180.0;

bool geo_within_radius(double lat1, double lon1, double lat2, double lon2, double distance)
{
    double dx, dy, dz;
    lon1 -= lon2;
    lon1 *= TO_RAD;
    lat1 *= TO_RAD;
    lat2 *= TO_RAD;

    dz = std::sin(lat1) - std::sin(lat2);
    dx = std::cos(lon1) * std::cos(lat1) - std::cos(lat2);
    dy = std::sin(lon1) * std::cos(lat1);

    return (std::asin(std::sqrt(dx * dx + dy * dy + dz * dz) / 2.0) * 2.0 * EARTH_RADIUS) <= distance;
}

bool contains(const char* value, const char* pattern)
{
    return std::strstr(value, pattern) != nullptr;
}

bool starts_with(const char* value, const char* pattern)
{
    const std::size_t value_size = std::strlen(value);
    const std::size_t pattern_size = std::strlen(pattern);
    if (value_size < pattern_size) {
        return false;
    }

    return std::strncmp(value, pattern, pattern_size) == 0;
}

bool ends_with(const char* value, const char* pattern)
{
    const std::size_t value_size = std::strlen(value);
    const std::size_t pattern_size = std::strlen(pattern);
    if (value_size < pattern_size) {
        return false;
    }

    const std::size_t off = value_size - pattern_size;
    return std::strncmp(value + off, pattern, pattern_size) == 0;
}

} // extern "C"
