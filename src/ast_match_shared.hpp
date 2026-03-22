#pragma once

#include <cstddef>
#include <cstdint>

#include "ast.h"

static inline std::size_t ast_match_next_low(
    const std::int64_t arr[], std::size_t low, std::size_t count, std::int64_t x)
{
    std::size_t high = count - 1;
    while (low < high) {
        std::size_t mid = low + (high - low) / 2;
        if (x <= arr[mid]) {
            high = mid;
        } else {
            low = mid + 1;
        }
    }
    if (low < count && arr[low] < x) {
        low++;
    }
    return low;
}

static inline bool ast_match_d64binary_search(
    const std::int64_t arr[], std::size_t count, std::int64_t to_find)
{
    int imin = 0;
    int imax = static_cast<int>(count) - 1;
    while (imax >= imin) {
        int imid = imin + ((imax - imin) / 2);
        if (arr[imid] == to_find) {
            return true;
        }
        if (arr[imid] < to_find) {
            imin = imid + 1;
        } else {
            imax = imid - 1;
        }
    }
    return false;
}

static inline bool ast_match_d64binary_search_counting(
    const std::int64_t arr[], std::size_t count, std::int64_t to_find, int* ops_count)
{
    int imin = 0;
    int imax = static_cast<int>(count) - 1;
    while (imax >= imin) {
        (*ops_count)++;
        int imid = imin + ((imax - imin) / 2);
        if (arr[imid] == to_find) {
            return true;
        }
        if (arr[imid] < to_find) {
            imin = imid + 1;
        } else {
            imax = imid - 1;
        }
    }
    return false;
}

static inline bool ast_match_sbinary_search(
    struct string_value arr[], std::size_t count, betree_str_t to_find)
{
    int imin = 0;
    int imax = static_cast<int>(count) - 1;
    while (imax >= imin) {
        int imid = imin + ((imax - imin) / 2);
        if (arr[imid].str == to_find) {
            return true;
        }
        if (arr[imid].str < to_find) {
            imin = imid + 1;
        } else {
            imax = imid - 1;
        }
    }
    return false;
}

static inline bool ast_match_sbinary_search_counting(
    struct string_value arr[], std::size_t count, betree_str_t to_find, int* ops_count)
{
    int imin = 0;
    int imax = static_cast<int>(count) - 1;
    while (imax >= imin) {
        (*ops_count)++;
        int imid = imin + ((imax - imin) / 2);
        if (arr[imid].str == to_find) {
            return true;
        }
        if (arr[imid].str < to_find) {
            imin = imid + 1;
        } else {
            imax = imid - 1;
        }
    }
    return false;
}

static inline bool ast_match_integer_in_integer_list(
    std::int64_t integer, struct betree_integer_list* list)
{
    return ast_match_d64binary_search(list->integers, list->count, integer);
}

static inline bool ast_match_integer_in_integer_list_counting(
    std::int64_t integer, struct betree_integer_list* list, int* ops_count)
{
    return ast_match_d64binary_search_counting(list->integers, list->count, integer, ops_count);
}

static inline bool ast_match_string_in_string_list(
    struct string_value string, struct betree_string_list* list)
{
    return ast_match_sbinary_search(list->strings, list->count, string.str);
}

static inline bool ast_match_string_in_string_list_counting(
    struct string_value string, struct betree_string_list* list, int* ops_count)
{
    return ast_match_sbinary_search_counting(list->strings, list->count, string.str, ops_count);
}

static inline bool ast_match_not_all_of_int(struct value variable, struct ast_list_expr list_expr)
{
    std::int64_t* xs;
    std::size_t x_count;
    std::int64_t* ys;
    std::size_t y_count;
    if (variable.integer_list_value->count < list_expr.value.integer_list_value->count) {
        xs = variable.integer_list_value->integers;
        x_count = variable.integer_list_value->count;
        ys = list_expr.value.integer_list_value->integers;
        y_count = list_expr.value.integer_list_value->count;
    } else {
        ys = variable.integer_list_value->integers;
        y_count = variable.integer_list_value->count;
        xs = list_expr.value.integer_list_value->integers;
        x_count = list_expr.value.integer_list_value->count;
    }
    std::size_t i = 0;
    std::size_t from = 0;
    while (i < x_count && from < y_count) {
        std::int64_t x = xs[i];
        from = ast_match_next_low(ys, from, y_count, x);
        if (from < y_count && ys[from] == x) {
            return true;
        }
        i++;
    }
    return false;
}

static inline bool ast_match_not_all_of_int_counting(
    struct value variable, struct ast_list_expr list_expr, int* ops_count)
{
    (*ops_count)++;
    std::int64_t* xs;
    std::size_t x_count;
    std::int64_t* ys;
    std::size_t y_count;
    if (variable.integer_list_value->count < list_expr.value.integer_list_value->count) {
        xs = variable.integer_list_value->integers;
        x_count = variable.integer_list_value->count;
        ys = list_expr.value.integer_list_value->integers;
        y_count = list_expr.value.integer_list_value->count;
    } else {
        ys = variable.integer_list_value->integers;
        y_count = variable.integer_list_value->count;
        xs = list_expr.value.integer_list_value->integers;
        x_count = list_expr.value.integer_list_value->count;
    }
    std::size_t i = 0;
    std::size_t from = 0;
    while (i < x_count && from < y_count) {
        (*ops_count)++;
        std::int64_t x = xs[i];
        from = ast_match_next_low(ys, from, y_count, x);
        if (from < y_count && ys[from] == x) {
            return true;
        }
        i++;
    }
    return false;
}

