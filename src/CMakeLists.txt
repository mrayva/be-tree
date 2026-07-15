# Source files for betree library

# C support sources that still exist in this checkout and do not have a mirrored
# C++ implementation. Mirrored modules are built from their `.cpp`
# translation units only, otherwise the archive contains duplicate exported symbols
# that broader test coverage will surface at link time.
set(BETREE_C_SOURCES
    special.c
)

# Allocation layer - must remain as C for the public C API boundary
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

# Flex and Bison emit scaffolding that triggers these Clang diagnostics. Keep
# the exceptions source-local so hand-written C remains subject to all warnings.
set_property(SOURCE lexer.c event_lexer.c APPEND PROPERTY COMPILE_OPTIONS
    "$<$<C_COMPILER_ID:Clang>:-Wno-unreachable-code>"
)
set_property(SOURCE parser.c event_parser.c APPEND PROPERTY COMPILE_OPTIONS
    "$<$<C_COMPILER_ID:Clang>:-Wno-unused-but-set-variable>"
)

# C++ source files for the supported core implementation.
# See examples/ directory for usage demonstrations.
set(BETREE_CPP_SOURCES
    memoize.cpp     # Bitset operations
    dyn_arr.cpp     # Dynamic array with stable ABI layout
    utils.cpp       # Utility functions using std::min/max/fabs
    hashmap.cpp     # Predicate map using red-black tree
    special.cpp     # Frequency caps, segments, geo, string utilities
    var.cpp         # Variable getter functions
    jsw_rbtree.cpp  # Red-black tree implementation
    ast_compare.cpp # AST node comparison functions
    config.cpp      # Configuration and attribute domain management
    value.cpp       # Value structures and operations
    printer.cpp     # AST and tree printing utilities
    debug_err.cpp   # DOT file generation for error trees
    ast_err.cpp     # AST node matching and evaluation
    debug.cpp       # DOT file generation and tree printing
    betree_err.cpp  # BE-tree error structure management (905 LOC)
    map.cpp         # Hash map implementation (207 LOC)
    betree.cpp      # Main BE-tree API implementation (1077 LOC)
    tree_err.cpp    # Error tree operations and subscriptions (1595 LOC)
    tree.cpp        # Tree operations and subscriptions (2346 LOC)
    ast.cpp         # AST node creation and matching (3396 LOC)
    helper.cpp      # Helper functions for variable parsing and tree operations (106 LOC)
    helper_err.cpp  # Helper functions for error tree variable parsing (106 LOC)
)

# Allocation bridge used by the C++ core
set(BETREE_CPP_BRIDGE_SOURCES
    compat_alloc.cpp
)

# Check if generated files exist
foreach(gen_file ${BETREE_GENERATED_SOURCES})
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${gen_file}")
        message(WARNING "Generated file ${gen_file} not found. Run 'make' in the repository root to generate lex/yacc files first.")
    endif()
endforeach()

# Create the betree library
if(BUILD_CPP_CORE)
    # The supported core build uses a mixed-language approach:
    # - Keep remaining C sources as C for the public C API boundary
    # - Build the supported core implementation as C++20
    # - Use the allocation bridge where needed
    set(ALL_SOURCES
        ${BETREE_ALLOC_SOURCES}
        ${BETREE_C_SOURCES}
        ${BETREE_CPP_SOURCES}
        ${BETREE_CPP_BRIDGE_SOURCES}
        ${BETREE_GENERATED_SOURCES}
    )
    
    add_library(betree STATIC ${ALL_SOURCES})
    
    # Only compile the C++ sources and allocation bridge as C++
    # Keep C sources as C for the public C boundary
    set_source_files_properties(
        ${BETREE_CPP_SOURCES}
        ${BETREE_CPP_BRIDGE_SOURCES}
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
    # The legacy pure-C core is no longer available in this checkout.
    # BUILD_CPP_CORE=OFF therefore keeps the public C API surface but still
    # links the supported C++ core implementation alongside the remaining C support files.
    set(ALL_SOURCES
        ${BETREE_ALLOC_SOURCES}
        ${BETREE_C_SOURCES}
        ${BETREE_CPP_SOURCES}
        ${BETREE_CPP_BRIDGE_SOURCES}
        ${BETREE_GENERATED_SOURCES}
    )

    add_library(betree STATIC ${ALL_SOURCES})

    set_source_files_properties(
        ${BETREE_CPP_SOURCES}
        ${BETREE_CPP_BRIDGE_SOURCES}
        PROPERTIES LANGUAGE CXX
    )

    set_source_files_properties(
        ${BETREE_ALLOC_SOURCES}
        ${BETREE_C_SOURCES}
        ${BETREE_GENERATED_SOURCES}
        PROPERTIES LANGUAGE C
    )

    target_compile_features(betree PUBLIC cxx_std_20)

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
endif()

# Include directories
target_include_directories(betree PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

# Link math library (required on Unix-like systems)
target_link_libraries(betree PUBLIC m)

# Set PIC for shared library usage
set_target_properties(betree PROPERTIES POSITION_INDEPENDENT_CODE ON)
