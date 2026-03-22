#include <cstdlib>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include "betree_cpp.hpp"

namespace {

void require(bool condition, const char* message) {
    if (!condition) {
        std::cerr << message << '\n';
        std::exit(1);
    }
}

bool insert_with_ephemeral_view(be::Tree& tree) {
    constexpr std::size_t expr_len = sizeof("xage >= 21 and country = \"USA\"") - 1;
    const std::string expr_storage = "xage >= 21 and country = \"USA\" trailing";
    return tree.insert(7, std::string_view(expr_storage).substr(0, expr_len));
}

bool insert_with_frequency_constants(be::Tree& tree) {
    constexpr std::size_t expr_len =
        sizeof("within_frequency_cap(\"flight:ip\", \"3495614\", 1, 5184000)") - 1;
    const std::string expr_storage =
        "within_frequency_cap(\"flight:ip\", \"3495614\", 1, 5184000) trailing";
    return tree.insert_with_constants(
        9,
        std::string_view(expr_storage).substr(0, expr_len),
        {
            {"campaign_id", 50650},
            {"advertiser_id", 6573},
            {"flight_id", 101801},
        });
}

void verify_wrapper_rejections() {
    be::Tree tree;
    tree.add_integer("age", false, 0, 10)
        .add_string("country", false, 8)
        .add_frequency_caps("frequency_caps", false)
        .add_integer("now", false, 0, 1000);

    require(!tree.insert(100, "missing = 1"),
            "wrapper insert accepted unknown variable");
    require(!tree.insert(101, "country = "),
            "wrapper insert accepted malformed expression");
    require(!tree.insert_with_constants(
                102,
                "within_frequency_cap(\"flight\", \"ns\", 100, 0)",
                {
                    {"advertiser_id", 20},
                    {"campaign_id", 30},
                }),
            "wrapper insert_with_constants accepted missing required constants");

    const auto invalid_event = tree.search("{\"age\": \"bad\", \"country\": \"USA\"}");
    require(invalid_event.empty(),
            "wrapper search did not return empty result on invalid event");

    const auto missing_required = tree.search("{\"country\": \"USA\"}");
    require(missing_required.empty(),
            "wrapper search did not return empty result on missing required variable");

    const std::vector<std::uint64_t> ids = {100, 101, 102};
    const auto invalid_event_ids = tree.search("{\"age\": \"bad\", \"country\": \"USA\"}", ids);
    require(invalid_event_ids.empty(),
            "wrapper filtered search did not return empty result on invalid event");
}

} // namespace

int main() {
    be::Tree tree;

    const std::string schema_name = "xage_suffix";
    const std::string country_name = "country_extra";
    constexpr std::size_t event_len =
        sizeof("{\"xage\": 25, \"country\": \"USA\", \"tags\": [1], \"labels\": [\"x\"], \"status\": 1}") - 1;

    tree.add_integer(std::string_view(schema_name).substr(0, 4), false, 0, 120)
        .add_string(std::string_view(country_name).substr(0, 7), false, 8)
        .add_integer_list("tags", true, 0, 100)
        .add_string_list("labels", true, 8)
        .add_integer_enum("status", true, 8);

    require(insert_with_ephemeral_view(tree),
            "insert with sliced string_view failed");
    require(tree.insert(8, "tags is empty and labels is empty and status = 2"),
            "insert with empty-list/integer-enum wrapper path failed");

    const std::string event_storage =
        "{\"xage\": 25, \"country\": \"USA\", \"tags\": [1], \"labels\": [\"x\"], \"status\": 1} trailing";
    const auto result = tree.search(std::string_view(event_storage).substr(0, event_len));
    require(result.size() == 1, "search with sliced string_view returned wrong match count");
    require(result.matched_subs[0] == 7, "search with sliced string_view returned wrong id");

    const std::vector<std::uint64_t> ids = {7, 9};
    const auto filtered = tree.search(std::string_view(event_storage).substr(0, event_len), ids);
    require(filtered.size() == 1, "filtered search with sliced string_view returned wrong count");
    require(filtered.matched_subs[0] == 7, "filtered search with sliced string_view returned wrong id");
    const std::vector<std::uint64_t> full_ids = {7, 8};
    const auto filtered_full = tree.search(std::string_view(event_storage).substr(0, event_len), full_ids);
    require(filtered_full.size() == result.size(), "filtered full-id search returned wrong count");
    require(filtered_full.matched_subs == result.matched_subs,
            "filtered full-id search did not match unfiltered wrapper results");

    const auto schema_def = tree.variable_definition(4);
    require(schema_def.type == BETREE_INTEGER_ENUM,
            "variable_definition returned wrong type for integer enum");

    const auto empty_lists = tree.search(
        "{\"xage\": 30, \"country\": \"CAN\", \"tags\": [], \"labels\": [], \"status\": 2}");
    require(empty_lists.size() == 1, "empty-list/integer-enum wrapper search returned wrong count");
    require(empty_lists.matched_subs[0] == 8,
            "empty-list/integer-enum wrapper search returned wrong id");
    const std::vector<std::uint64_t> empty_ids = {7, 8, 9};
    const auto empty_lists_filtered = tree.search(
        "{\"xage\": 30, \"country\": \"CAN\", \"tags\": [], \"labels\": [], \"status\": 2}",
        empty_ids);
    require(empty_lists_filtered.matched_subs == empty_lists.matched_subs,
            "wrapper filtered search did not preserve empty-list parity");

    be::Tree frequency_tree;
    frequency_tree.add_frequency_caps("frequency_caps", false).add_integer("now", false, 0, 2000000000);
    require(insert_with_frequency_constants(frequency_tree),
            "insert_with_constants wrapper path failed");
    const auto frequency_result = frequency_tree.search(
        "{\"now\": 1541704800, \"frequency_caps\": [[[\"flight:ip\",101801,\"3495614\"],1,1546537569676283]]}");
    require(frequency_result.empty(),
            "frequency constant wrapper search returned unexpected matches");
    const std::vector<std::uint64_t> frequency_ids = {9, 11};
    const auto filtered_frequency = frequency_tree.search(
        "{\"now\": 1541704800, \"frequency_caps\": [[[\"flight:ip\",101801,\"3495614\"],1,1546537569676283]]}",
        frequency_ids);
    require(filtered_frequency.empty(),
            "frequency constant wrapper filtered search returned unexpected matches");

    verify_wrapper_rejections();

    return 0;
}
