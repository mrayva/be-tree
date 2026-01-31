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
gcc -o basic_usage basic_usage.c -I../src -L../build/lib -lbetree -lm
```

**Run:**
```bash
LD_LIBRARY_PATH=../build/lib ./basic_usage
```

### 2. `cpp_usage.cpp` - Modern C++ Example

A modern C++20 example featuring:
- RAII wrapper class around the C API
- Smart pointers for automatic memory management
- Method chaining for fluent API
- Exception-safe resource management
- STL containers for results
- Zero manual cleanup required

**Compile:**
```bash
g++ -std=c++20 -o cpp_usage cpp_usage.cpp -I../src -L../build/lib -lbetree -lm
```

**Run:**
```bash
LD_LIBRARY_PATH=../build/lib ./cpp_usage
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
./examples/basic_usage
./examples/cpp_usage
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

Events are JSON objects with key-value pairs matching the defined schema:

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
```

### C++ Wrapper (shown in cpp_usage.cpp)

The example includes a simple C++ wrapper demonstrating:
- RAII for automatic resource management
- Method chaining for fluent API
- Modern C++ containers for results
- Exception safety

This wrapper can be extended or used as a starting point for your own C++ interface.

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