static inline bool ast_match_not_all_of_string(
    struct value variable, struct ast_list_expr list_expr)
{
    struct string_value* xs;
    std::size_t x_count;
    struct string_value* ys;
    std::size_t y_count;
    if (variable.string_list_value->count < list_expr.value.string_list_value->count) {
        xs = variable.string_list_value->strings;
        x_count = variable.string_list_value->count;
        ys = list_expr.value.string_list_value->strings;
        y_count = list_expr.value.string_list_value->count;
    } else {
        ys = variable.string_list_value->strings;
        y_count = variable.string_list_value->count;
        xs = list_expr.value.string_list_value->strings;
        x_count = list_expr.value.string_list_value->count;
    }
    std::size_t i = 0;
    std::size_t j = 0;
    while (i < x_count && j < y_count) {
        struct string_value* x = &xs[i];
        struct string_value* y = &ys[j];
        if (x->str == y->str) {
            return true;
        }
        if (y->str < x->str) {
            j++;
        } else {
            i++;
        }
    }
    return false;
}

static inline bool ast_match_not_all_of_string_counting(
    struct value variable, struct ast_list_expr list_expr, int* ops_count)
{
    (*ops_count)++;
    struct string_value* xs;
    std::size_t x_count;
    struct string_value* ys;
    std::size_t y_count;
    if (variable.string_list_value->count < list_expr.value.string_list_value->count) {
        xs = variable.string_list_value->strings;
        x_count = variable.string_list_value->count;
        ys = list_expr.value.string_list_value->strings;
        y_count = list_expr.value.string_list_value->count;
    } else {
        ys = variable.string_list_value->strings;
        y_count = variable.string_list_value->count;
        xs = list_expr.value.string_list_value->strings;
        x_count = list_expr.value.string_list_value->count;
    }
    std::size_t i = 0;
    std::size_t j = 0;
    while (i < x_count && j < y_count) {
        (*ops_count)++;
        struct string_value* x = &xs[i];
        struct string_value* y = &ys[j];
        if (x->str == y->str) {
            return true;
        }
        if (y->str < x->str) {
            j++;
        } else {
            i++;
        }
    }
    return false;
}

static inline bool ast_match_all_of_int(struct value variable, struct ast_list_expr list_expr)
{
    std::int64_t* xs = list_expr.value.integer_list_value->integers;
    std::size_t x_count = list_expr.value.integer_list_value->count;
    std::int64_t* ys = variable.integer_list_value->integers;
    std::size_t y_count = variable.integer_list_value->count;
    if (x_count <= y_count) {
        std::size_t from = 0;
        std::size_t j = 0;
        while (from < y_count && j < x_count) {
            std::int64_t x = xs[j];
            from = ast_match_next_low(ys, from, y_count, x);
            if (from < y_count && ys[from] == x) {
                j++;
            } else {
                return false;
            }
        }
        return j == x_count;
    }
    return false;
}

static inline bool ast_match_all_of_int_counting(
    struct value variable, struct ast_list_expr list_expr, int* ops_count)
{
    (*ops_count)++;
    std::int64_t* xs = list_expr.value.integer_list_value->integers;
    std::size_t x_count = list_expr.value.integer_list_value->count;
    std::int64_t* ys = variable.integer_list_value->integers;
    std::size_t y_count = variable.integer_list_value->count;
    if (x_count <= y_count) {
        std::size_t from = 0;
        std::size_t j = 0;
        while (from < y_count && j < x_count) {
            (*ops_count)++;
            std::int64_t x = xs[j];
            from = ast_match_next_low(ys, from, y_count, x);
            if (from < y_count && ys[from] == x) {
                j++;
            } else {
                return false;
            }
        }
        return j == x_count;
    }
    return false;
}

static inline bool ast_match_all_of_string(
    struct value variable, struct ast_list_expr list_expr)
{
    struct string_value* xs = list_expr.value.string_list_value->strings;
    std::size_t x_count = list_expr.value.string_list_value->count;
    struct string_value* ys = variable.string_list_value->strings;
    std::size_t y_count = variable.string_list_value->count;
    if (x_count <= y_count) {
        std::size_t i = 0;
        std::size_t j = 0;
        while (i < y_count && j < x_count) {
            struct string_value* x = &xs[j];
            struct string_value* y = &ys[i];
            if (y->str < x->str) {
                i++;
            } else if (x->str == y->str) {
                i++;
                j++;
            } else {
                return false;
            }
        }
        return j == x_count;
    }
    return false;
}

static inline bool ast_match_all_of_string_counting(
    struct value variable, struct ast_list_expr list_expr, int* ops_count)
{
    (*ops_count)++;
    struct string_value* xs = list_expr.value.string_list_value->strings;
    std::size_t x_count = list_expr.value.string_list_value->count;
    struct string_value* ys = variable.string_list_value->strings;
    std::size_t y_count = variable.string_list_value->count;
    if (x_count <= y_count) {
        std::size_t i = 0;
        std::size_t j = 0;
        while (i < y_count && j < x_count) {
            (*ops_count)++;
            struct string_value* x = &xs[j];
            struct string_value* y = &ys[i];
            if (y->str < x->str) {
                i++;
            } else if (x->str == y->str) {
                i++;
                j++;
            } else {
                return false;
            }
        }
        return j == x_count;
    }
    return false;
}
