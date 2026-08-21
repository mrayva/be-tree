#include <cstdlib>
#include <iostream>
#include <optional>
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
    // Regression cases for a real leak: trailing garbage *after* an
    // otherwise-complete sub-expression still leaves parse()'s *node
    // pointing at a fully-built (but now orphaned) tree when the overall
    // parse then fails on the garbage - unlike "country = " above, which
    // never completes any sub-expression at all. betree_insert_with_
    // constants() previously only freed that tree on every *other* failure
    // branch, not this one. Both cases return false either way (rejection
    // was never in question); it's ASan's leak check (see .github/
    // workflows/pr-checks.yml's "Check for leaks" step) that actually
    // exercises the regression - these calls alone prove nothing on a
    // build without a sanitizer.
    require(!tree.insert(103, "this is not valid !!!"),
            "wrapper insert accepted nonsense expression (bare-identifier trailing garbage)");
    require(!tree.insert(104, "age >= 21 and"),
            "wrapper insert accepted incomplete combinator (comparison-expr trailing garbage)");
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
    require(invalid_event.evaluated == 0 && invalid_event.memoized == 0
                && invalid_event.shorted == 0,
            "wrapper invalid search returned uninitialized statistics");

    const auto missing_required = tree.search("{\"country\": \"USA\"}");
    require(missing_required.empty(),
            "wrapper search did not return empty result on missing required variable");

    const std::vector<std::uint64_t> ids = {100, 101, 102};
    const auto invalid_event_ids = tree.search("{\"age\": \"bad\", \"country\": \"USA\"}", ids);
    require(invalid_event_ids.empty(),
            "wrapper filtered search did not return empty result on invalid event");

    const auto malformed_event = tree.search("{\"age\": 5");
    require(malformed_event.empty(),
            "wrapper search accepted malformed JSON");
    require(malformed_event.evaluated == 0 && malformed_event.memoized == 0
                && malformed_event.shorted == 0,
            "wrapper malformed search returned nonzero statistics");
    require(tree.search("{\"age\": 5} trailing", ids).empty(),
            "wrapper filtered search accepted trailing JSON tokens");
}

void verify_wrapper_event_api() {
    be::Tree tree;
    tree.add_boolean("flag", false)
        .add_integer("age", false, 0, 120)
        .add_string("country", false, 8)
        .add_integer_enum("status", false, 8)
        .add_string_list("labels", false, 8)
        .add_segments("seg", false)
        .add_frequency_caps("frequency_caps", false)
        .add_integer("now", false, 0, 1000);

    require(tree.insert(200, "flag and age >= 21 and country = \"USA\""),
            "event API insert failed");
    require(tree.insert(201, "status = 2 and labels is empty"),
            "event API conversion insert failed");
    require(tree.insert_with_constants(
                202,
                "segment_within(seg, 1, 20) and within_frequency_cap(\"flight\", \"ns\", 100, 0)",
                {{"flight_id", 10}}),
            "event API special insert failed");

    auto event = tree.make_event();
    event.set_boolean(0, true)
        .set_integer(1, 25)
        .set_string(2, "USA")
        .set_integer_enum(3, 2)
        .set_empty_list(4)
        .set_segments(5, {{1, 10000000}})
        .set_frequency_caps(6, {{"flight", 10, "ns", false, 0, 0}})
        .set_integer(7, 0);

    const auto json_result = tree.search(
        "{\"flag\": true, \"age\": 25, \"country\": \"USA\", \"status\": 2, \"labels\": [], "
        "\"seg\": [[1, 10000000]], \"frequency_caps\": [[\"flight\", 10, \"ns\", 0, 0]], "
        "\"now\": 0}");
    const auto event_result = tree.search(event);
    require(event_result.matched_subs == json_result.matched_subs,
            "event API search parity failed");
    require(tree.exists(event),
            "event API exists failed");

    const std::vector<std::uint64_t> filtered_ids = {200, 202};
    const auto filtered_result = tree.search(event, filtered_ids);
    require(filtered_result.size() == 2,
            "event API filtered search returned wrong count");
    require(filtered_result.matched_subs[0] == 200 && filtered_result.matched_subs[1] == 202,
            "event API filtered search returned wrong ids");

    event.set_integer(1, 19);
    const auto replaced = tree.search(event);
    require(replaced.size() == 2,
            "event API replacement search returned wrong count");
    require(replaced.matched_subs[0] == 201 && replaced.matched_subs[1] == 202,
            "event API replacement search returned wrong ids");

    const auto reused = tree.search(event);
    require(reused.matched_subs == replaced.matched_subs,
            "event API reuse after normalization failed");

    event.set_integer(1, 25);
    event.clear(0);
    const auto missing_required = tree.search(event);
    require(missing_required.empty(),
            "event API clear did not produce empty result on invalid event");
    require(!tree.exists(event),
            "event API clear did not make exists fail");
}

