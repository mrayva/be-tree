#include <cassert>
#include <cstdint>
#include <cstring>

#include "alloc.h"
#include "ast_err.h"
#include "flat.hpp"
#include "flat_err.hpp"
#include "hashmap.h"
#include "tree.h"
#include "tree_err.h"
#include "tree_eval_shared.hpp"
#include "tree_traversal_shared.hpp"

namespace {

constexpr std::size_t FLAT_INITIAL_CAPACITY = 4096;

struct flat_builder {
    std::uint8_t* buf;
    std::size_t len;
    std::size_t capacity;
};

void builder_init(flat_builder& b)
{
    b.buf = static_cast<std::uint8_t*>(bmalloc(FLAT_INITIAL_CAPACITY));
    b.len = 0;
    b.capacity = FLAT_INITIAL_CAPACITY;
}

void builder_ensure(flat_builder& b, std::size_t needed)
{
    if(b.len + needed <= b.capacity) {
        return;
    }
    while(b.len + needed > b.capacity) {
        b.capacity *= 2;
    }
    b.buf = static_cast<std::uint8_t*>(brealloc(b.buf, b.capacity));
}

void emit_u8(flat_builder& b, std::uint8_t v)
{
    builder_ensure(b, 1);
    b.buf[b.len++] = v;
}

void emit_var(flat_builder& b, betree_var_t v)
{
    builder_ensure(b, sizeof(v));
    std::memcpy(b.buf + b.len, &v, sizeof(v));
    b.len += sizeof(v);
}

void emit_ptr(flat_builder& b, const void* p)
{
    builder_ensure(b, sizeof(p));
    std::memcpy(b.buf + b.len, &p, sizeof(p));
    b.len += sizeof(p);
}

void emit_bound(flat_builder& b, const struct value_bound& bound)
{
    builder_ensure(b, sizeof(bound));
    std::memcpy(b.buf + b.len, &bound, sizeof(bound));
    b.len += sizeof(bound);
}

std::size_t emit_skip_placeholder(flat_builder& b)
{
    std::size_t offset = b.len;
    builder_ensure(b, 4);
    std::memset(b.buf + b.len, 0, 4);
    b.len += 4;
    return offset;
}

void patch_skip_length(flat_builder& b, std::size_t offset)
{
    std::uint32_t skip = static_cast<std::uint32_t>(b.len - offset);
    std::memcpy(b.buf + offset, &skip, 4);
}

void emit_sub(flat_builder& b, const struct betree_sub* sub)
{
    emit_u8(b, CHUNK_SUB);
    emit_ptr(b, sub);
}

std::size_t emit_pnode_header(flat_builder& b, const struct pnode_err* pn, const struct config* config)
{
    emit_u8(b, CHUNK_PNODE);
    std::size_t skip_offset = emit_skip_placeholder(b);
    emit_var(b, pn->attr_var.var);
    const struct attr_domain* ad = config->attr_domains[pn->attr_var.var];
    emit_u8(b, ad->allow_undefined ? 1 : 0);
    return skip_offset;
}

std::size_t emit_cdir_header(
    flat_builder& b, const struct cdir_err* cdir, bool open_left, bool open_right)
{
    emit_u8(b, CHUNK_CDIR);
    std::size_t skip_offset = emit_skip_placeholder(b);
    emit_var(b, cdir->attr_var.var);
    emit_u8(b, open_left ? 1 : 0);
    emit_u8(b, open_right ? 1 : 0);
    emit_bound(b, cdir->bound);
    // Unlike the plain tree's cdir (a raw void**/count pair meant for an
    // optional caller callback), sub_ids is a single self-describing
    // dynamic_array_t* already built by betree_make_sub_ids -- reason
    // attribution for an excluded branch reads it directly at search time.
    emit_ptr(b, cdir->sub_ids);
    return skip_offset;
}

void emit_end(flat_builder& b)
{
    emit_u8(b, CHUNK_END);
}

void flatten_cdir_err(
    const struct cdir_err* cdir, flat_builder& b, const struct config* config, bool open_left, bool open_right);

void flatten_cnode_err(const struct cnode_err* cnode, flat_builder& b, const struct config* config)
{
    if(cnode->lnode != nullptr) {
        for(std::size_t i = 0; i < cnode->lnode->sub_count; i++) {
            emit_sub(b, cnode->lnode->subs[i]);
        }
    }
    if(cnode->pdir != nullptr) {
        for(std::size_t i = 0; i < cnode->pdir->pnode_count; i++) {
            struct pnode_err* pn = cnode->pdir->pnodes[i];
            std::size_t skip_offset = emit_pnode_header(b, pn, config);
            if(pn->cdir != nullptr) {
                flatten_cdir_err(pn->cdir, b, config, true, true);
            }
            patch_skip_length(b, skip_offset);
        }
    }
    emit_end(b);
}

void flatten_cdir_err(
    const struct cdir_err* cdir, flat_builder& b, const struct config* config, bool open_left, bool open_right)
{
    std::size_t skip_offset = emit_cdir_header(b, cdir, open_left, open_right);
    flatten_cnode_err(cdir->cnode, b, config);
    if(cdir->lchild != nullptr) {
        flatten_cdir_err(cdir->lchild, b, config, open_left, false);
    }
    if(cdir->rchild != nullptr) {
        flatten_cdir_err(cdir->rchild, b, config, false, open_right);
    }
    patch_skip_length(b, skip_offset);
}

// --- Flat search ---

inline std::uint8_t read_u8(const std::uint8_t* buf, std::size_t* cur)
{
    std::uint8_t v = buf[*cur];
    *cur += 1;
    return v;
}

inline std::uint32_t read_u32(const std::uint8_t* buf, std::size_t* cur)
{
    std::uint32_t v;
    std::memcpy(&v, buf + *cur, 4);
    *cur += 4;
    return v;
}

inline betree_var_t read_var(const std::uint8_t* buf, std::size_t* cur)
{
    betree_var_t v;
    std::memcpy(&v, buf + *cur, sizeof(v));
    *cur += sizeof(v);
    return v;
}

inline void* read_ptr(const std::uint8_t* buf, std::size_t* cur)
{
    void* p;
    std::memcpy(&p, buf + *cur, sizeof(p));
    *cur += sizeof(p);
    return p;
}

inline struct value_bound read_bound(const std::uint8_t* buf, std::size_t* cur)
{
    struct value_bound bound;
    std::memcpy(&bound, buf + *cur, sizeof(bound));
    *cur += sizeof(bound);
    return bound;
}

void update_state_preds_err(struct flat_search_state_err* state, const struct betree_event* event)
{
    for(std::size_t i = 0; i < event->variable_count; i++) {
        struct betree_variable* pred = event->variables[i];
        if(pred == nullptr) {
            if(state->preds[i] == &BETREE_PRED_UNFETCHED) {
                state->preds[i] = nullptr;
                if(state->undefined != nullptr) {
                    set_bit(state->undefined, i);
                }
            }
            continue;
        }
        if(pred->value.value_type == BETREE_UNFETCHED) {
            continue;
        }
        betree_var_t var = pred->attr_var.var;
        state->preds[var] = pred;
        if(state->undefined != nullptr) {
            clear_bit(state->undefined, var);
        }
    }
}

} // namespace

