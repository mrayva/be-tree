#pragma once

// C++ allocation bridge
// This header provides C++ wrappers for the allocation helpers exposed by the C API.
// It keeps allocation semantics consistent across the maintained C/C++ boundary.

#ifdef __cplusplus

#include <cstddef>
#include <cstdlib>
#include <cstring>

extern "C" {

// Allocation functions that match the C API.
// These are implemented in compat_alloc.cpp and keep C linkage across the boundary.

void* bmalloc_cpp(size_t size);
void* bcalloc_cpp(size_t size);
void* brealloc_cpp(void* ptr, size_t size);
void bfree_cpp(void* ptr);
char* bstrdup_cpp(const char* s1);

} // extern "C"

// When compiling as C++, redirect the allocation macros to the bridge functions.
// Use inline functions rather than macros for basic type safety.
#ifndef NIF

#undef bmalloc
#undef bcalloc
#undef brealloc
#undef bfree

inline void* bmalloc(size_t size) { return bmalloc_cpp(size); }
inline void* bcalloc(size_t size) { return bcalloc_cpp(size); }
inline void* brealloc(void* ptr, size_t size) { return brealloc_cpp(ptr, size); }
inline void bfree(void* ptr) { bfree_cpp(ptr); }

#ifdef bstrdup
#undef bstrdup
#endif
inline char* bstrdup(const char* s1) { return bstrdup_cpp(s1); }

#endif // NIF

#endif // __cplusplus
