#pragma once

// When compiling as C++, use the modern C++ header
#ifdef __cplusplus
#include "ast_compare.hpp"
#else

int expr_cmp(const void* p1, const void* p2);

#endif // __cplusplus

