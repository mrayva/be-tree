# Source files for betree library

# List of core C source files (excluding generated ones and allocation layer)
# alloc.c is kept separate as it must remain as C
set(BETREE_C_SOURCES
    ast.c
    ast_compare.c
    ast_err.c
    betree.c
    betree_err.c
    config.c
    debug_err.c
    dyn_arr.c
    hashmap.c
    helper.c
    helper_err.c
    jsw_rbtree.c
    map.c
    printer.c
    special.c
    tree.c
    tree_err.c
    utils.c
    value.c
    var.c
)

# Allocation layer - must remain as C for compatibility
set(BETREE_ALLOC_SOURCES
    alloc.c
)

# Generated files from lex/yacc (these must remain as C)
# Note: These files should be generated before building
set(BETREE_GENERATED_SOURCES
    lexer.c
    parser.c
    event_lexer.c
    event_parser.c
)

# C++ source files (converted modules)
# NOTE: Example conversions are in examples/ directory for reference.
# Modules cannot be converted until headers are refactored to avoid C++ keywords
# (e.g., 'namespace' field in value.h must be renamed before C++ conversion is possible).
# When ready to convert a module:
# 1. Complete header refactoring
# 2. Add the converted .cpp file here
# 3. Remove the corresponding .c file from BETREE_C_SOURCES
set(BETREE_CPP_SOURCES
    memoize.cpp  # First converted module - bitset operations
    # Future converted C++ modules will be listed here
    # Example: debug.cpp (see examples/README.md for reference implementation)
)

# Compatibility shim for C++ compilation
set(BETREE_COMPAT_SOURCES
    compat_alloc.cpp
)

# Check if generated files exist
foreach(gen_file ${BETREE_GENERATED_SOURCES})
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${gen_file}")
        message(WARNING "Generated file ${gen_file} not found. Run 'make' in the repository root to generate lex/yacc files first.")
    endif()
endforeach()

# Create the betree library
if(BUILD_AS_CPP)
    # When building as C++, we use a mixed approach:
    # - Keep all C sources as C for compatibility (C++ keyword conflicts prevent direct compilation)
    # - Add C++ compatibility shim for future C++ modules
    # - Include converted C++ modules when ready (currently none due to header compatibility)
    # This provides the infrastructure for incremental migration
    set(ALL_SOURCES
        ${BETREE_ALLOC_SOURCES}
        ${BETREE_C_SOURCES}
        ${BETREE_CPP_SOURCES}
        ${BETREE_COMPAT_SOURCES}
        ${BETREE_GENERATED_SOURCES}
        debug.c  # Using C version for now
    )
    
    add_library(betree STATIC ${ALL_SOURCES})
    
    # Only compile the C++ sources and compat shim as C++
    # Keep C sources as C for compatibility
    set_source_files_properties(
        ${BETREE_CPP_SOURCES}
        ${BETREE_COMPAT_SOURCES}
        PROPERTIES LANGUAGE CXX
    )
    
    # Explicitly mark C sources as C (default but explicit for clarity)
    set_source_files_properties(
        ${BETREE_ALLOC_SOURCES}
        ${BETREE_C_SOURCES}
        ${BETREE_GENERATED_SOURCES}
        PROPERTIES LANGUAGE C
    )
    
    # Set C++ standard for C++ files
    target_compile_features(betree PUBLIC cxx_std_20)
    
    # Add compiler warnings for C++
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(betree PRIVATE
            $<$<COMPILE_LANGUAGE:CXX>:-Wall>
            $<$<COMPILE_LANGUAGE:CXX>:-Wextra>
            $<$<COMPILE_LANGUAGE:CXX>:-Wpedantic>
            $<$<COMPILE_LANGUAGE:CXX>:-Wno-deprecated-declarations>
            $<$<COMPILE_LANGUAGE:CXX>:-Wno-old-style-cast>
            $<$<COMPILE_LANGUAGE:CXX>:-Wno-zero-as-null-pointer-constant>
        )
    endif()
    
    # Add compiler warnings for C
    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(betree PRIVATE
            $<$<COMPILE_LANGUAGE:C>:-Wall>
            $<$<COMPILE_LANGUAGE:C>:-Wextra>
            $<$<COMPILE_LANGUAGE:C>:-Wpedantic>
            $<$<COMPILE_LANGUAGE:C>:-Wshadow>
            $<$<COMPILE_LANGUAGE:C>:-Wfloat-equal>
            $<$<COMPILE_LANGUAGE:C>:-Wundef>
            $<$<COMPILE_LANGUAGE:C>:-Wcast-align>
            $<$<COMPILE_LANGUAGE:C>:-Wwrite-strings>
            $<$<COMPILE_LANGUAGE:C>:-Wunreachable-code>
            $<$<COMPILE_LANGUAGE:C>:-Wformat=2>
        )
    endif()
else()
    # When building as C, use standard C compilation (without converted C++ modules)
    set(ALL_SOURCES
        ${BETREE_ALLOC_SOURCES}
        ${BETREE_C_SOURCES}
        ${BETREE_GENERATED_SOURCES}
        debug.c
    )
    
    add_library(betree STATIC ${ALL_SOURCES})
    
    # Add compiler warnings for C
    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(betree PRIVATE
            -Wall
            -Wextra
            -Wpedantic
            -Wshadow
            -Wfloat-equal
            -Wundef
            -Wcast-align
            -Wwrite-strings
            -Wunreachable-code
            -Wformat=2
        )
    endif()
endif()

# Include directories
target_include_directories(betree PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

# Link math library (required on Unix-like systems)
target_link_libraries(betree PUBLIC m)

# Link optional dependencies if found
if(TARGET fmt::fmt)
    target_link_libraries(betree PUBLIC fmt::fmt)
endif()

if(TARGET Microsoft.GSL::GSL)
    target_link_libraries(betree PUBLIC Microsoft.GSL::GSL)
endif()

# Set PIC for shared library usage
set_target_properties(betree PROPERTIES POSITION_INDEPENDENT_CODE ON)

# Installation rules (optional, for future use)
# install(TARGETS betree
#     ARCHIVE DESTINATION lib
#     LIBRARY DESTINATION lib
# )
