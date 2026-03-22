# Examples for BE-tree library usage

# Only build examples if requested
if(BUILD_EXAMPLES)
    # Basic C API example
    add_executable(basic_usage basic_usage.c)
    target_link_libraries(basic_usage PRIVATE betree)
    target_include_directories(basic_usage PRIVATE ${CMAKE_SOURCE_DIR}/src)

    # Filtered search C API example
    add_executable(filtered_search filtered_search.c)
    target_link_libraries(filtered_search PRIVATE betree)
    target_include_directories(filtered_search PRIVATE ${CMAKE_SOURCE_DIR}/src)

    # _err reason-reporting C API example
    add_executable(err_reason_example err_reason_example.c)
    target_link_libraries(err_reason_example PRIVATE betree)
    target_include_directories(err_reason_example PRIVATE ${CMAKE_SOURCE_DIR}/src)

    # Modern C++ example (inline wrapper)
    add_executable(cpp_usage cpp_usage.cpp)
    target_link_libraries(cpp_usage PRIVATE betree)
    target_include_directories(cpp_usage PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_compile_features(cpp_usage PRIVATE cxx_std_20)

    # Modern C++ example using betree_cpp.hpp API
    add_executable(cpp_modern cpp_modern.cpp)
    target_link_libraries(cpp_modern PRIVATE betree)
    target_include_directories(cpp_modern PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/include
    )
    target_compile_features(cpp_modern PRIVATE cxx_std_20)

    # Structured-event example for special domains
    add_executable(cpp_special_events cpp_special_events.cpp)
    target_link_libraries(cpp_special_events PRIVATE betree)
    target_include_directories(cpp_special_events PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/include
    )
    target_compile_features(cpp_special_events PRIVATE cxx_std_20)

    # Create examples target to build all examples
    add_custom_target(examples DEPENDS
        basic_usage
        filtered_search
        err_reason_example
        cpp_usage
        cpp_modern
        cpp_special_events
    )

    # Set output directory for examples
    set_target_properties(
        basic_usage filtered_search err_reason_example cpp_usage cpp_modern cpp_special_events
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/cmake/examples"
    )

    # Installation (optional)
    # install(TARGETS basic_usage cpp_usage
    #     RUNTIME DESTINATION examples
    # )

    message(STATUS "Examples will be built in ${CMAKE_BINARY_DIR}/cmake/examples/")
endif()
