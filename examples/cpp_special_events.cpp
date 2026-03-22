/*
 * C++ wrapper example for special-domain structured events.
 *
 * Demonstrates:
 * - segments
 * - frequency caps
 * - integer-enum and empty-list event helpers
 * - filtered structured-event search
 */

#include <iostream>
#include <vector>

#include "../include/betree_cpp.hpp"

namespace {

void print_result(const char* label, const be::SearchResult& result)
{
    std::cout << label << ": matched=" << result.size();
    if (!result.empty()) {
        std::cout << " [";
        for (std::size_t i = 0; i < result.matched_subs.size(); ++i) {
            if (i > 0) std::cout << ", ";
            std::cout << result.matched_subs[i];
        }
        std::cout << "]";
    }
    std::cout << '\n';
}

} // namespace

int main()
{
    try {
        std::cout << "BE-Tree C++ Special Event Example\n";
        std::cout << "================================\n\n";

        be::Tree tree;
        tree.add_integer_enum("status", false, 8)
            .add_string_list("labels", false, 16)
            .add_segments("seg", false)
            .add_frequency_caps("frequency_caps", false)
            .add_integer("now", false, 0, 1000);

        tree.insert(1, "status = 2 and labels is empty");
        tree.insert_with_constants(
            2,
            "segment_within(seg, 1, 20) and within_frequency_cap(\"flight\", \"ns\", 100, 0)",
            {{"flight_id", 10}});

        auto event = tree.make_event();
        event.set_integer_enum(0, 2)
             .set_empty_list(1)
             .set_segments(2, {{1, 10000000}})
             .set_frequency_caps(3, {{"flight", 10, "ns", false, 0, 0}})
             .set_integer(4, 0);

        auto full_result = tree.search(event);
        print_result("Structured event", full_result);

        std::vector<std::uint64_t> ids = {2};
        auto filtered_result = tree.search(event, ids);
        print_result("Filtered structured event", filtered_result);

        std::cout << "exists(event): " << (tree.exists(event) ? "true" : "false") << "\n";

        event.set_integer_list(2, {1});
        auto invalid_result = tree.search(event);
        print_result("Type-mismatched structured event", invalid_result);
        std::cout << "exists(invalid event): " << (tree.exists(event) ? "true" : "false") << "\n";

        return 0;
    } catch (const be::BetreeException& e) {
        std::cerr << e.what() << '\n';
        return 1;
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
