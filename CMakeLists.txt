cmake_minimum_required(VERSION 3.20)

# Project declaration
project(be-tree
    VERSION 0.1.0
    LANGUAGES C CXX
    DESCRIPTION "Binary Expression Tree library with incremental C++20/23 port support"
)

# vcpkg integration guidance
# To use vcpkg, set CMAKE_TOOLCHAIN_FILE to point to vcpkg's toolchain file:
# cmake -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake ..
# Or set the environment variable VCPKG_ROOT and it will be auto-detected below
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "Vcpkg toolchain file")
endif()

# Build options
# NOTE: BUILD_AS_CPP enables C++ support by compiling the compatibility shim and
# converted modules as C++, while keeping existing C sources compiled as C to avoid
# keyword conflicts (e.g., 'namespace' used as a field name in value.h).
# This allows gradual, incremental migration where individual modules are converted
# to C++ one at a time without breaking existing C code.
option(BUILD_AS_CPP "Enable C++ support and link converted C++ modules" ON)
option(BUILD_EXAMPLES "Build usage examples" OFF)

# Set C and C++ standards
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for tooling
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Build output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Find dependencies (only when using vcpkg or if installed system-wide)
# These are optional for now since we're maintaining compatibility
find_package(fmt CONFIG QUIET)
find_package(Microsoft.GSL CONFIG QUIET)

# Source directory
add_subdirectory(src)

# Examples directory (optional)
add_subdirectory(examples)

# Test executables (mimicking Makefile's testbenchmark targets)
# These require GSL library
find_package(GSL QUIET)

if(GSL_FOUND)
    # testbenchmark - real_tests.c
    add_executable(testbenchmark tests/real_tests.c)
    target_link_libraries(testbenchmark PRIVATE betree GSL::gsl GSL::gslcblas m)
    target_include_directories(testbenchmark PRIVATE ${CMAKE_SOURCE_DIR}/src)
    # Test files remain as C - they test the C API exposed via extern "C"

    # testbenchmark_err - real_tests_err.c
    add_executable(testbenchmark_err tests/real_tests_err.c)
    target_link_libraries(testbenchmark_err PRIVATE betree GSL::gsl GSL::gslcblas m)
    target_include_directories(testbenchmark_err PRIVATE ${CMAKE_SOURCE_DIR}/src)
    # Test files remain as C - they test the C API exposed via extern "C"
else()
    message(STATUS "GSL not found - testbenchmark executables will not be built")
    message(STATUS "Install GSL (libgsl-dev on Debian/Ubuntu) or use vcpkg to build tests")
endif()

# Custom targets for convenience
add_custom_target(build-all
    DEPENDS betree
    COMMENT "Building all targets"
)

# Print configuration summary
message(STATUS "")
message(STATUS "be-tree configuration:")
message(STATUS "  BUILD_AS_CPP: ${BUILD_AS_CPP}")
message(STATUS "  BUILD_EXAMPLES: ${BUILD_EXAMPLES}")
message(STATUS "  C++ Standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "  C Standard: C${CMAKE_C_STANDARD}")
if(fmt_FOUND)
    message(STATUS "  fmt library: Found")
endif()
if(Microsoft.GSL_FOUND)
    message(STATUS "  ms-gsl library: Found")
endif()
if(GSL_FOUND)
    message(STATUS "  GSL library: Found")
endif()
message(STATUS "")

# Installation rules
install(FILES include/betree_cpp.hpp
    DESTINATION include
    COMPONENT development
)
