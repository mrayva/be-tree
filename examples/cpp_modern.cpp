/*
 * Modern C++ BE-Tree Usage with betree_cpp.hpp
 *
 * This example demonstrates using the production-quality C++ wrapper API
 * provided by betree_cpp.hpp. This API offers:
 * - RAII-based automatic resource management
 * - Exception-safe operations
 * - Method chaining for fluent API
 * - STL containers for results
 * - std::string_view for zero-copy strings
 * - No manual memory management required
 */

#include <iostream>
#include <iomanip>
#include "../include/betree_cpp.hpp"

void print_results(const std::string& description, const be::SearchResult& result)
{
    std::cout << "\n" << description << "\n";
    std::cout << "  Matched: " << result.size() << " subscription(s)";

    if (!result.empty()) {
        std::cout << " [";
        for (std::size_t i = 0; i < result.matched_subs.size(); ++i) {
            if (i > 0) std::cout << ", ";
            std::cout << result.matched_subs[i];
        }
        std::cout << "]";
    }
    std::cout << "\n";

    std::cout << "  Performance: "
              << "evaluated=" << result.evaluated
              << ", memoized=" << result.memoized
              << ", shorted=" << result.shorted << "\n";
}

int main()
{
    std::cout << "BE-Tree Modern C++ API Example\n";
    std::cout << "===============================\n\n";

    try {
        // 1. Create tree and define schema with method chaining
        std::cout << "Building schema...\n";

        be::Tree tree;
        tree.add_integer("age", false, 0, 150)
            .add_string("country", false, 100)
            .add_boolean("premium", false)
            .add_float("score", false, 0.0, 100.0)
            .add_integer_list("purchase_history", false, 0, 10000);

        std::cout << "  ✓ Schema defined with 5 variables\n";

        // 2. Insert subscriptions with method chaining
        std::cout << "\nInserting subscriptions:\n";

        bool success = true;
        success &= tree.insert(1, "age >= 18 and age <= 25 and country = \"USA\"");
        std::cout << "  " << (success ? "✓" : "✗")
                  << " Sub 1: Young adults in USA\n";

        success &= tree.insert(2, "premium and score >= 80.0");
        std::cout << "  " << (success ? "✓" : "✗")
                  << " Sub 2: High-scoring premium users\n";

        success &= tree.insert(3, "age >= 30 and (country = \"Canada\" or country = \"UK\")");
        std::cout << "  " << (success ? "✓" : "✗")
                  << " Sub 3: Adults in Canada/UK\n";

        success &= tree.insert(4, "score >= 90.0");
        std::cout << "  " << (success ? "✓" : "✗")
                  << " Sub 4: Top scorers\n";

        success &= tree.insert(5, "10 in purchase_history");
        std::cout << "  " << (success ? "✓" : "✗")
                  << " Sub 5: Purchased item 10\n";

        // 3. Match events using string literals
        std::cout << "\nMatching events:\n";

        // Event 1: Young premium user with good score
        auto result1 = tree.search(R"({
            "age": 22,
            "country": "USA",
            "premium": true,
            "score": 85.5,
            "purchase_history": [10, 20, 30]
        })");
        print_results("Event 1: Young premium USA user", result1);

        // Event 2: Adult from Canada with excellent score
        auto result2 = tree.search(R"({
            "age": 35,
            "country": "Canada",
            "premium": false,
            "score": 95.0,
            "purchase_history": [5, 15]
        })");
        print_results("Event 2: Adult from Canada with top score", result2);

        // Event 3: Premium child (no age match)
        auto result3 = tree.search(R"({
            "age": 12,
            "country": "USA",
            "premium": true,
            "score": 75.0,
            "purchase_history": []
        })");
        print_results("Event 3: Premium child", result3);

        // Event 4: Search with filtered IDs
        std::vector<std::uint64_t> filter_ids = {1, 3, 5};
        auto result4 = tree.search(R"({
            "age": 40,
            "country": "UK",
            "premium": true,
            "score": 92.0,
            "purchase_history": [10, 25]
        })", filter_ids);
        print_results("Event 4: UK user (filtered IDs: 1, 3, 5)", result4);

        // 4. Summary
        std::cout << "\nSearch summary:\n";
        std::cout << "  Total events tested: 4\n";
        std::cout << "  All searches completed successfully\n";

        std::cout << "\n✓ Example completed successfully\n";
        std::cout << "  (All resources automatically cleaned up via RAII)\n";

    } catch (const be::BetreeException& e) {
        std::cerr << "BE-tree error: " << e.what() << "\n";
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
