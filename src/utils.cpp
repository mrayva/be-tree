// C++ implementation of utility functions
// Demonstrates idiomatic C++ conversion while maintaining C compatibility

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>

#include "betree.h"
#include "utils.hpp"
#include "value.h"

extern "C" {

int64_t d64min(int64_t a, int64_t b) {
    return std::min(a, b);
}

int64_t d64max(int64_t a, int64_t b) {
    return std::max(a, b);
}

uint64_t u64max(uint64_t a, uint64_t b) {
    return std::max(a, b);
}

size_t smin(size_t a, size_t b) {
    return std::min(a, b);
}

size_t smax(size_t a, size_t b) {
    return std::max(a, b);
}

bool feq(double a, double b) {
    return std::fabs(a - b) < std::numeric_limits<double>::epsilon();
}

bool fne(double a, double b) {
    return !feq(a, b);
}

int icmpfunc(const void *a, const void *b) {
    const auto x = *static_cast<const int64_t*>(a);
    const auto y = *static_cast<const int64_t*>(b);
    
    if (x < y) return -1;
    if (x > y) return 1;
    return 0;
}

int scmpfunc(const void *a, const void *b) {
    const auto* x = static_cast<const struct string_value*>(a);
    const auto* y = static_cast<const struct string_value*>(b);
    
    if (x->str < y->str) return -1;
    if (x->str > y->str) return 1;
    return 0;
}

int iecmpfunc(const void *a, const void *b) {
    const auto* x = static_cast<const struct integer_enum_value*>(a);
    const auto* y = static_cast<const struct integer_enum_value*>(b);
    
    if (x->ienum < y->ienum) return -1;
    if (x->ienum > y->ienum) return 1;
    return 0;
}

}  // extern "C"
