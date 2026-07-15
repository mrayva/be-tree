# BE-Tree Usage Examples

This directory contains practical examples demonstrating how to use the BE-tree library in both C and C++.

## Examples

### 1. `basic_usage.c` - C API Example

A complete example showing the C API:
- Creating a BE-tree
- Defining variable domains (schema)
- Inserting subscription expressions
- Matching events against subscriptions
- Accessing performance statistics
- Proper resource cleanup

**Compile:**
```bash
gcc -o basic_usage basic_usage.c -I../src -L../build/cmake/lib -lbetree -lm
```

**Run:**
```bash
LD_LIBRARY_PATH=../build/cmake/lib ./basic_usage
```

### 2. `filtered_search.c` - Sorted ID Filter Example

Small C API example showing filtered search with `betree_search_ids(...)`:
- sorted ascending id filters
- filtered result sets
- minimal setup for the common “only check these ids” path

**Compile:**
```bash
gcc -o filtered_search filtered_search.c -I../src -L../build/cmake/lib -lbetree -lm
```

**Run:**
```bash
LD_LIBRARY_PATH=../build/cmake/lib ./filtered_search
```

### 3. `err_reason_example.c` - `_err` Reason Reporting Example

Focused C API example for reason reporting:
- `betree_err`
- `make_report_err(...)`
- non-match reason buckets
- reporting subscription ids grouped by last-failing reason

**Compile:**
```bash
gcc -o err_reason_example err_reason_example.c -I../src -L../build/cmake/lib -lbetree -lm
```

**Run:**
```bash
LD_LIBRARY_PATH=../build/cmake/lib ./err_reason_example
```

### 4. `cpp_usage.cpp` - Modern C++ Example (Inline Wrapper)

A modern C++20 example featuring an inline wrapper class:
- RAII wrapper class around the C API
- Smart pointers for automatic memory management
- Method chaining for fluent API
- Exception-safe resource management
- STL containers for results
- Zero manual cleanup required

**Compile:**
```bash
g++ -std=c++20 -o cpp_usage cpp_usage.cpp -I../src -L../build/cmake/lib -lbetree -lm
```

**Run:**
```bash
LD_LIBRARY_PATH=../build/cmake/lib ./cpp_usage
```

### 5. `cpp_modern.cpp` - Production C++ API Example

Demonstrates the production-quality `betree_cpp.hpp` wrapper API:
- Uses the official `be::Tree` class from `include/betree_cpp.hpp`
- Full RAII-based resource management
- Exception-safe with custom `be::BetreeException`
- Method chaining for all schema operations
- `std::string_view` convenience at the wrapper API boundary
- `be::SearchResult` with STL containers
- JSON and structured-event search coverage

**Compile:**
```bash
g++ -std=c++20 -o cpp_modern cpp_modern.cpp -I../src -I../include -L../build/cmake/lib -lbetree -lm
```

**Run:**
```bash
LD_LIBRARY_PATH=../build/cmake/lib ./cpp_modern
```

### 6. `cpp_special_events.cpp` - Structured Special-Domain Example

Focused example for the wrapper's structured-event API:
- `Tree::make_event()`
- typed `Event::set_*` methods
- integer-enum event values
- empty-list polymorphism
- `segments` and `frequency_caps`
- filtered structured-event search
- negative special-domain mismatch behavior

**Compile:**
```bash
g++ -std=c++20 -o cpp_special_events cpp_special_events.cpp -I../src -I../include -L../build/cmake/lib -lbetree -lm
```

**Run:**
```bash
LD_LIBRARY_PATH=../build/cmake/lib ./cpp_special_events
```

## Building with CMake

The examples can be built using CMake:

```bash
cd build
cmake .. -DBUILD_EXAMPLES=ON
make examples
```

Then run:
```bash
./cmake/examples/basic_usage    # C API
./cmake/examples/filtered_search  # C API sorted-id filtering
./cmake/examples/err_reason_example  # C API _err reason reporting
./cmake/examples/cpp_usage      # C++ inline wrapper
./cmake/examples/cpp_modern     # C++ production API
./cmake/examples/cpp_special_events  # C++ structured special domains
```

## What These Examples Demonstrate

### Core Concepts

1. **Schema Definition**: Variables must be declared with types and bounds before use
2. **Expression Language**: Subscriptions use a SQL-like boolean expression syntax
3. **Event Matching**: Events are JSON objects matched against subscription expressions
4. **Performance**: The library provides statistics on evaluation efficiency

### Expression Syntax

The library supports a rich boolean expression language:

