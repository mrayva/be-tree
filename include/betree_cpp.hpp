/*
 * Modern C++ Wrapper for BE-Tree Library
 *
 * This header provides a RAII-based, exception-safe C++ interface to the
 * BE-tree library. It wraps the C API with modern C++ idioms including:
 * - Automatic resource management (RAII)
 * - Smart pointers
 * - STL containers
 * - Method chaining
 * - std::string_view for zero-copy string parameters
 * - Exception-based error handling
 *
 * Usage:
 *   #include <betree_cpp.hpp>
 *
 *   be::Tree tree;
 *   tree.add_integer("age", false, 0, 150)
 *       .add_string("country", false, 100)
 *       .insert(1, "age >= 18 and country = \"USA\"");
 *
 *   auto results = tree.search(R"({"age": 25, "country": "USA"})");
 *   for (auto sub_id : results.matched_subs) {
 *       std::cout << "Matched: " << sub_id << "\n";
 *   }
 */

#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <stdexcept>
#include <optional>
#include <cstdint>

extern "C" {
#include "betree.h"
}

namespace be {

// Forward declarations
class Tree;
class Event;
class Report;

/**
 * Exception thrown when BE-tree operations fail
 */
class BetreeException : public std::runtime_error {
public:
    explicit BetreeException(const std::string& message)
        : std::runtime_error("BE-tree error: " + message) {}
};

/**
 * Search results returned from tree.search()
 */
struct SearchResult {
    std::vector<std::uint64_t> matched_subs;  ///< IDs of matched subscriptions
    std::size_t evaluated;                     ///< Number of expressions evaluated
    std::size_t memoized;                      ///< Number of memoized results reused
    std::size_t shorted;                       ///< Number of short-circuit optimizations

    bool empty() const { return matched_subs.empty(); }
    std::size_t size() const { return matched_subs.size(); }
};

/**
 * RAII wrapper for betree_event
 */
class Event {
    std::unique_ptr<betree_event, decltype(&betree_free_event)> event_;

public:
    Event(betree_event* evt) : event_(evt, betree_free_event) {
        if (!event_) {
            throw BetreeException("Invalid event");
        }
    }

    betree_event* get() const { return event_.get(); }
};

/**
 * Main BE-tree class with RAII resource management
 */
class Tree {
    std::unique_ptr<betree, decltype(&betree_free)> tree_;

public:
    /**
     * Create a new BE-tree with default parameters
     */
    Tree() : tree_(betree_make(), betree_free) {
        if (!tree_) {
            throw BetreeException("Failed to create BE-tree");
        }
    }

    /**
     * Create a new BE-tree with custom parameters
     *
     * @param lnode_max_cap Maximum capacity for leaf nodes
     * @param min_partition_size Minimum partition size for tree splitting
     */
    Tree(std::uint64_t lnode_max_cap, std::uint64_t min_partition_size)
        : tree_(betree_make_with_parameters(lnode_max_cap, min_partition_size), betree_free) {
        if (!tree_) {
            throw BetreeException("Failed to create BE-tree with parameters");
        }
    }

    // No copy, only move
    Tree(const Tree&) = delete;
    Tree& operator=(const Tree&) = delete;
    Tree(Tree&&) = default;
    Tree& operator=(Tree&&) = default;

    /**
     * Add a boolean variable to the schema
     *
     * @param name Variable name
     * @param allow_undefined Whether undefined values are allowed
     * @return Reference to this tree for method chaining
     */
    Tree& add_boolean(std::string_view name, bool allow_undefined = false) {
        betree_add_boolean_variable(tree_.get(), name.data(), allow_undefined);
        return *this;
    }

    /**
     * Add an integer variable to the schema
     *
     * @param name Variable name
     * @param allow_undefined Whether undefined values are allowed
     * @param min Minimum value (inclusive)
     * @param max Maximum value (inclusive)
     * @return Reference to this tree for method chaining
     */
    Tree& add_integer(std::string_view name, bool allow_undefined,
                      std::int64_t min, std::int64_t max) {
        betree_add_integer_variable(tree_.get(), name.data(), allow_undefined, min, max);
        return *this;
    }

    /**
     * Add a float variable to the schema
     *
     * @param name Variable name
     * @param allow_undefined Whether undefined values are allowed
     * @param min Minimum value (inclusive)
     * @param max Maximum value (inclusive)
     * @return Reference to this tree for method chaining
     */
    Tree& add_float(std::string_view name, bool allow_undefined,
                    double min, double max) {
        betree_add_float_variable(tree_.get(), name.data(), allow_undefined, min, max);
        return *this;
    }

