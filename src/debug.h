#pragma once

struct cnode;
struct betree;

// When compiling as C++, use the modern C++ header for function declarations
#ifdef __cplusplus
#include "debug.hpp"
#endif

void write_dot_file(const struct betree* tree);
void write_dot_to_file(const struct betree* tree, const char* fname);