- **Comparisons**: `=`, `!=`, `<`, `<=`, `>`, `>=`
- **Logical operators**: `and`, `or`, `not`
- **Grouping**: `(...)`
- **String literals**: `"value"`
- **Numeric literals**: `42`, `3.14`
- **Boolean literals**: `true`, `false`

**Examples:**
```
age >= 18 and age < 65
country = "USA" or country = "Canada"
premium = true and (score >= 80.0 or purchases > 10)
```

### Event Format

Events can be supplied either as JSON objects or through the structured wrapper event API.

JSON example:

```json
{
  "age": 25,
  "country": "USA",
  "premium": true,
  "score": 85.5
}
```

## API Overview

### C API (betree.h)

```c
// Initialization
struct betree* betree_make();
void betree_free(struct betree* tree);

// Schema definition
void betree_add_integer_variable(struct betree* tree, const char* name,
                                  bool allow_undef, int64_t min, int64_t max);
void betree_add_string_variable(struct betree* tree, const char* name,
                                 bool allow_undef, size_t count);
void betree_add_boolean_variable(struct betree* tree, const char* name,
                                  bool allow_undef);
void betree_add_float_variable(struct betree* tree, const char* name,
                                bool allow_undef, double min, double max);

// Subscription management
bool betree_insert(struct betree* tree, betree_sub_t id, const char* expr);

// Event matching
struct report* make_report();
void free_report(struct report* report);
bool betree_search(const struct betree* tree, const char* event_json,
                   struct report* report);
bool betree_search_ids(const struct betree* tree, const char* event_json,
                       struct report* report, const uint64_t* ids, size_t sz);
```

### C `_err` API (betree_err.h)

```c
struct betree_err* betree_make_err();
bool betree_insert_err(struct betree_err* tree, betree_sub_t id, const char* expr);
bool betree_make_sub_ids(struct betree_err* tree);

struct report_err* make_report_err(const struct betree_err* tree);
void free_report_err(struct report_err* report);
bool betree_search_err(const struct betree_err* tree, const char* event_json,
                       struct report_err* report);
```

### C++ API (betree_cpp.hpp)

The library now includes a production-quality C++ wrapper in `include/betree_cpp.hpp`:

```cpp
#include <betree_cpp.hpp>

be::Tree tree;
tree.add_integer("age", false, 0, 150)
    .add_string("country", false, 100)
    .insert(1, "age >= 18 and country = \"USA\"");

auto results = tree.search(R"({"age": 25, "country": "USA"})");
for (auto sub_id : results.matched_subs) {
    std::cout << "Matched: " << sub_id << "\n";
}
// Automatic cleanup via RAII
```

Structured event example:

```cpp
auto event = tree.make_event();
event.set_integer(0, 25)
     .set_string(1, "USA");

auto results = tree.search(event);
bool any = tree.exists(event);
```

An event is bound to the tree that created it and can only be searched by that tree. The C++ wrapper keeps the underlying tree alive until the event is destroyed.

Features:
- **Namespace:** All types in `be::` namespace
- **RAII:** Automatic resource management with `be::Tree`
- **Exceptions:** `be::BetreeException` for error handling
- **Results:** `be::SearchResult` with `std::vector<uint64_t>`
- **Safe wrapper boundary:** Accepts `std::string_view` and materializes safe C strings when calling the C API
- **Method chaining:** Fluent API for schema definition
- **Structured events:** `Tree::make_event()` plus typed `Event::set_*` methods
- **Type safety:** Strong typing with C++20

The wrapper is header-only and works alongside the C API.

## Performance Notes

- The BE-tree uses **memoization** to cache sub-expression results
- **Short-circuiting** stops evaluation early when the result is determined
- Performance stats are available in the `report` structure:
  - `evaluated`: Number of expressions evaluated
  - `memoized`: Number of memoized results reused
  - `shorted`: Number of short-circuit optimizations

## Further Reading

- [BUILD.md](../BUILD.md) - Building and compilation instructions
- [src/betree.h](../src/betree.h) - Complete C API reference
- [tests/](../tests/) - Comprehensive test suite with more usage examples

## Common Pitfalls

1. **Schema must be defined first**: All variables must be declared before inserting subscriptions
2. **Variable bounds**: Integer and float variables require min/max bounds
3. **String count**: String variables need an estimated unique value count for optimization
4. **Memory management**: In C, always call `free_report()` and `betree_free()` to prevent leaks
5. **Event JSON format**: Events must be valid JSON with keys matching the schema

## Contributing Examples

Have a useful example? Consider contributing:
1. Keep examples focused and well-commented
2. Demonstrate one concept clearly
3. Include both C and C++ versions when applicable
4. Update this README with usage instructions
