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
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include <stdexcept>
#include <initializer_list>
#include <cstdint>

extern "C" {
#include "betree.h"
}

namespace be {

namespace detail {

inline std::string as_c_string(std::string_view value) {
    return std::string(value);
}

} // namespace detail

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
    std::size_t evaluated = 0;                 ///< Number of expressions evaluated
    std::size_t memoized = 0;                  ///< Number of memoized results reused
    std::size_t shorted = 0;                   ///< Number of short-circuit optimizations

    bool empty() const { return matched_subs.empty(); }
    std::size_t size() const { return matched_subs.size(); }
};

struct IntegerConstant {
    std::string_view name;
    std::int64_t value;
};

struct Segment {
    std::int64_t id;
    std::int64_t timestamp;
};

struct FrequencyCap {
    std::string_view type;
    std::uint32_t id;
    std::string_view ns;
    bool timestamp_defined;
    std::int64_t timestamp;
    std::uint32_t value;
};

/**
 * RAII wrapper for betree_event
 */
class Event {
    std::shared_ptr<betree> tree_;
    std::unique_ptr<betree_event, decltype(&betree_free_event)> event_;

    friend class Tree;

    Event(std::shared_ptr<betree> tree, betree_event* evt)
        : tree_(std::move(tree)), event_(evt, betree_free_event) {
        if (!tree_ || !event_) {
            throw BetreeException("Invalid event");
        }
    }

    [[nodiscard]] const char* variable_name(std::size_t index) const {
        if (index >= event_->variable_count) {
            throw BetreeException("Event variable index out of range");
        }
        return betree_get_variable_definition(tree_.get(), index).name;
    }

    void set_variable(std::size_t index, betree_variable* variable) {
        if (!variable) {
            throw BetreeException("Failed to create event variable");
        }
        betree_set_variable(event_.get(), index, variable);
    }

public:
    betree_event* get() const { return event_.get(); }

    Event& clear(std::size_t index) {
        if (index >= event_->variable_count) {
            throw BetreeException("Event variable index out of range");
        }
        betree_set_variable(event_.get(), index, nullptr);
        return *this;
    }

    Event& set_boolean(std::size_t index, bool value) {
        set_variable(index, betree_make_boolean_variable(variable_name(index), value));
        return *this;
    }

    Event& set_integer(std::size_t index, std::int64_t value) {
        set_variable(index, betree_make_integer_variable(variable_name(index), value));
        return *this;
    }

    Event& set_integer_enum(std::size_t index, std::int64_t value) {
        return set_integer(index, value);
    }

    Event& set_float(std::size_t index, double value) {
        set_variable(index, betree_make_float_variable(variable_name(index), value));
        return *this;
    }

    Event& set_string(std::size_t index, std::string_view value) {
        const auto value_str = detail::as_c_string(value);
        set_variable(index, betree_make_string_variable(variable_name(index), value_str.c_str()));
        return *this;
    }

    Event& set_integer_list(std::size_t index, const std::vector<std::int64_t>& values) {
        const char* name = variable_name(index);
        betree_integer_list* list = betree_make_integer_list(values.size());
        for (std::size_t i = 0; i < values.size(); ++i) {
            betree_add_integer(list, i, values[i]);
        }
        set_variable(index, betree_make_integer_list_variable(name, list));
        return *this;
    }

    Event& set_integer_list(std::size_t index, std::initializer_list<std::int64_t> values) {
        return set_integer_list(index, std::vector<std::int64_t>(values));
    }

    Event& set_string_list(std::size_t index, const std::vector<std::string_view>& values) {
        const char* name = variable_name(index);
        betree_string_list* list = betree_make_string_list(values.size());
        for (std::size_t i = 0; i < values.size(); ++i) {
            const auto value_str = detail::as_c_string(values[i]);
            betree_add_string(list, i, value_str.c_str());
        }
        set_variable(index, betree_make_string_list_variable(name, list));
        return *this;
    }

    Event& set_string_list(std::size_t index, std::initializer_list<std::string_view> values) {
        return set_string_list(index, std::vector<std::string_view>(values));
    }

    Event& set_empty_list(std::size_t index) {
        const char* name = variable_name(index);
        set_variable(index, betree_make_integer_list_variable(name, betree_make_integer_list(0)));
        return *this;
    }

    Event& set_segments(std::size_t index, const std::vector<Segment>& values) {
        const char* name = variable_name(index);
        betree_segments* segments = betree_make_segments(values.size());
        for (std::size_t i = 0; i < values.size(); ++i) {
            betree_add_segment(
                segments, i, betree_make_segment(values[i].id, values[i].timestamp));
        }
        set_variable(index, betree_make_segments_variable(name, segments));
        return *this;
    }

    Event& set_segments(std::size_t index, std::initializer_list<Segment> values) {
        return set_segments(index, std::vector<Segment>(values));
    }

    Event& set_frequency_caps(std::size_t index, const std::vector<FrequencyCap>& values) {
        const char* name = variable_name(index);
        betree_frequency_caps* caps = betree_make_frequency_caps(values.size());
        for (std::size_t i = 0; i < values.size(); ++i) {
            const auto type_str = detail::as_c_string(values[i].type);
            const auto ns_str = detail::as_c_string(values[i].ns);
            betree_frequency_cap* cap = betree_make_frequency_cap(type_str.c_str(),
                values[i].id,
                ns_str.c_str(),
                values[i].timestamp_defined,
                values[i].timestamp,
                values[i].value);
            if (!cap) {
                betree_free_frequency_caps(caps);
                throw BetreeException("Invalid frequency-cap type");
            }
            betree_add_frequency_cap(caps, i, cap);
        }
        set_variable(index, betree_make_frequency_caps_variable(name, caps));
        return *this;
    }