extern "C" {

void betree_flatten_err(struct betree_err* tree)
{
    assert(tree->cnode != nullptr);
    if(tree->flat.buf != nullptr) {
        return;
    }

    flat_builder b;
    builder_init(b);
    flatten_cnode_err(tree->cnode, b, tree->config);

    tree->flat.buf = b.buf;
    tree->flat.len = b.len;
}

void betree_free_flat_err(struct flat_tree* ft)
{
    if(ft->buf != nullptr) {
        bfree(ft->buf);
        ft->buf = nullptr;
        ft->len = 0;
    }
}

} // extern "C"

enum flat_search_result flat_search_err(
    const struct betree_err* tree, struct flat_search_state_err* state, struct report_err* report)
{
    const std::uint8_t* buf = tree->flat.buf;
    const std::size_t len = tree->flat.len;
    std::size_t cur = state->cursor;
    const struct betree_variable** preds = state->preds;
    const std::size_t dom_cnt = tree->config->attr_domain_count;

    while(cur < len) {
        std::uint8_t tag = buf[cur];
        cur++;

        switch(tag) {
            case CHUNK_SUB: {
                const auto* sub = static_cast<const struct betree_sub*>(read_ptr(buf, &cur));

                report->evaluated++;
                betree_var_t last_reason = INVALID_VAR;
                enum match_result res = match_sub_tri_err(dom_cnt, preds, sub, report,
                    &state->memoize, state->undefined, &last_reason, state->memoize_reason);
                if(res == MATCH_UNKNOWN) {
                    state->cursor = cur - 1 - sizeof(void*);
                    return FLAT_SEARCH_YIELD;
                }
                if(res == MATCH_TRUE) {
                    append_report_sub_id(sub->id, report);
                }
                else {
                    betree_reason_map_additem(report->reason_sub_id_list, last_reason, sub->id);
                }
                break;
            }
            case CHUNK_PNODE: {
                std::size_t skip_offset = cur;
                std::uint32_t skip_length = read_u32(buf, &cur);
                betree_var_t var_id = read_var(buf, &cur);
                std::uint8_t allow_undefined = read_u8(buf, &cur);

                if(preds[var_id] == &BETREE_PRED_UNFETCHED) {
                    state->cursor = skip_offset - 1;
                    return FLAT_SEARCH_YIELD;
                }
                if(!allow_undefined && preds[var_id] == nullptr) {
                    cur = skip_offset + skip_length;
                }
                break;
            }
            case CHUNK_CDIR: {
                std::size_t skip_offset = cur;
                std::uint32_t skip_length = read_u32(buf, &cur);
                betree_var_t var_id = read_var(buf, &cur);
                std::uint8_t open_left = read_u8(buf, &cur);
                std::uint8_t open_right = read_u8(buf, &cur);
                struct value_bound bound = read_bound(buf, &cur);
                auto* sub_ids = static_cast<dynamic_array_t*>(read_ptr(buf, &cur));

                // See flat.cpp: is_event_enclosed_shared only reads
                // .attr_var.var and .bound, so a stack-local synthetic
                // cdir_err carrying just those two is enough to reuse it.
                struct cdir_err synthetic {};
                synthetic.attr_var.var = var_id;
                synthetic.bound = bound;
                if(!is_event_enclosed_shared(preds, &synthetic, open_left != 0, open_right != 0)) {
                    if(sub_ids != nullptr && sub_ids->size > 0) {
                        betree_reason_map_join(report->reason_sub_id_list, var_id, sub_ids);
                    }
                    cur = skip_offset + skip_length;
                }
                break;
            }
            case CHUNK_END:
                break;
            default:
                assert(false && "invalid chunk tag");
                break;
        }
    }

    state->cursor = cur;
    return FLAT_SEARCH_DONE;
}

