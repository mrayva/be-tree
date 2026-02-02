#pragma once

#include <cstdint>
#include <vector>
#include <memory>

// Maintain C struct layout for ABI compatibility
// IMPORTANT: C code directly accesses arr->size and arr->data fields
// We maintain these fields in sync with the internal std::vector
struct dynamic_array_t {
    std::size_t size;       // Kept in sync with vector.size()
    std::size_t capacity;   // Kept in sync with vector.capacity()
    std::uint64_t* data;    // Kept in sync with vector.data()

    // Internal storage using std::vector for RAII and safety
    // This is opaque to C code - only C++ implementation uses it
    void* _internal_vector;  // Actually std::vector<uint64_t>*
};

#define INITIAL_CAPACITY 16

namespace dyn_arr_ops {

// Helper to get the internal vector
inline std::vector<std::uint64_t>* get_vector(dynamic_array_t* arr) {
    return static_cast<std::vector<std::uint64_t>*>(arr->_internal_vector);
}

inline const std::vector<std::uint64_t>* get_vector(const dynamic_array_t* arr) {
    return static_cast<const std::vector<std::uint64_t>*>(arr->_internal_vector);
}

// Sync the C-visible fields with the vector state
inline void sync_fields(dynamic_array_t* arr) {
    if (arr && arr->_internal_vector) {
        auto* vec = get_vector(arr);
        arr->size = vec->size();
        arr->capacity = vec->capacity();
        arr->data = vec->data();
    }
}

// Modern C++ implementation using std::vector
inline dynamic_array_t* create_dynamic_array_impl(std::size_t initial_capacity) {
    try {
        auto* arr = new dynamic_array_t{};
        arr->_internal_vector = new std::vector<std::uint64_t>();
        auto* vec = get_vector(arr);
        vec->reserve(initial_capacity);
        sync_fields(arr);
        return arr;
    } catch (const std::bad_alloc&) {
        return nullptr;
    }
}

inline void clear_dynamic_array_impl(dynamic_array_t* arr) noexcept {
    if (arr && arr->_internal_vector) {
        get_vector(arr)->clear();
        sync_fields(arr);
    }
}

inline void destroy_dynamic_array_impl(dynamic_array_t* arr) noexcept {
    if (arr) {
        if (arr->_internal_vector) {
            delete get_vector(arr);
        }
        delete arr;
    }
}

inline void resize_dynamic_array_impl(dynamic_array_t* arr, std::size_t new_capacity) {
    if (!arr || !arr->_internal_vector) return;

    try {
        auto* vec = get_vector(arr);
        vec->reserve(new_capacity);
        if (vec->size() > new_capacity) {
            vec->resize(new_capacity);
        }
        sync_fields(arr);
    } catch (const std::bad_alloc&) {
        std::abort();
    }
}

inline void dynamic_array_add_impl(dynamic_array_t* arr, std::uint64_t element) {
    if (!arr || !arr->_internal_vector) return;

    try {
        get_vector(arr)->push_back(element);
        sync_fields(arr);
    } catch (const std::bad_alloc&) {
        std::abort();
    }
}

inline dynamic_array_t* dynamic_array_merge_impl(dynamic_array_t* arr1, dynamic_array_t* arr2) {
    if (!arr1 || !arr2 || !arr1->_internal_vector || !arr2->_internal_vector) {
        return nullptr;
    }

    try {
        auto* vec1 = get_vector(arr1);
        auto* vec2 = get_vector(arr2);

        auto* merged_arr = create_dynamic_array_impl(vec1->size() + vec2->size());
        if (!merged_arr) return nullptr;

        auto* merged_vec = get_vector(merged_arr);
        merged_vec->insert(merged_vec->end(), vec1->begin(), vec1->end());
        merged_vec->insert(merged_vec->end(), vec2->begin(), vec2->end());

        sync_fields(merged_arr);
        return merged_arr;
    } catch (const std::bad_alloc&) {
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
