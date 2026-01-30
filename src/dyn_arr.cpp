#include "dyn_arr.hpp"

extern "C" {

dynamic_array_t* create_dynamic_array(std::size_t initial_capacity) {
    return betree::create_dynamic_array_impl(initial_capacity);
}

void clear_dynamic_array(dynamic_array_t* arr) {
    betree::clear_dynamic_array_impl(arr);
}

void destroy_dynamic_array(dynamic_array_t* arr) {
    betree::destroy_dynamic_array_impl(arr);
}

void resize_dynamic_array(dynamic_array_t* arr, std::size_t new_capacity) {
    betree::resize_dynamic_array_impl(arr, new_capacity);
}

void dynamic_array_add(dynamic_array_t* arr, std::uint64_t element) {
    betree::dynamic_array_add_impl(arr, element);
}

dynamic_array_t* dynamic_array_merge(dynamic_array_t* arr1, dynamic_array_t* arr2) {
    return betree::dynamic_array_merge_impl(arr1, arr2);
}

} // extern "C"
