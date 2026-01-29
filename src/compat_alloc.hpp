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
#ifndef NIF
#undef bmalloc
#undef bcalloc
#undef brealloc
#undef bfree

#define bmalloc bmalloc_cpp
#define bcalloc bcalloc_cpp
#define brealloc brealloc_cpp
#define bfree bfree_cpp
#endif

#endif // __cplusplus
