# Source files for be-tree library

# Regular C source files (non-generated)
set(BETREE_SOURCES
    alloc.c
    ast.c
    ast_compare.c
    ast_err.c
    betree.c
    betree_err.c
    config.c
    debug.c
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
    value.c
    var.c
)

# Conditionally use C++ version of utils when building as C++
if(BUILD_AS_CPP)
    list(APPEND BETREE_SOURCES utils.cpp)
else()
    list(APPEND BETREE_SOURCES utils.c)
endif()

# Generated files from lex/yacc - compile as C even when BUILD_AS_CPP is ON
# TODO: If these files don't exist yet, they need to be generated using flex/bison
#       Run: make realclean && make to generate them, or add custom commands here
set(BETREE_GENERATED_SOURCES
    lexer.c
    parser.c
    event_lexer.c
    event_parser.c
)

# When building as C++, add compatibility shim
if(BUILD_AS_CPP)
    list(APPEND BETREE_SOURCES
        compat_alloc.cpp
    )
endif()

# Create static library
add_library(betree STATIC
    ${BETREE_SOURCES}
    ${BETREE_GENERATED_SOURCES}
)

# Set language for source files
if(BUILD_AS_CPP)
    # Compile regular sources as C++
    set_source_files_properties(${BETREE_SOURCES} PROPERTIES LANGUAGE CXX)
    # Keep generated files as C
    set_source_files_properties(${BETREE_GENERATED_SOURCES} PROPERTIES LANGUAGE C)
    
    # Set C++ standard
    target_compile_features(betree PUBLIC cxx_std_20)
    
    # Add compiler warnings for C++ but be less strict for C-as-C++ compilation
    target_compile_options(betree PRIVATE
        $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wno-missing-field-initializers -fpermissive>
        $<$<CXX_COMPILER_ID:MSVC>:/W4>
    )
else()
    # Compile all as C
    set_source_files_properties(${BETREE_SOURCES} ${BETREE_GENERATED_SOURCES} PROPERTIES LANGUAGE C)
    target_compile_features(betree PUBLIC c_std_11)
    
    # Add compiler warnings for C (matching Makefile)
    target_compile_options(betree PRIVATE
        $<$<C_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wshadow -Wfloat-equal -Wundef -Wcast-align -Wwrite-strings -Wunreachable-code -Wformat=2 -Wswitch-enum -Wswitch-default -Winit-self>
    )
endif()

# Add include directories
target_include_directories(betree PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# NIF support (optional Erlang integration)
if(NIF)
    target_compile_definitions(betree PUBLIC NIF)
    # TODO: Add Erlang include paths if NIF is enabled
    # find_package(Erlang REQUIRED)
    # target_include_directories(betree PRIVATE ${ERLANG_INCLUDE_PATH})
endif()

# Link math library
target_link_libraries(betree PUBLIC m)

# Link optional C++ dependencies when building as C++
if(BUILD_AS_CPP)
    if(fmt_FOUND)
        target_link_libraries(betree PRIVATE fmt::fmt)
    endif()
    if(Microsoft.GSL_FOUND)
        target_link_libraries(betree PRIVATE Microsoft.GSL::GSL)
    endif()
endif()
