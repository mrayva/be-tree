#pragma once

#include <cstdint>

// Type definitions
using betree_pred_t = std::uint64_t;
constexpr betree_pred_t INVALID_PRED = UINT64_MAX;

// Memoization bitset structure
struct memoize {
    std::uint64_t* pass;
    std::uint64_t* fail;
};

// Modern C++ inline implementations with constexpr where possible
namespace betree {
    // Set bit k in bitset A
    inline void set_bit(std::uint64_t A[], std::uint64_t k) noexcept {
        A[k / 64ULL] |= (1ULL << (k % 64ULL));
    }

    // Clear bit k in bitset A
    inline void clear_bit(std::uint64_t A[], std::uint64_t k) noexcept {
        A[k / 64ULL] &= (~(1ULL << (k % 64ULL)));
    }

    // Test if bit k is set in bitset A
    [[nodiscard]] inline bool test_bit(const std::uint64_t A[], std::uint64_t k) noexcept {
        return ((A[k / 64ULL] & (1ULL << (k % 64ULL))) != 0ULL);
    }
} // namespace betree

// C API compatibility - extern "C" wrappers for existing C code
#ifdef __cplusplus
extern "C" {
#endif

void set_bit(std::uint64_t A[], std::uint64_t k);
void clear_bit(std::uint64_t A[], std::uint64_t k);
bool test_bit(const std::uint64_t A[], std::uint64_t k);

#ifdef __cplusplus
}
#endif