void verify_wrapper_event_api_rejections() {
    be::Tree tree;
    tree.add_segments("seg", false)
        .add_frequency_caps("frequency_caps", false)
        .add_integer("now", false, 0, 1000);

    require(tree.insert_with_constants(
                300,
                "segment_within(seg, 1, 20) and within_frequency_cap(\"flight\", \"ns\", 100, 0)",
                {{"flight_id", 10}}),
            "wrapper event rejection insert failed");

    auto event = tree.make_event();
    event.set_integer_list(0, {1})
        .set_string_list(1, {"bad"})
        .set_integer(2, 0);

    const auto result = tree.search(event);
    require(result.empty(),
            "wrapper event API accepted special-domain type mismatch");
    require(!tree.exists(event),
            "wrapper event API exists accepted special-domain type mismatch");

    const std::vector<std::uint64_t> ids = {300};
    const auto filtered = tree.search(event, ids);
    require(filtered.empty(),
            "wrapper event API filtered search accepted special-domain type mismatch");

    auto invalid_cap_event = tree.make_event();
    bool rejected_invalid_cap = false;
    try {
        invalid_cap_event.set_frequency_caps(1, {{"invalid", 10, "ns", false, 0, 0}});
    } catch (const be::BetreeException&) {
        rejected_invalid_cap = true;
    }
    require(rejected_invalid_cap,
            "wrapper event API accepted an invalid frequency-cap type");
}

void verify_wrapper_event_list_setter_out_of_range() {
    // Event::set_integer_list/set_string_list/set_empty_list/set_segments/
    // set_frequency_caps used to build their C-heap list/segments/caps
    // object *before* the variable_name(index) call that can throw on an
    // out-of-range index, leaking that object. This only has one variable
    // (index 0), so index 1 is guaranteed out of range for every setter.
    be::Tree tree;
    tree.add_integer_list("tags", false, 0, 100);
    auto event = tree.make_event();

    int caught = 0;

    try {
        event.set_integer_list(1, {1, 2, 3});
    } catch (const be::BetreeException&) {
        ++caught;
    }
    try {
        event.set_string_list(1, {"a", "b"});
    } catch (const be::BetreeException&) {
        ++caught;
    }
    try {
        event.set_empty_list(1);
    } catch (const be::BetreeException&) {
        ++caught;
    }
    try {
        event.set_segments(1, {{1, 10000000}});
    } catch (const be::BetreeException&) {
        ++caught;
    }
    try {
        event.set_frequency_caps(1, {{"flight", 10, "ns", false, 0, 0}});
    } catch (const be::BetreeException&) {
        ++caught;
    }

    require(caught == 5,
            "wrapper event list setters did not all reject an out-of-range index");
}

void verify_wrapper_foreign_event_rejection() {
    be::Tree source;
    source.add_integer("age", false, 0, 120)
        .add_string("country", false, 8);
    auto event = source.make_event();
    event.set_integer(0, 25).set_string(1, "USA");

    be::Tree target;
    target.add_integer("age", false, 0, 120);
    require(target.insert(400, "age >= 21"),
            "foreign event rejection insert failed");

    require(target.search(event).empty(),
            "wrapper accepted an event created by another tree");
    require(!target.exists(event),
            "wrapper exists accepted an event created by another tree");
    require(target.search(event, {400}).empty(),
            "wrapper filtered search accepted an event created by another tree");
}

void verify_wrapper_event_lifetime() {
    std::optional<be::Event> retained_event;
    {
        be::Tree owner;
        owner.add_integer("age", false, 0, 120);
        retained_event.emplace(owner.make_event());
    }
    retained_event->set_integer(0, 25).clear(0).set_integer(0, 30);
    retained_event.reset();

    be::Tree original;
    original.add_integer("age", false, 0, 120);
    require(original.insert(500, "age >= 21"),
            "event lifetime insert failed");
    auto event = original.make_event();
    be::Tree moved = std::move(original);
    event.set_integer(0, 25);
    const auto result = moved.search(event);
    require(result.size() == 1 && result.matched_subs[0] == 500,
            "event did not survive moving its owning tree");
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
    verify_wrapper_event_api();
    verify_wrapper_event_api_rejections();
    verify_wrapper_event_list_setter_out_of_range();
    verify_wrapper_foreign_event_rejection();
    verify_wrapper_event_lifetime();

    return 0;
}
