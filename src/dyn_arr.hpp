#pragma once

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <new>

// Maintain C struct layout for ABI compatibility
// C code directly accesses arr->size field
struct dynamic_array_t {
    std::size_t size;
    std::size_t capacity;
    std::uint64_t* data;
};

#define INITIAL_CAPACITY 16

namespace dyn_arr_ops {

// C++ helper functions using modern practices
inline dynamic_array_t* create_dynamic_array_impl(std::size_t initial_capacity) {
    try {
        auto* arr = new dynamic_array_t;
        arr->data = new std::uint64_t[initial_capacity];
        arr->size = 0;
        arr->capacity = initial_capacity;
        return arr;
    } catch (const std::bad_alloc&) {
        std::perror("Failed to allocate memory for dynamic array");
        return nullptr;
    }
}

inline void clear_dynamic_array_impl(dynamic_array_t* arr) noexcept {
    if (arr) {
        arr->size = 0;
    }
}

inline void destroy_dynamic_array_impl(dynamic_array_t* arr) noexcept {
    if (arr) {
        delete[] arr->data;
        delete arr;
    }
}

inline void resize_dynamic_array_impl(dynamic_array_t* arr, std::size_t new_capacity) {
    if (!arr) return;

    try {
        auto* new_data = new std::uint64_t[new_capacity];
        std::size_t copy_size = (arr->size < new_capacity) ? arr->size : new_capacity;
        if (arr->data && copy_size > 0) {
            std::memcpy(new_data, arr->data, copy_size * sizeof(std::uint64_t));
        }
        delete[] arr->data;
        arr->data = new_data;
        arr->capacity = new_capacity;
        if (arr->size > new_capacity) {
            arr->size = new_capacity;
        }
    } catch (const std::bad_alloc&) {
        std::perror("Failed to reallocate memory for array data");
        std::abort();
    }
}

inline void dynamic_array_add_impl(dynamic_array_t* arr, std::uint64_t element) {
    if (!arr) return;

    if (arr->size >= arr->capacity) {
        std::size_t new_capacity = (arr->capacity == 0) ? 1 : (arr->capacity << 1);
        resize_dynamic_array_impl(arr, new_capacity);
    }

    arr->data[arr->size++] = element;
}

inline dynamic_array_t* dynamic_array_merge_impl(dynamic_array_t* arr1, dynamic_array_t* arr2) {
    if (!arr1 || !arr2) return nullptr;

    try {
        auto* merged_arr = create_dynamic_array_impl(arr1->size + arr2->size);
        if (!merged_arr) return nullptr;

        if (arr1->size > 0) {
            std::memcpy(merged_arr->data, arr1->data, arr1->size * sizeof(std::uint64_t));
        }
        if (arr2->size > 0) {
            std::memcpy(merged_arr->data + arr1->size, arr2->data, arr2->size * sizeof(std::uint64_t));
        }

        merged_arr->size = arr1->size + arr2->size;
        return merged_arr;
    } catch (const std::bad_alloc&) {
        std::perror("Failed to merge arrays");
        return nullptr;
    }
}

} // namespace dyn_arr_ops

// C API compatibility
#ifdef __cplusplus
extern "C" {
#endif

dynamic_array_t* create_dynamic_array(std::size_t initial_capacity);
void clear_dynamic_array(dynamic_array_t* arr);
void destroy_dynamic_array(dynamic_array_t* arr);
void resize_dynamic_array(dynamic_array_t* arr, std::size_t new_capacity);
void dynamic_array_add(dynamic_array_t* arr, std::uint64_t element);
dynamic_array_t* dynamic_array_merge(dynamic_array_t* arr1, dynamic_array_t* arr2);

#ifdef __cplusplus
}
#endif
