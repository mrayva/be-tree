#pragma once

// C++ version of debug module
// This is an idiomatic C++ conversion demonstrating RAII and modern C++ practices

#ifdef __cplusplus

#include <string>
#include <memory>

// Forward declarations
struct betree;

extern "C" {
#endif

// Keep the same public API for C compatibility
void write_dot_file(const struct betree* tree);
void write_dot_to_file(const struct betree* tree, const char* fname);

#ifdef __cplusplus
} // extern "C"

// C++ only utilities can be added here in the future
namespace betree_debug {
    // Future: Add C++-specific debug utilities here
    // Example: std::string get_dot_string(const betree* tree);
}

#endif // __cplusplus
