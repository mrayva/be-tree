# Building be-tree with CMake and vcpkg

This document describes how to build the be-tree library using the modern CMake build system with optional vcpkg dependency management.

## Table of Contents
- [Prerequisites](#prerequisites)
- [Quick Start](#quick-start)
- [Installing](#installing)
- [Building with vcpkg](#building-with-vcpkg)
- [Building without vcpkg](#building-without-vcpkg)
- [Build Options](#build-options)
- [Running Tests](#running-tests)
- [Legacy Makefile](#legacy-makefile)

## Prerequisites

### Required
- CMake 3.20 or higher
- C compiler (GCC 7+, Clang 10+, or MSVC 2019+)
- C++ compiler with C++20 support (GCC 10+, Clang 11+, or MSVC 2019+)
- Flex (for lexer generation, if not already generated)
- Bison (for parser generation, if not already generated)
- GNU Scientific Library (GSL) for test executables

## Quick Start

### 1. Generate lex/yacc files (if not already generated)
```bash
# Use the existing Makefile to generate lexer/parser files
make src/lexer.c src/parser.c src/event_lexer.c src/event_parser.c
```

### 2. Build with CMake (default)
```bash
# Create build directory
mkdir build && cd build

# Configure with the default supported core
cmake ..

# Build
cmake --build .

# The library will be at: build/cmake/lib/libbetree.a
# Library output: build/cmake/lib/libbetree.a
# Test executables: build/cmake/bin/
```

### 3. Build with compatibility aliases
```bash
mkdir build-c && cd build-c

# Configure with the canonical option
cmake .. -DBUILD_CPP_CORE=OFF

# Build
cmake --build .
```

## Installing

Install the static library, C and C++ headers, and CMake package metadata into a prefix:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j4
cmake --install build --prefix /path/to/prefix
```

Downstream CMake projects can then consume either API through the exported target:

```cmake
find_package(betree CONFIG REQUIRED)
target_link_libraries(your_target PRIVATE betree::betree)
```

Set `CMAKE_PREFIX_PATH=/path/to/prefix` when the prefix is outside CMake's default search paths. Because the library implementation uses C++20, downstream projects must enable both C and C++ even when calling only the C API.

## Building with vcpkg

vcpkg is a cross-platform package manager that simplifies dependency management.

The manifest installs GSL, which is used by the benchmark-style test executables.

### 1. Install vcpkg
```bash
# Clone vcpkg repository
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg

# Bootstrap vcpkg
./bootstrap-vcpkg.sh  # On Linux/macOS
# or
./bootstrap-vcpkg.bat  # On Windows

# Set environment variable (optional, for convenience)
export VCPKG_ROOT=/path/to/vcpkg
```

### 2. Build be-tree with vcpkg dependencies
```bash
cd /path/to/be-tree

# Generate lex/yacc files first
make src/lexer.c src/parser.c src/event_lexer.c src/event_parser.c

# Create build directory
mkdir build-vcpkg && cd build-vcpkg

# Configure with vcpkg toolchain
cmake .. -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake

# Build
cmake --build .
```

vcpkg will automatically install the dependencies listed in `vcpkg.json`:
- gsl (GNU Scientific Library, used by benchmark-style tests)

## Building without vcpkg

If you don't want to use vcpkg, you can install dependencies system-wide or skip optional dependencies:

### Ubuntu/Debian
```bash
sudo apt-get install libgsl-dev

# Generate lex/yacc files
make src/lexer.c src/parser.c src/event_lexer.c src/event_parser.c

# Build
mkdir build && cd build
cmake ..
cmake --build .
```

### macOS (with Homebrew)
```bash
brew install gsl

# Generate lex/yacc files
make src/lexer.c src/parser.c src/event_lexer.c src/event_parser.c

# Build
mkdir build && cd build
cmake ..
cmake --build .
```

## Build Options

### BUILD_CPP_CORE (default: ON)
Controls whether to use the supported C++ core implementation. This is the
canonical option name.

```bash
# Build with the supported C++ core (default)
cmake .. -DBUILD_CPP_CORE=ON

# Compatibility names still accepted for older scripts
cmake .. -DBUILD_CONVERTED_CPP_CORE=ON
cmake .. -DBUILD_AS_CPP=ON
```

```bash
# Disable the canonical option
cmake .. -DBUILD_CPP_CORE=OFF

# Deprecated aliases, still accepted
cmake .. -DBUILD_CONVERTED_CPP_CORE=OFF
cmake .. -DBUILD_AS_CPP=OFF
```

When `BUILD_CPP_CORE=ON`:
- C++ modules and the allocation bridge are compiled as C++20
- Existing C sources remain compiled as C11 to avoid keyword conflicts
- Allocation bridge (`compat_alloc.cpp`) keeps allocation semantics consistent across the C/C++ boundary
- Core modules can use modern C++ features
- Generated lex/yacc files remain compiled as C for compatibility

When `BUILD_CPP_CORE=OFF`:
- The public C API remains available
- The build still links the supported C++ core, because the old pure-C core is no longer present in this checkout
- Remaining support files and generated lex/yacc sources are still compiled as C11

### CMAKE_BUILD_TYPE
Controls the optimization level and debug symbols.

```bash
# Debug build (default)
cmake .. -DCMAKE_BUILD_TYPE=Debug

# Release build with optimizations
cmake .. -DCMAKE_BUILD_TYPE=Release

# Release with debug info
cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo
```

## Running Tests

### CTest Suite
The CMake build now registers the existing C API tests and the C++ wrapper smoke test with CTest.

```bash
# From repository root
ctest --test-dir build --output-on-failure
```

This runs:
- the C++ wrapper smoke test
- the existing MinUnit-style unit tests
- the benchmark-style `real_tests` and `real_tests_err` executables with a reduced search count

The wrapper smoke test now covers both wrapper surfaces:
- JSON-based `Tree::search(...)`
- structured-event usage through `Tree::make_event()`, typed `Event::set_*` methods, `Tree::search(event, ...)`, and `Tree::exists(event)`

Malformed JSON is recoverable: normal searches return `false`, wrapper searches return an empty result with zeroed statistics, and `_err` searches report `invalid_event`.

Structured events are schema-bound, and search rejects an event created by a different tree. The C++ wrapper keeps the creating tree alive until all of its events are destroyed.

The parser suite also locks down the empty-list contract used by the matcher:
- JSON `[]` is intentionally treated as an untyped empty list at parse time
- the parser stores it with canonical `BETREE_INTEGER_LIST` tagging while preserving zero-sized views for the other list-capable domains
- this behavior is covered by `event_parser_tests`

The filtered search APIs (`betree_search_ids`, `betree_search_with_event_ids`, and the `_err` variants) expect the `ids` array to be sorted in ascending order and treated as a set. The current implementation uses binary search over that filter, so unsorted or duplicate-heavy arrays are not a guaranteed contract.

CTest runs the tests from the repository root so relative paths like `data/...` and generated DOT outputs under `tests/` resolve correctly.

### Benchmark-Labeled Tests
The heavier benchmark-style tests are registered with the `benchmark` label:

```bash
# Run only the benchmark-style CTest entries
ctest --test-dir build -L benchmark --output-on-failure
```

These labeled tests currently map to:
- `real_tests`
- `real_tests_err`

They require:
- GSL to be installed
- the benchmark data files under `data/` to be present

If the benchmark data files are missing, those executables print a skip message and exit successfully.

### Benchmark Executables
You can still run the benchmark executables directly if needed:

```bash
# From repository root
./build/cmake/bin/testbenchmark 1
./build/cmake/bin/testbenchmark_err 1
```

The optional numeric argument controls the search-count loop. The default is `10`; the CTest registration uses `1` to keep runtime small.

### Legacy Makefile Tests
The old Make-based test workflow has been retired. Use CTest instead:

```bash
# From repository root
ctest --test-dir build --output-on-failure
```

## Legacy Makefile

The root `Makefile` is now a generation-only helper.
The old Make-based library and test build has been retired along with the removed mirrored legacy `.c` core files.

You can still use it for:
- regenerating tracked lexer/parser sources

```bash
# Regenerate tracked lexer/parser sources
make src/lexer.c src/parser.c src/event_lexer.c src/event_parser.c
```

## Troubleshooting

### Lex/Yacc Generated Files Not Found
If CMake complains about missing generated files:
```bash
# Generate them using the Makefile
make src/lexer.c src/parser.c src/event_lexer.c src/event_parser.c
```

### GSL Not Found
If GSL library is not found:
- Ubuntu/Debian: `sudo apt-get install libgsl-dev`
- macOS: `brew install gsl`
- Or use vcpkg: Files will be auto-installed

### vcpkg Dependencies Not Installing
Make sure vcpkg is properly bootstrapped and the toolchain file path is correct:
```bash
# Bootstrap vcpkg
cd /path/to/vcpkg
./bootstrap-vcpkg.sh

# Use absolute path to toolchain file
cmake .. -DCMAKE_TOOLCHAIN_FILE=/absolute/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
```

## Example: Complete Build from Scratch

```bash
# 1. Clone repository
git clone https://github.com/mrayva/be-tree.git
cd be-tree

# 2. Generate parser/lexer files
make src/lexer.c src/parser.c src/event_lexer.c src/event_parser.c

# 3. Install dependencies (Ubuntu example)
sudo apt-get install cmake libgsl-dev flex bison

# 4. Build with CMake using the supported core
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)

# 5. Run the full CTest suite
cd ..
ctest --test-dir build --output-on-failure

# 6. Optionally run only the benchmark-labeled tests
ctest --test-dir build -L benchmark --output-on-failure
```

## Migration Notes

This CMake build system uses a mixed-language layout built around the supported core:

1. **Converted Core**: The main library core is built from the converted C++ sources
2. **C API Compatibility**: The public C API remains available
3. **Legacy Support**: The root Makefile remains only for flex/bison regeneration
4. **Optional Dependencies**: vcpkg integration can provide optional libraries when needed

### Current Status

✅ **Current Core Status**: The main core path is the converted C++20 implementation.

- **Infrastructure**: CMake build system, vcpkg manifest, C++ allocation bridge
- **C++20 Features**: nullptr, std::fprintf, std::abort, auto, static_cast, smart pointers
- **ABI Compatibility**: All public APIs maintain C linkage via extern "C" wrappers
- **Remaining C Code**: Only allocation layer (`alloc.c`), `special.c`, and generated parser files remain as C

### Converted Modules

All core modules are now C++20:
- Utilities: memoize, dyn_arr, utils, hashmap, special, var, jsw_rbtree
- AST: ast, ast_compare, ast_err
- Configuration: config, value
- Trees: tree, tree_err, betree, betree_err
- Helpers: helper, helper_err, printer, debug, debug_err
- Map: map

### Usage Examples

The `examples/` directory contains practical demonstrations:

1. **basic_usage.c** - C API usage showing:
   - Creating a BE-tree
   - Defining variable domains
   - Inserting subscriptions
   - Matching events
   - Performance statistics

2. **cpp_usage.cpp** - Modern C++ usage with:
   - RAII wrapper classes
   - Smart pointers for automatic memory management
   - Method chaining
   - STL containers for results

Build examples:
```bash
cmake .. -DBUILD_EXAMPLES=ON
make examples
./examples/basic_usage
./examples/cpp_usage
```

See `examples/README.md` for detailed API documentation and usage patterns.

## Further Information

- CMake documentation: https://cmake.org/documentation/
- vcpkg documentation: https://vcpkg.io/
- Project repository: https://github.com/mrayva/be-tree
