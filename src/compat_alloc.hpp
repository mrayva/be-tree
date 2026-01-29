#pragma once

// Compatibility allocation shim for C++ compilation
// This header provides C-compatible allocation functions that wrap C++ memory management
// Active only when compiling as C++ (BUILD_AS_CPP=ON)

#ifdef __cplusplus

#include <cstddef>
#include <cstdlib>

extern "C" {

// Basic allocation functions matching the C API
void* bmalloc(size_t size);
void* bcalloc(size_t size);  // Zero-initialized allocation
void* brealloc(void* ptr, size_t size);
void bfree(void* ptr);

// String duplication
char* bstrdup(const char* s1);

// Formatted string allocation (already defined in alloc.h, but provide declarations)
int bvasprintf(char** buf, const char* format, va_list va);
int basprintf(char** buf, const char* format, ...);

}  // extern "C"

#endif  // __cplusplus
