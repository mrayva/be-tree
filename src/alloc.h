#pragma once

#ifdef __cplusplus
// Still need the string allocation functions from alloc.c
extern "C" {
#endif

#ifdef NIF
#include <erl_nif.h>
#ifndef __cplusplus
#define bmalloc enif_alloc
#define bcalloc enif_calloc
#define brealloc enif_realloc
#define bfree enif_free
#endif
void* enif_calloc(size_t size);
#else
#ifndef __cplusplus
#define bmalloc malloc
#define bcalloc(x) calloc(x, 1)
#define brealloc realloc
#define bfree free
#endif
#endif

#include <stdarg.h>

char* bstrdup(const char *s1);
int bvasprintf(char **buf, const char *format, va_list va);
int basprintf(char **buf, const char *format, ...);

// Include the C++ allocation bridge when compiling as C++
// This must come after the macro definitions above
#ifdef __cplusplus
}  // extern "C"
#include "compat_alloc.hpp"
#endif