    Event& set_frequency_caps(std::size_t index, std::initializer_list<FrequencyCap> values) {
        return set_frequency_caps(index, std::vector<FrequencyCap>(values));
    }
};

/**
 * Main BE-tree class with RAII resource management
 */
class Tree {
    std::shared_ptr<betree> tree_;

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
        const auto name_str = detail::as_c_string(name);
        betree_add_boolean_variable(tree_.get(), name_str.c_str(), allow_undefined);
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
        const auto name_str = detail::as_c_string(name);
        betree_add_integer_variable(tree_.get(), name_str.c_str(), allow_undefined, min, max);
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
        const auto name_str = detail::as_c_string(name);
        betree_add_float_variable(tree_.get(), name_str.c_str(), allow_undefined, min, max);
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
        const auto name_str = detail::as_c_string(name);
        betree_add_string_variable(tree_.get(), name_str.c_str(), allow_undefined, count);
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
        const auto name_str = detail::as_c_string(name);
        betree_add_integer_list_variable(tree_.get(), name_str.c_str(), allow_undefined, min, max);
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
        const auto name_str = detail::as_c_string(name);
        betree_add_string_list_variable(tree_.get(), name_str.c_str(), allow_undefined, count);
        return *this;
    }

    /**
     * Add an integer-enum variable to the schema
     *
     * @param name Variable name
     * @param allow_undefined Whether undefined values are allowed
     * @param count Estimated number of distinct enum values
     * @return Reference to this tree for method chaining
     */
    Tree& add_integer_enum(std::string_view name, bool allow_undefined, std::size_t count) {
        const auto name_str = detail::as_c_string(name);
        betree_add_integer_enum_variable(tree_.get(), name_str.c_str(), allow_undefined, count);
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
        const auto name_str = detail::as_c_string(name);
        betree_add_frequency_caps_variable(tree_.get(), name_str.c_str(), allow_undefined);
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
        const auto name_str = detail::as_c_string(name);
        betree_add_segments_variable(tree_.get(), name_str.c_str(), allow_undefined);
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
        const auto expr_str = detail::as_c_string(expr);
        return betree_insert(tree_.get(), id, expr_str.c_str());
    }

    /**
     * Insert a subscription expression with integer constants
     *
     * @param id Unique subscription ID
     * @param expr Boolean expression string
     * @param constants Integer constants available during insert
     * @return true if insertion succeeded, false otherwise
     */
    bool insert_with_constants(
        std::uint64_t id, std::string_view expr, const std::vector<IntegerConstant>& constants) {
        const auto expr_str = detail::as_c_string(expr);
        std::vector<betree_constant*> owned_constants;
        std::vector<const betree_constant*> raw_constants;
        owned_constants.reserve(constants.size());
        raw_constants.reserve(constants.size());
        for (const auto& constant : constants) {
            const auto name_str = detail::as_c_string(constant.name);
            betree_constant* ptr = betree_make_integer_constant(name_str.c_str(), constant.value);
            if (!ptr) {
                betree_free_constants(owned_constants.size(), owned_constants.data());
                throw BetreeException("Failed to create integer constant");
            }
            owned_constants.push_back(ptr);
            raw_constants.push_back(ptr);
        }

        const bool ok = betree_insert_with_constants(
            tree_.get(), id, raw_constants.size(), raw_constants.data(), expr_str.c_str());
        betree_free_constants(owned_constants.size(), owned_constants.data());
        return ok;
    }

    bool insert_with_constants(std::uint64_t id,
        std::string_view expr,
        std::initializer_list<IntegerConstant> constants) {
        return insert_with_constants(id, expr, std::vector<IntegerConstant>(constants));
    }

    Event make_event() const {
        return Event(tree_, betree_make_event(tree_.get()));
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
        const auto event_json_str = detail::as_c_string(event_json);
        if (betree_search(tree_.get(), event_json_str.c_str(), rep.get())) {
            result.matched_subs.assign(rep->subs, rep->subs + rep->matched);
            result.evaluated = rep->evaluated;
            result.memoized = rep->memoized;
            result.shorted = rep->shorted;
        }
        return result;
    }

    SearchResult search(Event& event) const {
        std::unique_ptr<report, decltype(&free_report)> rep(make_report(), free_report);
        if (!rep) {
            throw BetreeException("Failed to create report");
        }

        SearchResult result;
        if (betree_search_with_event(tree_.get(), event.get(), rep.get())) {
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
        const auto event_json_str = detail::as_c_string(event_json);
        if (betree_search_ids(tree_.get(), event_json_str.c_str(), rep.get(),
                             ids.data(), ids.size())) {
            result.matched_subs.assign(rep->subs, rep->subs + rep->matched);
            result.evaluated = rep->evaluated;
            result.memoized = rep->memoized;
            result.shorted = rep->shorted;
        }
        return result;
    }

    SearchResult search(Event& event, const std::vector<std::uint64_t>& ids) const {
        std::unique_ptr<report, decltype(&free_report)> rep(make_report(), free_report);
        if (!rep) {
            throw BetreeException("Failed to create report");
        }

        SearchResult result;
        if (betree_search_with_event_ids(
                tree_.get(), event.get(), rep.get(), ids.data(), ids.size())) {
            result.matched_subs.assign(rep->subs, rep->subs + rep->matched);
            result.evaluated = rep->evaluated;
            result.memoized = rep->memoized;
            result.shorted = rep->shorted;
        }
        return result;
    }

    bool exists(Event& event) const {
        return betree_exists_with_event(tree_.get(), event.get());
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
