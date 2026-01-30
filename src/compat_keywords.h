#pragma once

// C++ keyword compatibility for C code
// This header provides workarounds for C identifiers that are C++ keywords

#ifdef __cplusplus
// Rename C identifiers that conflict with C++ keywords
#define namespace namespace_  // Avoid C++ keyword 'namespace'
#endif
