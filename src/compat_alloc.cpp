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

// C++ implementation of brealloc using operator new/delete
void* brealloc_cpp(void* ptr, size_t size) {
    if (ptr == nullptr) {
        return bmalloc_cpp(size);
    }
    if (size == 0) {
        bfree_cpp(ptr);
        return nullptr;
    }
    
    // Note: This is a simple implementation that allocates new memory,
    // copies the old data, and frees the old memory. A production
    // implementation might want to track allocation sizes for proper realloc.
    // For now, we use standard realloc as a fallback since we don't track sizes.
    void* new_ptr = std::realloc(ptr, size);
    return new_ptr;
}

// C++ implementation of bfree using operator delete
void bfree_cpp(void* ptr) {
    if (ptr != nullptr) {
        ::operator delete(ptr);
    }
}

// C++ implementation of bstrdup
char* bstrdup_cpp(const char* s1) {
    if (s1 == nullptr) {
        return nullptr;
    }
    
    size_t size = std::strlen(s1) + 1;
    char* str = static_cast<char*>(bmalloc_cpp(size));
    if (str != nullptr) {
        std::memcpy(str, s1, size);
    }
    return str;
}

} // extern "C"
