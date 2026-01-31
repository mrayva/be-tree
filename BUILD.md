# Building be-tree with CMake and vcpkg

This document describes how to build the be-tree library using the modern CMake build system with optional vcpkg dependency management.

## Table of Contents
- [Prerequisites](#prerequisites)
- [Quick Start](#quick-start)
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

### Optional (via vcpkg)
- fmt library (version 10.0.0+)
- Microsoft GSL (version 4.0.0+)
- Catch2 (version 3.0.0+) for future C++ unit tests

## Quick Start

### 1. Generate lex/yacc files (if not already generated)
```bash
# Use the existing Makefile to generate lexer/parser files
make src/lexer.c src/parser.c src/event_lexer.c src/event_parser.c
```

### 2. Build with CMake (C++ mode - default)
```bash
# Create build directory
mkdir build && cd build

# Configure with C++ compilation enabled (default)
cmake ..

# Build
cmake --build .

# The library will be at: build/lib/libbetree.a
# Test executables will be at: build/bin/testbenchmark and build/bin/testbenchmark_err
```

### 3. Build in C mode (legacy compatibility)
```bash
mkdir build-c && cd build-c

# Configure with C compilation
cmake .. -DBUILD_AS_CPP=OFF

# Build
cmake --build .
```

## Building with vcpkg

vcpkg is a cross-platform package manager that simplifies dependency management.

The dependencies declared in `vcpkg.json` (fmt, ms-gsl, catch2) are currently not used by the C codebase. They are reserved for future C++ modules that will leverage modern C++ libraries. When converting modules to C++, these libraries will be available for use.

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
- fmt (formatted output library)
- ms-gsl (Microsoft's Guidelines Support Library)
- catch2 (testing framework, dev dependency)

## Building without vcpkg

If you don't want to use vcpkg, you can install dependencies system-wide or skip optional dependencies:

### Ubuntu/Debian
```bash
sudo apt-get install libgsl-dev libfmt-dev

# Generate lex/yacc files
make src/lexer.c src/parser.c src/event_lexer.c src/event_parser.c

# Build
mkdir build && cd build
cmake ..
cmake --build .
```

### macOS (with Homebrew)
```bash
brew install gsl fmt

# Generate lex/yacc files
make src/lexer.c src/parser.c src/event_lexer.c src/event_parser.c

# Build
mkdir build && cd build
cmake ..
cmake --build .
```

## Build Options

### BUILD_AS_CPP (default: ON)
Controls whether to compile library sources as C++ or C.

```bash
# Build as C++ (default)
cmake .. -DBUILD_AS_CPP=ON

# Build as C (legacy mode)
cmake .. -DBUILD_AS_CPP=OFF
```

When `BUILD_AS_CPP=ON`:
- C++ modules and compatibility shim are compiled as C++20
- Existing C sources remain compiled as C11 to avoid keyword conflicts
- Compatibility shim (`compat_alloc.cpp`) provides C++-friendly allocation wrappers
- Converted modules can use modern C++ features
- Generated lex/yacc files remain compiled as C for compatibility

When `BUILD_AS_CPP=OFF`:
- All sources are compiled as standard C11
- No C++ dependencies are required

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

### Test Benchmark Executables
After building, you can run the benchmark test executables:

```bash
# From build directory
./bin/testbenchmark
./bin/testbenchmark_err
```

Note: These executables require:
- GSL library to be installed
- Test data files in the `data/` directory

### Legacy Test Suite
You can still use the original Makefile test targets:

```bash
# From repository root
make test
make valgrind
```

## Legacy Makefile

The original Makefile is still present and fully functional. You can continue to use it for:
- Generating lex/yacc files
- Building the library with traditional Make
- Running the existing test suite

```bash
# Build with Makefile
make

# Run tests
make test

# Run valgrind checks
make valgrind

# Build test benchmarks
make build-test-benchmark
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

# 4. Build with CMake in C++ mode
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)

# 5. Run tests (if test data is available)
./bin/testbenchmark
```

## Migration Notes

This CMake build system is designed for incremental migration to modern C++:

1. **Compatibility**: Both C and C++ compilation modes are supported
2. **Gradual Conversion**: Individual modules can be converted to C++ incrementally
3. **Legacy Support**: Original Makefile remains fully functional
4. **Modern Dependencies**: vcpkg integration allows easy use of modern C++ libraries

### Current Status

✅ **Migration Complete**: All 22 core C modules have been successfully converted to C++20!

- **Infrastructure**: CMake build system, vcpkg manifest, C++ compatibility shim
- **C++20 Features**: nullptr, std::fprintf, std::abort, auto, static_cast, smart pointers
- **ABI Compatibility**: All public APIs maintain C linkage via extern "C" wrappers
- **Remaining C Code**: Only allocation layer (alloc.c) and generated parser files remain as C

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
