#include "compat_alloc.hpp"

#include <cstddef>
#include <cstdlib>
#include <cstring>
extern "C" {

// The supported core still depends on C allocation semantics, including realloc.
// Keep the allocation bridge on the malloc/calloc/realloc/free family so allocations can
// safely move across the C/C++ boundary.
void* bmalloc_cpp(size_t size) {
    if (size == 0) {
        return nullptr;
    }
    return std::malloc(size);
}

void* bcalloc_cpp(size_t size) {
    if (size == 0) {
        return nullptr;
    }
    return std::calloc(1, size);
}

void* brealloc_cpp(void* ptr, size_t size) {
    if (size == 0) {
        std::free(ptr);
        return nullptr;
    }
    return std::realloc(ptr, size);
}

void bfree_cpp(void* ptr) {
    if (ptr != nullptr) {
        std::free(ptr);
    }
}

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
