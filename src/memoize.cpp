#include "memoize.hpp"

// C API implementations - delegate to C++ namespace functions
extern "C" {

void set_bit(std::uint64_t A[], std::uint64_t k) {
    memoize_ops::set_bit(A, k);
}

void clear_bit(std::uint64_t A[], std::uint64_t k) {
    memoize_ops::clear_bit(A, k);
}

bool test_bit(const std::uint64_t A[], std::uint64_t k) {
    return memoize_ops::test_bit(A, k);
}

} // extern "C"
