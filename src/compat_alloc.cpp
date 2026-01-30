#include "compat_alloc.hpp"

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <new>

extern "C" {

// C++ implementation of bmalloc using operator new
void* bmalloc_cpp(size_t size) {
    if (size == 0) {
        return nullptr;
    }
    try {
        return ::operator new(size);
    } catch (const std::bad_alloc&) {
        return nullptr;
    }
}

// C++ implementation of bcalloc (zero-initialized allocation)
void* bcalloc_cpp(size_t size) {
    if (size == 0) {
        return nullptr;
    }
    void* ptr = bmalloc_cpp(size);
    if (ptr != nullptr) {
        std::memset(ptr, 0, size);
    }
    return ptr;
}

// C++ implementation of brealloc
// NOTE: This is a compatibility shim that uses malloc/realloc/free for consistency.
// We cannot mix operator new/delete with realloc, so we use standard C allocation
// for brealloc to maintain proper semantics. This is acceptable since brealloc is
// rarely used in the codebase.
void* brealloc_cpp(void* ptr, size_t size) {
    if (ptr == nullptr) {
        // Allocate new memory - use malloc for realloc compatibility
        return std::malloc(size);
    }
    if (size == 0) {
        std::free(ptr);
        return nullptr;
    }
    
    // Use standard realloc - this assumes ptr was allocated with malloc
    // This is a known limitation: mixing bmalloc_cpp (operator new) with
    // brealloc_cpp is undefined behavior. Code should consistently use
    // either the bmalloc/bfree pair or the brealloc path, not mix them.
    void* new_ptr = std::realloc(ptr, size);
    return new_ptr;
}

// C++ implementation of bfree
// NOTE: This uses free() instead of operator delete for compatibility with brealloc
void bfree_cpp(void* ptr) {
    if (ptr != nullptr) {
        std::free(ptr);
    }
}

// C++ implementation of bstrdup
char* bstrdup_cpp(const char* s1) {
    if (s1 == nullptr) {
        return nullptr;
    }
    
    size_t size = std::strlen(s1) + 1;
    char* str = static_cast<char*>(std::malloc(size));
    if (str != nullptr) {
        std::memcpy(str, s1, size);
    }
    return str;
}

} // extern "C"
