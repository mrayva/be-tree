/*
 * Modern C++ BE-Tree Usage Example
 *
 * This example demonstrates:
 * - RAII-based wrapper around C API
 * - Modern C++ idioms (smart pointers, RAII)
 * - Exception-safe resource management
 * - Using the library from C++20 code
 */

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <stdexcept>

extern "C" {
#include "betree.h"
}

namespace be {

// RAII wrapper for struct betree
class Tree {
    std::unique_ptr<betree, decltype(&betree_free)> tree_;

public:
    Tree() : tree_(betree_make(), betree_free) {
        if (!tree_) {
            throw std::runtime_error("Failed to create BE-tree");
        }
    }

    // No copy, only move
    Tree(const Tree&) = delete;
    Tree& operator=(const Tree&) = delete;
    Tree(Tree&&) = default;
    Tree& operator=(Tree&&) = default;

    // Add variables with method chaining
    Tree& add_integer(const std::string& name, bool allow_undef, int64_t min, int64_t max) {
        betree_add_integer_variable(tree_.get(), name.c_str(), allow_undef, min, max);
        return *this;
    }

    Tree& add_string(const std::string& name, bool allow_undef, size_t count) {
        betree_add_string_variable(tree_.get(), name.c_str(), allow_undef, count);
        return *this;
    }

    Tree& add_boolean(const std::string& name, bool allow_undef) {
        betree_add_boolean_variable(tree_.get(), name.c_str(), allow_undef);
        return *this;
    }

    Tree& add_float(const std::string& name, bool allow_undef, double min, double max) {
        betree_add_float_variable(tree_.get(), name.c_str(), allow_undef, min, max);
        return *this;
    }

    // Insert subscription
    bool insert(uint64_t id, const std::string& expr) {
        return betree_insert(tree_.get(), id, expr.c_str());
    }

    // Search with RAII report
    struct SearchResult {
        std::vector<uint64_t> matched_subs;
        size_t evaluated;
        size_t memoized;
        size_t shorted;
    };

    SearchResult search(const std::string& event_json) const {
        std::unique_ptr<report, decltype(&free_report)> rep(make_report(), free_report);

        SearchResult result;
        if (betree_search(tree_.get(), event_json.c_str(), rep.get())) {
            result.matched_subs.assign(rep->subs, rep->subs + rep->matched);
            result.evaluated = rep->evaluated;
            result.memoized = rep->memoized;
            result.shorted = rep->shorted;
        }
        return result;
    }

    betree* get() const { return tree_.get(); }
};

} // namespace be

int main()
{
    using namespace be;

    std::cout << "BE-Tree Modern C++ Usage Example\n";
    std::cout << "=================================\n\n";

    try {
        // 1. Create tree and define schema with method chaining
        std::cout << "Creating BE-tree and defining schema...\n";

        Tree tree;
        tree.add_integer("age", false, 0, 150)
            .add_string("country", false, 100)
            .add_boolean("premium", false)
            .add_float("score", false, 0.0, 100.0);

        std::cout << "  ✓ Schema defined\n\n";

        // 2. Insert subscriptions
        std::cout << "Inserting subscriptions:\n";

        tree.insert(1, "age >= 18 and age <= 25 and country = \"USA\"");
        std::cout << "  ✓ Sub 1: Young adults in USA\n";

        tree.insert(2, "premium and score >= 80.0");
        std::cout << "  ✓ Sub 2: High-scoring premium users\n";

        tree.insert(3, "age >= 30 and (country = \"Canada\" or country = \"UK\")");
        std::cout << "  ✓ Sub 3: Adults in Canada/UK\n";

        tree.insert(4, "score >= 90.0");
        std::cout << "  ✓ Sub 4: Top scorers\n\n";

        // 3. Match events (RAII automatically cleans up)
        std::cout << "Matching events:\n";

        auto test_event = [](const Tree& tree, const std::string& event, const std::string& description) {
            std::cout << "\n" << description << "\n";
            std::cout << "  Event: " << event << "\n";

            auto result = tree.search(event);

            std::cout << "  Matched: " << result.matched_subs.size() << " subscription(s)";
            if (!result.matched_subs.empty()) {
                std::cout << " [";
                for (size_t i = 0; i < result.matched_subs.size(); ++i) {
                    if (i > 0) std::cout << ", ";
                    std::cout << result.matched_subs[i];
                }
                std::cout << "]";
            }
            std::cout << "\n";
            std::cout << "  Stats: evaluated=" << result.evaluated
                     << ", memoized=" << result.memoized
                     << ", shorted=" << result.shorted << "\n";
        };

        // Test various events
        test_event(tree,
            R"({"age": 22, "country": "USA", "premium": true, "score": 85.5})",
            "Event 1: Young premium user from USA with good score");

        test_event(tree,
            R"({"age": 35, "country": "Canada", "premium": false, "score": 95.0})",
            "Event 2: Adult from Canada with excellent score");

        test_event(tree,
            R"({"age": 12, "country": "USA", "premium": true, "score": 75.0})",
            "Event 3: Premium child with average score");

        test_event(tree,
            R"({"age": 40, "country": "UK", "premium": true, "score": 92.0})",
            "Event 4: Premium adult from UK with top score");

        std::cout << "\n✓ Example completed successfully\n";

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    // RAII automatically cleans up tree
    std::cout << "Cleanup complete (automatic via RAII)\n";
    return 0;
}
