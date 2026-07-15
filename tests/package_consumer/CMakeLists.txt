cmake_minimum_required(VERSION 3.20)

project(betree-package-consumer LANGUAGES C CXX)

find_package(betree CONFIG REQUIRED)

add_executable(c_package_consumer main.c)
target_link_libraries(c_package_consumer PRIVATE betree::betree)

add_executable(cpp_package_consumer main.cpp)
target_link_libraries(cpp_package_consumer PRIVATE betree::betree)
target_compile_features(cpp_package_consumer PRIVATE cxx_std_20)
