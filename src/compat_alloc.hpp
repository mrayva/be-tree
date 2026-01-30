#pragma once

// C++ Compatibility Allocation Shim
// This header provides C++-compatible wrapper functions for the allocation
// helpers used in the original C codebase. When BUILD_AS_CPP is enabled,
// these wrappers allow C code to be compiled as C++ with minimal changes.

#ifdef __cplusplus

#include <cstddef>
#include <cstdlib>
#include <cstring>

extern "C" {

// Allocation functions that match the original C API
// These are implemented in compat_alloc.cpp and provide C++ implementations
// using operator new/delete while maintaining C linkage for compatibility.

void* bmalloc_cpp(size_t size);
void* bcalloc_cpp(size_t size);
void* brealloc_cpp(void* ptr, size_t size);
void bfree_cpp(void* ptr);
char* bstrdup_cpp(const char* s1);

} // extern "C"

// When compiling as C++, redefine the allocation macros to use C++ implementations
// Note: We use inline functions instead of macros to provide better type safety
#ifndef NIF

#undef bmalloc
#undef bcalloc
#undef brealloc
#undef bfree

// Use inline functions to preserve type safety in C++
inline void* bmalloc(size_t size) { return bmalloc_cpp(size); }
inline void* bcalloc(size_t size) { return bcalloc_cpp(size); }
inline void* brealloc(void* ptr, size_t size) { return brealloc_cpp(ptr, size); }
inline void bfree(void* ptr) { bfree_cpp(ptr); }

// Override bstrdup as well for completeness
#ifdef bstrdup
#undef bstrdup
#endif
inline char* bstrdup(const char* s1) { return bstrdup_cpp(s1); }

#endif // NIF

#endif // __cplusplus
