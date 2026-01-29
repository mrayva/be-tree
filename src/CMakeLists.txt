# Source files for betree library

# List of core C source files (excluding generated ones)
set(BETREE_C_SOURCES
    alloc.c
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
    memoize.c
    printer.c
    special.c
    tree.c
    tree_err.c
    utils.c
    value.c
    var.c
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
set(BETREE_CPP_SOURCES
    debug.cpp
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
    # When building as C++, compile C sources as C++ (except generated files)
    set(ALL_SOURCES
        ${BETREE_C_SOURCES}
        ${BETREE_CPP_SOURCES}
        ${BETREE_COMPAT_SOURCES}
        ${BETREE_GENERATED_SOURCES}
    )
    
    add_library(betree STATIC ${ALL_SOURCES})
    
    # Set C sources to compile as C++, except for generated files
    set_source_files_properties(
        ${BETREE_C_SOURCES}
        ${BETREE_CPP_SOURCES}
        ${BETREE_COMPAT_SOURCES}
        PROPERTIES LANGUAGE CXX
    )
    
    # Force generated files to remain as C
    set_source_files_properties(
        ${BETREE_GENERATED_SOURCES}
        PROPERTIES LANGUAGE C
    )
    
    # Set C++ standard
    target_compile_features(betree PUBLIC cxx_std_20)
    
    # Add compiler warnings for C++
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(betree PRIVATE
            -Wall
            -Wextra
            -Wpedantic
            -Wno-deprecated-declarations
            $<$<COMPILE_LANGUAGE:CXX>:-Wno-old-style-cast>
            $<$<COMPILE_LANGUAGE:CXX>:-Wno-zero-as-null-pointer-constant>
        )
    endif()
else()
    # When building as C, use standard C compilation (without converted C++ modules)
    set(ALL_SOURCES
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
