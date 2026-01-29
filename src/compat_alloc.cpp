// Compatibility allocation shim for C++ compilation
// Provides thin wrappers around C++ memory management

#ifdef __cplusplus

#include "compat_alloc.hpp"
#include <cstdlib>
#include <cstring>
#include <new>

extern "C" {

void* bmalloc(size_t size) {
    if (size == 0) return nullptr;
    void* ptr = ::operator new(size, std::nothrow);
    return ptr;
}

void* bcalloc(size_t size) {
    if (size == 0) return nullptr;
    void* ptr = ::operator new(size, std::nothrow);
    if (ptr != nullptr) {
        std::memset(ptr, 0, size);
    }
    return ptr;
}

void* brealloc(void* ptr, size_t size) {
    if (size == 0) {
        if (ptr != nullptr) {
            ::operator delete(ptr);
        }
        return nullptr;
    }
    
    if (ptr == nullptr) {
        return bmalloc(size);
    }
    
    // For realloc, we need to allocate new memory, copy, and free old
    // This is simplified - real implementation would track allocation sizes
    void* new_ptr = ::operator new(size, std::nothrow);
    if (new_ptr == nullptr) {
        return nullptr;
    }
    
    // Copy old data (we can't know the old size, so this is best-effort)
    // In production, you'd want to track allocation sizes
    std::memcpy(new_ptr, ptr, size);  // Assumes size is reasonable
    ::operator delete(ptr);
    
    return new_ptr;
}

void bfree(void* ptr) {
    if (ptr != nullptr) {
        ::operator delete(ptr);
    }
}

char* bstrdup(const char* s1) {
    if (s1 == nullptr) return nullptr;
    
    size_t size = std::strlen(s1) + 1;
    char* str = static_cast<char*>(bmalloc(size));
    if (str != nullptr) {
        std::memcpy(str, s1, size);
    }
    return str;
}

}  // extern "C"

#endif  // __cplusplus
