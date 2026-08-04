#pragma once

#include <cstddef>
#include <cstdint>

#include "betree_err.h"
#include "memoize.h"

// Internal-only: serialized-tree layout and continuation state for the
// _err flat search path (see flat_err.cpp / betree_search_flat_err in
// betree_err.h). Mirrors flat.hpp for the plain API; the chunk_tag enum
// itself is shared (see flat.hpp), only the state and CDIR payload shape
// differ, since the _err tree tracks reasons instead of running an
// optional excluded-branch callback.

struct flat_search_state_err {
    const struct betree_variable** preds;
    std::uint64_t* undefined;
    struct memoize memoize;
    betree_var_t* memoize_reason;
    std::size_t cursor;
};

enum flat_search_result flat_search_err(
    const struct betree_err* tree, struct flat_search_state_err* state, struct report_err* report);

void flat_search_state_free_err(struct flat_search_state_err* state);