    /**
     * Add a string variable to the schema
     *
     * @param name Variable name
     * @param allow_undefined Whether undefined values are allowed
     * @param count Estimated number of unique string values (for optimization)
     * @return Reference to this tree for method chaining
     */
    Tree& add_string(std::string_view name, bool allow_undefined, std::size_t count) {
        betree_add_string_variable(tree_.get(), name.data(), allow_undefined, count);
        return *this;
    }

    /**
     * Add an integer list variable to the schema
     *
     * @param name Variable name
     * @param allow_undefined Whether undefined values are allowed
     * @param min Minimum value for list elements
     * @param max Maximum value for list elements
     * @return Reference to this tree for method chaining
     */
    Tree& add_integer_list(std::string_view name, bool allow_undefined,
                           std::int64_t min, std::int64_t max) {
        betree_add_integer_list_variable(tree_.get(), name.data(), allow_undefined, min, max);
        return *this;
    }

    /**
     * Add a string list variable to the schema
     *
     * @param name Variable name
     * @param allow_undefined Whether undefined values are allowed
     * @param count Estimated number of unique string values
     * @return Reference to this tree for method chaining
     */
    Tree& add_string_list(std::string_view name, bool allow_undefined, std::size_t count) {
        betree_add_string_list_variable(tree_.get(), name.data(), allow_undefined, count);
        return *this;
    }

    /**
     * Add a frequency caps variable to the schema
     *
     * @param name Variable name
     * @param allow_undefined Whether undefined values are allowed
     * @return Reference to this tree for method chaining
     */
    Tree& add_frequency_caps(std::string_view name, bool allow_undefined) {
        betree_add_frequency_caps_variable(tree_.get(), name.data(), allow_undefined);
        return *this;
    }

    /**
     * Add a segments variable to the schema
     *
     * @param name Variable name
     * @param allow_undefined Whether undefined values are allowed
     * @return Reference to this tree for method chaining
     */
    Tree& add_segments(std::string_view name, bool allow_undefined) {
        betree_add_segments_variable(tree_.get(), name.data(), allow_undefined);
        return *this;
    }

    /**
     * Insert a subscription expression into the tree
     *
     * @param id Unique subscription ID
     * @param expr Boolean expression string
     * @return true if insertion succeeded, false otherwise
     */
    bool insert(std::uint64_t id, std::string_view expr) {
        return betree_insert(tree_.get(), id, expr.data());
    }

    /**
     * Search for matching subscriptions given an event JSON string
     *
     * @param event_json JSON string representing the event
     * @return SearchResult containing matched subscription IDs and statistics
     */
    SearchResult search(std::string_view event_json) const {
        std::unique_ptr<report, decltype(&free_report)> rep(make_report(), free_report);
        if (!rep) {
            throw BetreeException("Failed to create report");
        }

        SearchResult result;
        if (betree_search(tree_.get(), event_json.data(), rep.get())) {
            result.matched_subs.assign(rep->subs, rep->subs + rep->matched);
            result.evaluated = rep->evaluated;
            result.memoized = rep->memoized;
            result.shorted = rep->shorted;
        }
        return result;
    }

    /**
     * Search for matching subscriptions, filtering by specific subscription IDs
     *
     * @param event_json JSON string representing the event
     * @param ids Vector of subscription IDs to check
     * @return SearchResult containing matched subscription IDs and statistics
     */
    SearchResult search(std::string_view event_json,
                       const std::vector<std::uint64_t>& ids) const {
        std::unique_ptr<report, decltype(&free_report)> rep(make_report(), free_report);
        if (!rep) {
            throw BetreeException("Failed to create report");
        }

        SearchResult result;
        if (betree_search_ids(tree_.get(), event_json.data(), rep.get(),
                             ids.data(), ids.size())) {
            result.matched_subs.assign(rep->subs, rep->subs + rep->matched);
            result.evaluated = rep->evaluated;
            result.memoized = rep->memoized;
            result.shorted = rep->shorted;
        }
        return result;
    }

    /**
     * Get variable definition by index
     * Note: The C API does not provide a way to query the total count,
     * so you must know the schema you defined.
     *
     * @param index Variable index
     * @return Variable definition
     */
    betree_variable_definition variable_definition(std::size_t index) const {
        return betree_get_variable_definition(tree_.get(), index);
    }

    /**
     * Get the underlying C API betree pointer
     * Use with caution - prefer the C++ API when possible
     *
     * @return Raw pointer to betree structure
     */
    betree* get() const { return tree_.get(); }

    /**
     * Get the underlying C API betree pointer (const version)
     *
     * @return Raw const pointer to betree structure
     */
    const betree* c_ptr() const { return tree_.get(); }
};

} // namespace be
