#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdlib>

#include "betree.h"

template <typename Cdir>
inline bool is_event_enclosed_shared(
    const struct betree_variable** preds, const Cdir* cdir, bool open_left, bool open_right)
{
    if(cdir == nullptr) {
        return false;
    }
    const struct betree_variable* pred = preds[cdir->attr_var.var];
    if(pred == nullptr) {
        return true;
    }
    switch(pred->value.value_type) {
        case BETREE_BOOLEAN:
            return (cdir->bound.bmin <= pred->value.boolean_value)
                && (cdir->bound.bmax >= pred->value.boolean_value);
        case BETREE_INTEGER:
            return (open_left || cdir->bound.imin <= pred->value.integer_value)
                && (open_right || cdir->bound.imax >= pred->value.integer_value);
        case BETREE_FLOAT:
            return (open_left || cdir->bound.fmin <= pred->value.float_value)
                && (open_right || cdir->bound.fmax >= pred->value.float_value);
        case BETREE_STRING:
            return (cdir->bound.smin <= pred->value.string_value.str)
                && (open_right || cdir->bound.smax >= pred->value.string_value.str);
        case BETREE_INTEGER_ENUM:
            return (cdir->bound.smin <= pred->value.integer_enum_value.ienum)
                && (open_right || cdir->bound.smax >= pred->value.integer_enum_value.ienum);
        case BETREE_INTEGER_LIST:
            if(pred->value.integer_list_value->count != 0) {
                const std::int64_t min = pred->value.integer_list_value->integers[0];
                const std::int64_t max =
                    pred->value.integer_list_value
                        ->integers[pred->value.integer_list_value->count - 1];
                const std::int64_t bound_min = open_left ? INT64_MIN : cdir->bound.imin;
                const std::int64_t bound_max = open_right ? INT64_MAX : cdir->bound.imax;
                return min <= bound_max && bound_min <= max;
            }
            return true;
        case BETREE_STRING_LIST:
            if(pred->value.string_list_value->count != 0) {
                const std::size_t min = pred->value.string_list_value->strings[0].str;
                const std::size_t max =
                    pred->value.string_list_value
                        ->strings[pred->value.string_list_value->count - 1]
                        .str;
                const std::size_t bound_min = cdir->bound.smin;
                const std::size_t bound_max
                    = open_right ? static_cast<std::size_t>(-1) : cdir->bound.smax;
                return min <= bound_max && bound_min <= max;
            }
            return true;
        case BETREE_SEGMENTS:
        case BETREE_FREQUENCY_CAPS:
            return true;
        case BETREE_UNFETCHED:
            // Value not yet known: can't rule the subtree out, so don't prune it.
            return true;
        default:
            std::abort();
    }
}

template <typename Pdir, typename SearchFn>
inline void traverse_pdir_shared(const struct attr_domain** attr_domains,
    const struct betree_variable** preds,
    const Pdir* pdir,
    SearchFn&& search_child)
{
    if(pdir == nullptr) {
        return;
    }
    for(std::size_t i = 0; i < pdir->pnode_count; i++) {
        auto* pnode = pdir->pnodes[i];
        const struct attr_domain* attr_domain = get_attr_domain(attr_domains, pnode->attr_var.var);
        if(attr_domain->allow_undefined || event_contains_variable(preds, pnode->attr_var.var)) {
            search_child(pnode);
        }
    }
}

template <typename Cdir, typename VisitFn, typename ExcludeFn>
inline void traverse_cdir_children_shared(const struct betree_variable** preds,
    Cdir* cdir,
    bool open_left,
    bool open_right,
    VisitFn&& visit_child,
    ExcludeFn&& exclude_child)
{
    if(is_event_enclosed_shared(preds, cdir->lchild, open_left, false)) {
        visit_child(cdir->lchild, open_left, false);
    }
    else {
        exclude_child(cdir->lchild);
    }
    if(is_event_enclosed_shared(preds, cdir->rchild, false, open_right)) {
        visit_child(cdir->rchild, false, open_right);
    }
    else {
        exclude_child(cdir->rchild);
    }
}
