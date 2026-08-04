cmake_minimum_required(VERSION 3.20)

# Project declaration
project(be-tree
    VERSION 0.1.0
    LANGUAGES C CXX
    DESCRIPTION "Binary Expression Tree library with a C API and C++20 core"
)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# vcpkg integration guidance
# To use vcpkg, set CMAKE_TOOLCHAIN_FILE to point to vcpkg's toolchain file:
# cmake -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake ..
# Or set the environment variable VCPKG_ROOT and it will be auto-detected below
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "Vcpkg toolchain file")
endif()

# Build options
# The historical pure-C core is no longer present in this checkout. The canonical
# option therefore describes what the build actually selects: the supported C++ core,
# while generated parser files and a few support layers remain C.
if(DEFINED BUILD_CPP_CORE)
    set(_build_cpp_core_default "${BUILD_CPP_CORE}")
elseif(DEFINED BUILD_CONVERTED_CPP_CORE)
    set(_build_cpp_core_default "${BUILD_CONVERTED_CPP_CORE}")
elseif(DEFINED BUILD_AS_CPP)
    set(_build_cpp_core_default "${BUILD_AS_CPP}")
else()
    set(_build_cpp_core_default ON)
endif()

option(BUILD_CPP_CORE
    "Build with the supported C++ core implementation"
    ${_build_cpp_core_default}
)
set(BUILD_CONVERTED_CPP_CORE "${BUILD_CPP_CORE}" CACHE BOOL
    "Deprecated alias for BUILD_CPP_CORE" FORCE)
set(BUILD_AS_CPP "${BUILD_CPP_CORE}" CACHE BOOL
    "Deprecated alias for BUILD_CPP_CORE" FORCE)
unset(_build_cpp_core_default)
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
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/cmake/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/cmake/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/cmake/bin)

enable_testing()

function(add_betree_c_test test_name)
    add_executable(${test_name} tests/${test_name}.c)
    target_link_libraries(${test_name} PRIVATE betree)
    target_include_directories(${test_name} PRIVATE ${CMAKE_SOURCE_DIR}/src)
    set_source_files_properties(tests/${test_name}.c PROPERTIES LANGUAGE C)
    add_test(NAME ${test_name} COMMAND ${test_name})
    set_tests_properties(${test_name} PROPERTIES WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
endfunction()

# Source directory
add_subdirectory(src)
add_library(betree::betree ALIAS betree)

# Examples directory (optional)
add_subdirectory(examples)

# Benchmark-style test executables
# These require GSL library
find_package(GSL QUIET)

if(GSL_FOUND)
    # testbenchmark - real_tests.c
    add_executable(testbenchmark tests/real_tests.c)
    target_link_libraries(testbenchmark PRIVATE betree GSL::gsl GSL::gslcblas m)
    target_include_directories(testbenchmark PRIVATE ${CMAKE_SOURCE_DIR}/src)
    # Test files remain as C - they test the C API exposed via extern "C"
    add_test(NAME real_tests COMMAND testbenchmark 1)
    set_tests_properties(real_tests PROPERTIES
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        LABELS "benchmark"
    )

    # testbenchmark_err - real_tests_err.c
    add_executable(testbenchmark_err tests/real_tests_err.c)
    target_link_libraries(testbenchmark_err PRIVATE betree GSL::gsl GSL::gslcblas m)
    target_include_directories(testbenchmark_err PRIVATE ${CMAKE_SOURCE_DIR}/src)
    # Test files remain as C - they test the C API exposed via extern "C"
    add_test(NAME real_tests_err COMMAND testbenchmark_err 1)
    set_tests_properties(real_tests_err PROPERTIES
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        LABELS "benchmark"
    )
else()
    message(STATUS "GSL not found - testbenchmark executables will not be built")
    message(STATUS "Install GSL (libgsl-dev on Debian/Ubuntu) or use vcpkg to build tests")
endif()

add_executable(cpp_wrapper_smoke tests/cpp_wrapper_smoke.cpp)
target_link_libraries(cpp_wrapper_smoke PRIVATE betree)
target_include_directories(cpp_wrapper_smoke PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/include
)
target_compile_features(cpp_wrapper_smoke PRIVATE cxx_std_20)
add_test(NAME cpp_wrapper_smoke COMMAND cpp_wrapper_smoke)
set_tests_properties(cpp_wrapper_smoke PROPERTIES WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

set(BETREE_UNIT_TESTS
    betree_search_cb_tests
    betree_search_reason_tests
    betree_search_tests
    betree_tests
    bound_tests
    change_boundaries_tests
    counting_tests
    dyn_arr_tests
    eq_expr_tests
    event_parser_tests
    integration_tests
    memoize_tests
    parser_tests
    performance_tests
    printer_tests
    report_tests
    special_tests
    unfetched_tests
    unfetched_err_tests
    valid_tests
)

foreach(test_name IN LISTS BETREE_UNIT_TESTS)
    add_betree_c_test(${test_name})
endforeach()

add_test(NAME package_consumer
    COMMAND ${CMAKE_COMMAND}
        -DBETREE_SOURCE_DIR=${CMAKE_SOURCE_DIR}
        -DBETREE_BINARY_DIR=${CMAKE_BINARY_DIR}
        -DBETREE_GENERATOR=${CMAKE_GENERATOR}
        -DBETREE_C_COMPILER=${CMAKE_C_COMPILER}
        -DBETREE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
        "-DBETREE_C_FLAGS=${CMAKE_C_FLAGS}"
        "-DBETREE_CXX_FLAGS=${CMAKE_CXX_FLAGS}"
        "-DBETREE_EXE_LINKER_FLAGS=${CMAKE_EXE_LINKER_FLAGS}"
        -DBETREE_CONFIG=$<CONFIG>
        -P ${CMAKE_SOURCE_DIR}/tests/package_consumer/run.cmake
)

# Custom targets for convenience
add_custom_target(build-all
    DEPENDS betree
    COMMENT "Building all targets"
)

# Print configuration summary
message(STATUS "")
message(STATUS "be-tree configuration:")
message(STATUS "  BUILD_CPP_CORE: ${BUILD_CPP_CORE}")
message(STATUS "  BUILD_EXAMPLES: ${BUILD_EXAMPLES}")
message(STATUS "  C++ Standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "  C Standard: C${CMAKE_C_STANDARD}")
if(GSL_FOUND)
    message(STATUS "  GSL library: Found")
endif()
message(STATUS "")

# Installation and CMake package export
install(TARGETS betree
    EXPORT betreeTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES
    include/betree_cpp.hpp
    src/betree.h
    src/betree.hpp
    src/betree_err.h
    src/betree_err.hpp
    src/dyn_arr.h
    src/dyn_arr.hpp
    src/value.h
    src/value.hpp
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT development
)

set(BETREE_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/betree")
configure_package_config_file(
    cmake/betreeConfig.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/betreeConfig.cmake"
    INSTALL_DESTINATION "${BETREE_INSTALL_CMAKEDIR}"
)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/betreeConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(EXPORT betreeTargets
    FILE betreeTargets.cmake
    NAMESPACE betree::
    DESTINATION "${BETREE_INSTALL_CMAKEDIR}"
)
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/betreeConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/betreeConfigVersion.cmake"
    DESTINATION "${BETREE_INSTALL_CMAKEDIR}"
)
