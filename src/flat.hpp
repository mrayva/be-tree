#pragma once

#include <cstddef>
#include <cstdint>

#include "betree.h"
#include "memoize.h"

// Internal-only: serialized-tree layout and continuation state for the flat
// search path (see flat.cpp / betree_search_flat in betree.h). Nothing
// outside flat.cpp constructs or reads a flat_search_state directly -- it's
// only ever held behind report->state, opaque to callers.

enum chunk_tag {
    CHUNK_SUB = 1,
    CHUNK_PNODE = 2,
    CHUNK_CDIR = 3,
    CHUNK_END = 4,
};

struct flat_search_state {
    const struct betree_variable** preds;
    std::uint64_t* undefined;
    struct memoize memoize;
    betree_var_t* memoize_vars;
    std::size_t cursor;
};

enum flat_search_result flat_search(
    const struct betree* tree, struct flat_search_state* state, struct report* report);

void flat_search_state_free(struct flat_search_state* state);
