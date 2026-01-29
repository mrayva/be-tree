#pragma once

// Compatibility allocation shim for C++ compilation
// This header provides C-compatible allocation functions that wrap C++ memory management
// Active only when compiling as C++ (BUILD_AS_CPP=ON)

#ifdef __cplusplus

#include <cstddef>
#include <cstdlib>

extern "C" {

// Basic allocation functions matching the C API - return void* to match C semantics
void* bmalloc_impl(size_t size);
void* bcalloc_impl(size_t size);  // Zero-initialized allocation
void* brealloc_impl(void* ptr, size_t size);
void bfree_impl(void* ptr);

// Note: bstrdup, bvasprintf, and basprintf are provided by alloc.c

}  // extern "C"

// C++ wrappers that just call the implementation
// The compiler will require explicit casts at call sites, which is correct C++ behavior
#undef bmalloc
#undef bcalloc
#undef brealloc
#undef bfree
#define bmalloc bmalloc_impl
#define bcalloc bcalloc_impl
#define brealloc brealloc_impl
#define bfree bfree_impl

#endif  // __cplusplus