extern "C" {

enum flat_search_result betree_search_flat_err(
    struct betree_err* tree, struct betree_event* event, struct report_err* report)
{
    assert(tree->flat.buf != nullptr);
    const struct config* config = tree->config;
    std::size_t dom_cnt = config->attr_domain_count;

    fill_event(config, event);
    sort_event_lists(event);

    struct flat_search_state_err* state = report->state;

    if(state == nullptr) {
        std::size_t pred_count = config->pred_map->memoize_count;
        state = static_cast<struct flat_search_state_err*>(bmalloc(sizeof(*state)));
        state->preds = make_environment(dom_cnt, event);
        state->undefined = make_undefined(dom_cnt, state->preds);
        state->memoize = make_memoize(pred_count);
        state->memoize_reason = pred_count > 0
            ? static_cast<betree_var_t*>(bmalloc(pred_count * sizeof(*state->memoize_reason)))
            : nullptr;
        state->cursor = 0;
    }
    else {
        update_state_preds_err(state, event);
    }

    enum flat_search_result res = flat_search_err(tree, state, report);

    if(res == FLAT_SEARCH_DONE) {
        flat_search_state_free_err(state);
        report->state = nullptr;
    }
    else {
        report->state = state;
    }
    return res;
}

} // extern "C"

void flat_search_state_free_err(struct flat_search_state_err* state)
{
    if(state == nullptr) {
        return;
    }
    bfree(const_cast<void*>(static_cast<const void*>(state->preds)));
    if(state->undefined != nullptr) {
        bfree(state->undefined);
    }
    free_memoize(state->memoize);
    if(state->memoize_reason != nullptr) {
        bfree(state->memoize_reason);
    }
    bfree(state);
}
