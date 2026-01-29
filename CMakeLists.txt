cmake_minimum_required(VERSION 3.20)

# Detect vcpkg toolchain from environment if not explicitly provided
# To use vcpkg: export CMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
# or pass -DCMAKE_TOOLCHAIN_FILE=... to cmake

project(be-tree
    VERSION 0.1.0
    DESCRIPTION "Boolean Expression Tree - fast evaluation of large numbers of boolean expressions"
    LANGUAGES C CXX
)

# Options
option(BUILD_AS_CPP "Compile library sources as C++" ON)
option(NIF "Build with Erlang NIF support" OFF)

# Set output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Find dependencies (only when building as C++)
if(BUILD_AS_CPP)
    find_package(fmt CONFIG)
    find_package(Microsoft.GSL CONFIG)
endif()

# Add subdirectories
add_subdirectory(src)

# Test executables (replicating Makefile's testbenchmark and testbenchmark_err)
add_executable(testbenchmark tests/real_tests.c)
target_include_directories(testbenchmark PRIVATE src)
target_link_libraries(testbenchmark PRIVATE betree m)
if(BUILD_AS_CPP)
    set_source_files_properties(tests/real_tests.c PROPERTIES LANGUAGE CXX)
endif()

add_executable(testbenchmark_err tests/real_tests_err.c)
target_include_directories(testbenchmark_err PRIVATE src)
target_link_libraries(testbenchmark_err PRIVATE betree m)
if(BUILD_AS_CPP)
    set_source_files_properties(tests/real_tests_err.c PROPERTIES LANGUAGE CXX)
endif()

# Try to find GSL for tests
find_library(GSL_LIB gsl)
find_library(GSLCBLAS_LIB gslcblas)
if(GSL_LIB AND GSLCBLAS_LIB)
    target_link_libraries(testbenchmark PRIVATE ${GSL_LIB} ${GSLCBLAS_LIB})
    target_link_libraries(testbenchmark_err PRIVATE ${GSL_LIB} ${GSLCBLAS_LIB})
else()
    message(STATUS "GSL not found - testbenchmark may not link without GSL")
endif()

# Optional: Add custom targets for convenience
add_custom_target(run-tests
    COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/testbenchmark
    DEPENDS testbenchmark
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Running testbenchmark"
)
