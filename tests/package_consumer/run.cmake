set(prefix "${BETREE_BINARY_DIR}/package-consumer-prefix")
set(consumer_build "${BETREE_BINARY_DIR}/package-consumer-build")
file(REMOVE_RECURSE "${prefix}" "${consumer_build}")

set(config_args)
if(BETREE_CONFIG)
    list(APPEND config_args --config "${BETREE_CONFIG}")
endif()

execute_process(
    COMMAND "${CMAKE_COMMAND}" --install "${BETREE_BINARY_DIR}" --prefix "${prefix}" ${config_args}
    RESULT_VARIABLE install_result
)
if(NOT install_result EQUAL 0)
    message(FATAL_ERROR "Failed to install be-tree package")
endif()

execute_process(
    COMMAND "${CMAKE_COMMAND}"
        -S "${BETREE_SOURCE_DIR}/tests/package_consumer"
        -B "${consumer_build}"
        -G "${BETREE_GENERATOR}"
        -DCMAKE_PREFIX_PATH=${prefix}
        -DCMAKE_C_COMPILER=${BETREE_C_COMPILER}
        -DCMAKE_CXX_COMPILER=${BETREE_CXX_COMPILER}
        "-DCMAKE_C_FLAGS=${BETREE_C_FLAGS}"
        "-DCMAKE_CXX_FLAGS=${BETREE_CXX_FLAGS}"
        "-DCMAKE_EXE_LINKER_FLAGS=${BETREE_EXE_LINKER_FLAGS}"
    RESULT_VARIABLE configure_result
)
if(NOT configure_result EQUAL 0)
    message(FATAL_ERROR "Failed to configure installed-package consumer")
endif()

execute_process(
    COMMAND "${CMAKE_COMMAND}" --build "${consumer_build}" ${config_args}
    RESULT_VARIABLE build_result
)
if(NOT build_result EQUAL 0)
    message(FATAL_ERROR "Failed to build installed-package consumer")
endif()

execute_process(
    COMMAND "${consumer_build}/c_package_consumer"
    RESULT_VARIABLE c_run_result
)
if(NOT c_run_result EQUAL 0)
    message(FATAL_ERROR "Installed C package consumer failed")
endif()

execute_process(
    COMMAND "${consumer_build}/cpp_package_consumer"
    RESULT_VARIABLE cpp_run_result
)
if(NOT cpp_run_result EQUAL 0)
    message(FATAL_ERROR "Installed C++ package consumer failed")
endif()
