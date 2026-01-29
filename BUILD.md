# Build Instructions

This document describes how to build the be-tree project using CMake and vcpkg.

## Prerequisites

- CMake 3.20 or higher
- A C++20-capable compiler (GCC 10+, Clang 10+, MSVC 2019+)
- vcpkg (optional, but recommended for dependency management)
- GNU Scientific Library (GSL) for test executables

## Building with CMake and vcpkg

### 1. Install vcpkg

If you don't have vcpkg installed:

```bash
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh  # On Windows: .\bootstrap-vcpkg.bat
```

### 2. Set up environment

Export the vcpkg toolchain file path:

```bash
export CMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
```

Or you can pass it directly to CMake (see below).

### 3. Build the project

#### Building as C++ (default)

```bash
# Create build directory
mkdir build
cd build

# Configure with vcpkg toolchain
cmake .. -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake

# Build
cmake --build .

# Executables will be in build/bin/
# Library will be in build/lib/
```

#### Building as C (legacy mode)

To build with C compilation (preserving original behavior):

```bash
mkdir build
cd build

# Configure with BUILD_AS_CPP=OFF
cmake .. -DBUILD_AS_CPP=OFF

# Build
cmake --build .
```

### 4. Running tests

After building:

```bash
# Run testbenchmark directly
./bin/testbenchmark

# Or use the custom target
cmake --build . --target run-tests
```

## Building with the Original Makefile

The original Makefile is preserved for compatibility:

```bash
# Build library
make

# Build and run tests
make test

# Build testbenchmark executables
make build-test-benchmark

# Clean
make clean
```

## CMake Build Options

- `BUILD_AS_CPP` (default: ON) - Compile library sources as C++
- `NIF` (default: OFF) - Build with Erlang NIF support

Example with custom options:

```bash
cmake .. -DBUILD_AS_CPP=ON -DNIF=OFF
```

## Dependencies

When building as C++, the following dependencies are managed through vcpkg:

- **fmt** - Modern formatting library
- **ms-gsl** - Microsoft's Guidelines Support Library

These are automatically installed by vcpkg when you configure the project.

## Troubleshooting

### Missing generated files (lexer.c, parser.c, etc.)

If you get errors about missing `lexer.c`, `parser.c`, `event_lexer.c`, or `event_parser.c`, you need to generate them first using the Makefile:

```bash
# Generate lex/yacc files
make

# Then you can use CMake
mkdir build && cd build
cmake ..
cmake --build .
```

### GSL not found

If GSL is not found, testbenchmark executables may fail to link. Install GSL:

```bash
# Ubuntu/Debian
sudo apt-get install libgsl-dev

# macOS
brew install gsl

# Fedora/RHEL
sudo dnf install gsl-devel
```

### vcpkg dependencies not found

Make sure you're passing the correct toolchain file to CMake:

```bash
cmake .. -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
```

## Migration Notes

This CMake build system is part of an incremental migration to modern C++. Currently:

- The library can be built as either C or C++ (controlled by `BUILD_AS_CPP`)
- A compatibility shim (`compat_alloc.cpp`) provides C-compatible allocation functions when building as C++
- Generated files (lexer, parser) are always compiled as C to avoid issues
- The `debug` module has been converted to C++ as an example of idiomatic conversion

The original Makefile remains unchanged and fully functional for those who prefer the traditional build system.
