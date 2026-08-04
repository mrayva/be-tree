#include <cctype>
#include <cstdarg>
#include <cfloat>
#include <cinttypes>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <optional>

#include "alloc.h"
#include "ast.h"
#include "betree.h"
#include "error.h"
#include "hashmap.h"
#include "memoize.h"
#include "printer.h"
#include "tree.h"
#include "tree_eval_shared.hpp"
#include "tree_match_shared.hpp"
#include "tree_traversal_shared.hpp"
#include "utils.h"

// Forward declarations for functions from other C modules
extern "C" {
    int parse(const char* text, struct ast_node** node);
    int event_parse(const char* text, struct betree_event** event);
}

const struct betree_variable BETREE_PRED_UNFETCHED = {};

static void search_cdir_ids(const struct attr_domain** attr_domains,
    const struct betree_variable** preds,
    struct cdir* cdir,
    struct subs_to_eval* subs,
    bool open_left,
    bool open_right,
    const std::uint64_t* ids,
    std::size_t sz);

extern "C" {

void init_subs_to_eval(struct subs_to_eval* subs)
{
    std::size_t init = 10;
    subs->subs = static_cast<struct betree_sub**>(bmalloc(init * sizeof(struct betree_sub*)));
    subs->capacity = init;
    subs->count = 0;
}

void init_subs_to_eval_ext(struct subs_to_eval* subs, std::size_t init)
{
    subs->subs = static_cast<struct betree_sub**>(bmalloc(init * sizeof(struct betree_sub*)));
    subs->capacity = init;
    subs->count = 0;
}

} // extern "C"

extern "C" {

bool match_sub(std::size_t attr_domains_count,
    const struct betree_variable** preds,
    const struct betree_sub* sub,
    struct report* report,
    struct memoize* memoize,
    const std::uint64_t* undefined)
{
    auto short_circuit =
        try_short_circuit(attr_domains_count, &sub->short_circuit, undefined);
    if(short_circuit != SHORT_CIRCUIT_NONE) {
        if(report != nullptr) {
            report->shorted++;
        }
        if(short_circuit == SHORT_CIRCUIT_PASS) {
            return true;
        }
        if(short_circuit == SHORT_CIRCUIT_FAIL) {
            return false;
        }
    }
    bool result = match_node(preds, sub->expr, memoize, report);
    return result;
}

bool match_sub_(std::size_t attr_domains_count,
    const struct betree_variable** preds,
    const struct betree_sub* sub,
    struct report* report,
    struct memoize* memoize,
    const std::uint64_t* undefined)
{
    auto short_circuit =
        try_short_circuit_(attr_domains_count, &sub->short_circuit, undefined, &report->last_var);
    if(short_circuit != SHORT_CIRCUIT_NONE) {
        if(report != nullptr) {
            report->shorted++;
        }
        if(short_circuit == SHORT_CIRCUIT_PASS) {
            return true;
        }
        if(short_circuit == SHORT_CIRCUIT_FAIL) {
            return false;
        }
    }
    bool result = match_node(preds, sub->expr, memoize, report);
    return result;
}

enum match_result match_sub_tri(std::size_t attr_domains_count,
    const struct betree_variable** preds,
    const struct betree_sub* sub,
    struct report* report,
    struct memoize* memoize,
    const std::uint64_t* undefined)
{
    auto short_circuit =
        try_short_circuit_(attr_domains_count, &sub->short_circuit, undefined, &report->last_var);
    if(short_circuit != SHORT_CIRCUIT_NONE) {
        if(report != nullptr) {
            report->shorted++;
        }
        if(short_circuit == SHORT_CIRCUIT_PASS) {
            return MATCH_TRUE;
        }
        if(short_circuit == SHORT_CIRCUIT_FAIL) {
            return MATCH_FALSE;
        }
    }
    return match_node_tri(preds, sub->expr, memoize, report);
}

bool match_sub_counting(std::size_t attr_domains_count,
    const struct betree_variable** preds,
    const struct betree_sub* sub,
    struct report_counting* report,
    struct memoize* memoize,
    const std::uint64_t* undefined)
{
    auto short_circuit
        = try_short_circuit(attr_domains_count, &sub->short_circuit, undefined);
    if(short_circuit != SHORT_CIRCUIT_NONE) {
        if(report != nullptr) {
            report->shorted++;
        }
        if(short_circuit == SHORT_CIRCUIT_PASS) {
            return true;
        }
        if(short_circuit == SHORT_CIRCUIT_FAIL) {
            return false;
        }
    }
    bool result = match_node_counting(preds, sub->expr, memoize, report);
    return result;
}

} // extern "C"

static void check_sub(const struct lnode* lnode, struct subs_to_eval* subs)
{
    for(std::size_t i = 0; i < lnode->sub_count; i++) {
        struct betree_sub* sub = lnode->subs[i];
        add_sub_to_eval(sub, subs);
    }
}

static void check_sub_ids(
    const struct lnode* lnode, struct subs_to_eval* subs, const std::uint64_t* ids, std::size_t sz)
{
    for(std::size_t i = 0; i < lnode->sub_count; i++) {
        struct betree_sub* sub = lnode->subs[i];
        if(is_id_in(sub->id, ids, sz)) {
            add_sub_to_eval(sub, subs);
        }
    }
}

static void check_sub_node_counting(
    const struct lnode* lnode, struct subs_to_eval* subs, int* node_count)
{
    for(std::size_t i = 0; i < lnode->sub_count; i++) {
        struct betree_sub* sub = lnode->subs[i];
        add_sub_to_eval(sub, subs);
        ++*node_count;
    }
}

static struct pnode* search_pdir(betree_var_t variable_id, const struct pdir* pdir)
{
    if(pdir == nullptr) {
        return nullptr;
    }
    for(std::size_t i = 0; i < pdir->pnode_count; i++) {
        struct pnode* pnode = pdir->pnodes[i];
        if(variable_id == pnode->attr_var.var) {
            return pnode;
        }
    }
    return nullptr;
}

static void search_cdir(const struct config* config,
    const struct betree_variable** preds,
    struct cdir* cdir,
    struct subs_to_eval* subs,
    bool open_left,
    bool open_right,
    struct report* report);

static void search_cdir_node_counting(const struct attr_domain** attr_domains,
    const struct betree_variable** preds,
    struct cdir* cdir,
    struct subs_to_eval* subs,
    bool open_left,
    bool open_right,
    int* node_count);

extern "C" {

void match_be_tree(const struct config* config,
    const struct betree_variable** preds,
    const struct cnode* cnode,
    struct subs_to_eval* subs,
    struct report* report)
{
    check_sub(cnode->lnode, subs);
    traverse_pdir_shared((const struct attr_domain**)config->attr_domains, preds, cnode->pdir,
        [&](struct pnode* pnode) { search_cdir(config, preds, pnode->cdir, subs, true, true, report); });
}

} // extern "C"

static void match_be_tree_ids(const struct attr_domain** attr_domains,
    const struct betree_variable** preds,
    const struct cnode* cnode,
    struct subs_to_eval* subs,
    const std::uint64_t* ids,
    std::size_t sz)
{
    check_sub_ids(cnode->lnode, subs, ids, sz);
    traverse_pdir_shared(attr_domains, preds, cnode->pdir, [&](struct pnode* pnode) {
        search_cdir_ids(attr_domains, preds, pnode->cdir, subs, true, true, ids, sz);
    });
}

extern "C" {

void match_be_tree_node_counting(const struct attr_domain** attr_domains,
    const struct betree_variable** preds,
    const struct cnode* cnode,
    struct subs_to_eval* subs,
    int* node_count)
{
    check_sub_node_counting(cnode->lnode, subs, node_count);
    traverse_pdir_shared(attr_domains, preds, cnode->pdir, [&](struct pnode* pnode) {
        search_cdir_node_counting(attr_domains, preds, pnode->cdir, subs, true, true, node_count);
        ++*node_count;
    });
    if(cnode->pdir != nullptr) {
        ++*node_count;
    }
}

} // extern "C"

extern "C" {

bool sub_is_enclosed(
    const struct attr_domain** attr_domains, const struct betree_sub* sub, const struct cdir* cdir)
{
    if(cdir == nullptr) {
        return false;
    }
    if(test_bit(sub->attr_vars, cdir->attr_var.var) == true) {
        const struct attr_domain* attr_domain = get_attr_domain(attr_domains, cdir->attr_var.var);
        struct value_bound bound = get_variable_bound(attr_domain, sub->expr);
        switch(attr_domain->bound.value_type) {
            case(BETREE_INTEGER):
            case(BETREE_INTEGER_LIST):
                return cdir->bound.imin <= bound.imin && cdir->bound.imax >= bound.imax;
            case(BETREE_FLOAT): {
                return cdir->bound.fmin <= bound.fmin && cdir->bound.fmax >= bound.fmax;
            }
            case(BETREE_BOOLEAN): {
                return cdir->bound.bmin <= bound.bmin && cdir->bound.bmax >= bound.bmax;
            }
            case(BETREE_STRING):
            case(BETREE_STRING_LIST):
            case(BETREE_INTEGER_ENUM):
                return cdir->bound.smin <= bound.smin && cdir->bound.smax >= bound.smax;
            case(BETREE_SEGMENTS): {
                std::fprintf(stderr, "%s a segments value cdir should never happen for now\n", __func__);
                std::abort();
            }
            case(BETREE_FREQUENCY_CAPS): {
                std::fprintf(
                    stderr, "%s a frequency value cdir should never happen for now\n", __func__);
                std::abort();
            }
            default:
                std::abort();
        }
    }
    return false;
}

} // extern "C"

static inline void exclude_cdir(struct cdir* cdir, struct report* report) {
    if (cdir == nullptr || report == nullptr || report->cba == nullptr) return;
    assert(cdir->subs_data_array != nullptr);
    (*report->cba)(report->arg, cdir->subs_data_array, cdir->subs_data_count, (void*)cdir->attr_var.var);
}

static void search_cdir(const struct config* config,
    const struct betree_variable** preds,
    struct cdir* cdir,
    struct subs_to_eval* subs,
    bool open_left,
    bool open_right,
    struct report* report)
{
    match_be_tree(config, preds, cdir->cnode, subs, report);
    traverse_cdir_children_shared(
        preds,
        cdir,
        open_left,
        open_right,
        [&](struct cdir* child, bool child_open_left, bool child_open_right) {
            search_cdir(config, preds, child, subs, child_open_left, child_open_right, report);
        },
        [&](struct cdir* child) { exclude_cdir(child, report); });
}

static void search_cdir_ids(const struct attr_domain** attr_domains,
    const struct betree_variable** preds,
    struct cdir* cdir,
    struct subs_to_eval* subs,
    bool open_left,
    bool open_right,
    const std::uint64_t* ids,
    std::size_t sz)
{
    match_be_tree_ids(attr_domains, preds, cdir->cnode, subs, ids, sz);
    traverse_cdir_children_shared(
        preds,
        cdir,
        open_left,
        open_right,
        [&](struct cdir* child, bool child_open_left, bool child_open_right) {
            search_cdir_ids(
                attr_domains, preds, child, subs, child_open_left, child_open_right, ids, sz);
        },
        [](struct cdir*) {});
}

static void search_cdir_node_counting(const struct attr_domain** attr_domains,
    const struct betree_variable** preds,
    struct cdir* cdir,
    struct subs_to_eval* subs,
    bool open_left,
    bool open_right,
    int* node_count)
{
    match_be_tree_node_counting(attr_domains, preds, cdir->cnode, subs, node_count);
    traverse_cdir_children_shared(
        preds,
        cdir,
        open_left,
        open_right,
        [&](struct cdir* child, bool child_open_left, bool child_open_right) {
            search_cdir_node_counting(
                attr_domains, preds, child, subs, child_open_left, child_open_right, node_count);
        },
        [](struct cdir*) {});
}

static bool is_used_cnode(betree_var_t variable_id, const struct cnode* cnode);

static bool is_used_pdir(betree_var_t variable_id, const struct pdir* pdir)
{
    if(pdir == nullptr || pdir->parent == nullptr) {
        return false;
    }
    return is_used_cnode(variable_id, pdir->parent);
}

static bool is_used_pnode(betree_var_t variable_id, const struct pnode* pnode)
{
    if(pnode == nullptr || pnode->parent == nullptr) {
        return false;
    }
    if(pnode->attr_var.var == variable_id) {
        return true;
    }
    return is_used_pdir(variable_id, pnode->parent);
}

static bool is_used_cdir(betree_var_t variable_id, const struct cdir* cdir)
{
    if(cdir == nullptr) {
        return false;
    }
    if(cdir->attr_var.var == variable_id) {
        return true;
    }
    switch(cdir->parent_type) {
        case CNODE_PARENT_PNODE: {
            return is_used_pnode(variable_id, cdir->pnode_parent);
        }
        case CNODE_PARENT_CDIR: {
            return is_used_cdir(variable_id, cdir->cdir_parent);
        }
        default:
            std::abort();
    }
}

static bool is_used_cnode(betree_var_t variable_id, const struct cnode* cnode)
{
    if(cnode == nullptr) {
        return false;
    }
    if(cnode->parent == nullptr) {
        return false;
    }
    return is_used_cdir(variable_id, cnode->parent);
}

static void insert_sub(const struct betree_sub* sub, struct lnode* lnode)
{
    if(lnode->sub_count == 0) {
        lnode->subs = static_cast<struct betree_sub**>(bcalloc(sizeof(struct betree_sub*)));
        if(lnode->subs == nullptr) {
            std::fprintf(stderr, "%s bcalloc failed\n", __func__);
            std::abort();
        }
    }
    else {
        auto subs = static_cast<struct betree_sub**>(brealloc(lnode->subs, sizeof(struct betree_sub*) * (lnode->sub_count + 1)));
        if(subs == nullptr) {
            std::fprintf(stderr, "%s brealloc failed\n", __func__);
            std::abort();
        }
        lnode->subs = subs;
    }
    lnode->subs[lnode->sub_count] = (struct betree_sub*)sub;
    lnode->sub_count++;
}

static bool is_root(const struct cnode* cnode)
{
    if(cnode == nullptr) {
        return false;
    }
    return cnode->parent == nullptr;
}

static void space_partitioning(const struct config* config, struct cnode* cnode);
static void space_clustering(const struct config* config, struct cdir* cdir);
static struct cdir* insert_cdir(
    const struct config* config, const struct betree_sub* sub, struct cdir* cdir);

static std::size_t count_attr_in_lnode(betree_var_t variable_id, const struct lnode* lnode);

static std::size_t count_attr_in_cdir(betree_var_t variable_id, const struct cdir* cdir)
{
    if(cdir == nullptr) {
        return 0;
    }
    std::size_t count = 0;
    if(cdir->cnode != nullptr) {
        count += count_attr_in_lnode(variable_id, cdir->cnode->lnode);
    }
    count += count_attr_in_cdir(variable_id, cdir->lchild);
    count += count_attr_in_cdir(variable_id, cdir->rchild);
    return count;
}

static std::size_t domain_bound_diff(const struct attr_domain* attr_domain)
{
    const struct value_bound* b = &attr_domain->bound;
    switch(b->value_type) {
        case BETREE_BOOLEAN:
            return 1;
        case BETREE_INTEGER:
        case BETREE_INTEGER_LIST:
            if(b->imin == INT64_MIN && b->imax == INT64_MAX) {
                return SIZE_MAX;
            }
            else {
                return static_cast<std::size_t>(llabs(b->imax - b->imin));
            }
        case BETREE_FLOAT:
            if(feq(b->fmin, -DBL_MAX) && feq(b->fmax, DBL_MAX)) {
                return SIZE_MAX;
            }
            else {
                return static_cast<std::size_t>(fabs(b->fmax - b->fmin));
            }
        case BETREE_STRING:
        case BETREE_STRING_LIST:
        case BETREE_INTEGER_ENUM:
            return b->smax - b->smin;
        case BETREE_SEGMENTS:
        case BETREE_FREQUENCY_CAPS:
        default:
            std::abort();
    }
}

static double get_attr_domain_score(const struct attr_domain* attr_domain)
{
    std::size_t diff = domain_bound_diff(attr_domain);
    if(diff < 1) {
        diff = 1;
    }
    double num = attr_domain->allow_undefined ? 1. : 10.;
    double bound_score = num / static_cast<double>(diff);
    return bound_score;
}

static double get_score(const struct attr_domain** attr_domains, betree_var_t var, std::size_t count)
{
    const struct attr_domain* attr_domain = get_attr_domain(attr_domains, var);
    double attr_domain_score = get_attr_domain_score(attr_domain);
    double score = static_cast<double>(count) * attr_domain_score;
    return score + 10000 * attr_domain->rank;
}

static double get_pnode_score(const struct attr_domain** attr_domains, struct pnode* pnode)
{
    std::size_t count = count_attr_in_cdir(pnode->attr_var.var, pnode->cdir);
    return get_score(attr_domains, pnode->attr_var.var, count);
}

static double get_lnode_score(
    const struct attr_domain** attr_domains, const struct lnode* lnode, betree_var_t var)
{
    std::size_t count = count_attr_in_lnode(var, lnode);
    return get_score(attr_domains, var, count);
}

static void update_partition_score(const struct attr_domain** attr_domains, struct pnode* pnode)
{
    pnode->score = get_pnode_score(attr_domains, pnode);
}

extern "C" {

bool insert_be_tree(const struct config* config,
    const struct betree_sub* sub,
    struct cnode* cnode,
    struct cdir* cdir)
{
    if(config == nullptr) {
        std::fprintf(stderr, "Config is NULL, required to insert in the be tree\n");
        std::abort();
    }
    bool foundPartition = false;
    struct pnode* max_pnode = nullptr;
    if(cnode->pdir != nullptr) {
        float max_score = -DBL_MAX;
        for(std::size_t i = 0; i < config->attr_domain_count; i++) {
            if(test_bit(sub->attr_vars, i) == false) {
                continue;
            }
            betree_var_t variable_id = i;
            if(!is_used_cnode(variable_id, cnode)) {
                struct pnode* pnode = search_pdir(variable_id, cnode->pdir);
                if(pnode != nullptr) {
                    foundPartition = true;
                    if(max_score < pnode->score) {
                        max_pnode = pnode;
                        max_score = pnode->score;
                    }
                }
            }
        }
    }
    if(!foundPartition) {
        insert_sub(sub, cnode->lnode);
        if(is_root(cnode)) {
            space_partitioning(config, cnode);
        }
        else {
            space_clustering(config, cdir);
        }
    }
    else {
        struct cdir* maxCdir = insert_cdir(config, sub, max_pnode->cdir);
        insert_be_tree(config, sub, maxCdir->cnode, maxCdir);
        update_partition_score((const struct attr_domain**)config->attr_domains, max_pnode);
    }
    return true;
}

} // extern "C"

static bool is_leaf(const struct cdir* cdir)
{
    return cdir->lchild == nullptr && cdir->rchild == nullptr;
}

static struct cdir* insert_cdir(
    const struct config* config, const struct betree_sub* sub, struct cdir* cdir)
{
    if(is_leaf(cdir)) {
        return cdir;
    }
    if(sub_is_enclosed((const struct attr_domain**)config->attr_domains, sub, cdir->lchild)) {
        return insert_cdir(config, sub, cdir->lchild);
    }
    if(sub_is_enclosed((const struct attr_domain**)config->attr_domains, sub, cdir->rchild)) {
        return insert_cdir(config, sub, cdir->rchild);
    }
    return cdir;
}

static bool is_overflowed(const struct lnode* lnode)
{
    return lnode->sub_count > lnode->max;
}

extern "C" {

bool sub_has_attribute(const struct betree_sub* sub, betree_var_t variable_id)
{
    return test_bit(sub->attr_vars, variable_id);
}

bool sub_has_attribute_str(struct config* config, const struct betree_sub* sub, const char* attr)
{
    betree_var_t variable_id = try_get_id_for_attr(config, attr);
    if(variable_id == INVALID_VAR) {
        return false;
    }
    return sub_has_attribute(sub, variable_id);
}

} // extern "C"

static bool remove_sub(betree_sub_t sub, struct lnode* lnode)
{
    for(std::size_t i = 0; i < lnode->sub_count; i++) {
        const struct betree_sub* lnode_sub = lnode->subs[i];
        if(sub == lnode_sub->id) {
            for(std::size_t j = i; j < lnode->sub_count - 1; j++) {
                lnode->subs[j] = lnode->subs[j + 1];
            }
            lnode->sub_count--;
            if(lnode->sub_count == 0) {
                bfree(lnode->subs);
                lnode->subs = nullptr;
            }
            else {
                auto subs
                    = static_cast<struct betree_sub**>(brealloc(lnode->subs, sizeof(struct betree_sub*) * lnode->sub_count));
                if(subs == nullptr) {
                    std::fprintf(stderr, "%s brealloc failed\n", __func__);
                    std::abort();
                }
                lnode->subs = subs;
            }
            return true;
        }
    }
    return false;
}

static void move(const struct betree_sub* sub, struct lnode* origin, struct lnode* destination)
{
    bool isFound = remove_sub(sub->id, origin);
    if(!isFound) {
        std::fprintf(stderr, "Could not find sub %" PRIu64 "\n", sub->id);
        std::abort();
    }
    if(destination->sub_count == 0) {
        destination->subs = static_cast<struct betree_sub**>(bcalloc(sizeof(struct betree_sub*)));
        if(destination->subs == nullptr) {
            std::fprintf(stderr, "%s bcalloc failed\n", __func__);
            std::abort();
        }
    }
    else {
        auto subs = static_cast<struct betree_sub**>(brealloc(
            destination->subs, sizeof(struct betree_sub*) * (destination->sub_count + 1)));
        if(subs == nullptr) {
            std::fprintf(stderr, "%s brealloc failed\n", __func__);
            std::abort();
        }
        destination->subs = subs;
    }
    destination->subs[destination->sub_count] = (struct betree_sub*)sub;
    destination->sub_count++;
}

static struct cdir* create_cdir(const struct config* config,
    const char* attr,
    betree_var_t variable_id,
    struct value_bound bound)
{
    auto cdir = static_cast<struct cdir*>(bcalloc(sizeof(struct cdir)));
    if(cdir == nullptr) {
        std::fprintf(stderr, "%s bcalloc failed\n", __func__);
        std::abort();
    }
    cdir->attr_var.attr = bstrdup(attr);
    cdir->attr_var.var = variable_id;
    cdir->bound = bound;
    cdir->cnode = make_cnode(config, cdir);
    cdir->lchild = nullptr;
    cdir->rchild = nullptr;
    return cdir;
}

static struct cdir* create_cdir_with_cdir_parent(
    const struct config* config, struct cdir* parent, struct value_bound bound)
{
    struct cdir* cdir = create_cdir(config, parent->attr_var.attr, parent->attr_var.var, bound);
    cdir->parent_type = CNODE_PARENT_CDIR;
    cdir->cdir_parent = parent;
    return cdir;
}

static struct cdir* create_cdir_with_pnode_parent(
    const struct config* config, struct pnode* parent, struct value_bound bound)
{
    struct cdir* cdir = create_cdir(config, parent->attr_var.attr, parent->attr_var.var, bound);
    cdir->parent_type = CNODE_PARENT_PNODE;
    cdir->pnode_parent = parent;
    return cdir;
}

extern "C" {

struct pnode* create_pdir(
    const struct config* config, const char* attr, betree_var_t variable_id, struct cnode* cnode)
{
    if(cnode == nullptr) {
        std::fprintf(stderr, "cnode is NULL, cannot create a pdir and pnode\n");
        std::abort();
    }
    struct pdir* pdir = cnode->pdir;
    if(cnode->pdir == nullptr) {
        pdir = static_cast<struct pdir*>(bcalloc(sizeof(struct pdir)));
        if(pdir == nullptr) {
            std::fprintf(stderr, "%s pdir bcalloc failed\n", __func__);
            std::abort();
        }
        pdir->parent = cnode;
        pdir->pnode_count = 0;
        pdir->pnodes = nullptr;
        cnode->pdir = pdir;
    }

    auto pnode = static_cast<struct pnode*>(bcalloc(sizeof(struct pnode)));
    if(pnode == nullptr) {
        std::fprintf(stderr, "%s pnode bcalloc failed\n", __func__);
        std::abort();
    }
    pnode->cdir = nullptr;
    pnode->parent = pdir;
    pnode->attr_var.attr = bstrdup(attr);
    pnode->attr_var.var = variable_id;
    pnode->score = 0.f;
    struct value_bound bound;
    bool found = false;
    for(std::size_t i = 0; i < config->attr_domain_count; i++) {
        const struct attr_domain* attr_domain = config->attr_domains[i];
        if(attr_domain->attr_var.var == variable_id) {
            bound = attr_domain->bound;
            found = true;
            break;
        }
    }
    if(!found) {
        std::fprintf(stderr, "No domain definition for attr %" PRIu64 " in config\n", variable_id);
        std::abort();
    }
    pnode->cdir = create_cdir_with_pnode_parent(config, pnode, bound);

    if(pdir->pnode_count == 0) {
        pdir->pnodes = static_cast<struct pnode**>(bcalloc(sizeof(struct pnode*)));
        if(pdir->pnodes == nullptr) {
            std::fprintf(stderr, "%s pnodes bcalloc failed\n", __func__);
            std::abort();
        }
    }
    else {
        auto pnodes = static_cast<struct pnode**>(brealloc(pdir->pnodes, sizeof(struct pnode*) * (pdir->pnode_count + 1)));
        if(pnodes == nullptr) {
            std::fprintf(stderr, "%s brealloc failed\n", __func__);
            std::abort();
        }
        pdir->pnodes = pnodes;
    }
    pdir->pnodes[pdir->pnode_count] = pnode;
    pdir->pnode_count++;
    return pnode;
}

} // extern "C"

static std::size_t count_attr_in_lnode(betree_var_t variable_id, const struct lnode* lnode)
{
    std::size_t count = 0;
    if(lnode == nullptr) {
        return count;
    }
    for(std::size_t i = 0; i < lnode->sub_count; i++) {
        const struct betree_sub* sub = lnode->subs[i];
        if(sub == nullptr) {
            std::fprintf(stderr, "%s, sub is NULL\n", __func__);
            continue;
        }
        if(test_bit(sub->attr_vars, variable_id) == true) {
            count++;
        }
    }
    return count;
}

static bool is_attr_used_in_parent_cnode(betree_var_t variable_id, const struct cnode* cnode);

static bool is_attr_used_in_parent_pdir(betree_var_t variable_id, const struct pdir* pdir)
{
    return is_attr_used_in_parent_cnode(variable_id, pdir->parent);
}

static bool is_attr_used_in_parent_pnode(betree_var_t variable_id, const struct pnode* pnode)
{
    if(pnode->attr_var.var == variable_id) {
        return true;
    }
    return is_attr_used_in_parent_pdir(variable_id, pnode->parent);
}

static bool is_attr_used_in_parent_cdir(betree_var_t variable_id, const struct cdir* cdir)
{
    if(cdir->attr_var.var == variable_id) {
        return true;
    }
    switch(cdir->parent_type) {
        case(CNODE_PARENT_CDIR): {
            return is_attr_used_in_parent_cdir(variable_id, cdir->cdir_parent);
        }
        case(CNODE_PARENT_PNODE): {
            return is_attr_used_in_parent_pnode(variable_id, cdir->pnode_parent);
        }
        default:
            std::abort();
    }
}

static bool is_attr_used_in_parent_cnode(betree_var_t variable_id, const struct cnode* cnode)
{
    if(is_root(cnode)) {
        return false;
    }
    return is_attr_used_in_parent_cdir(variable_id, cnode->parent);
}

static bool is_attr_used_in_parent_lnode(betree_var_t variable_id, const struct lnode* lnode)
{
    return is_attr_used_in_parent_cnode(variable_id, lnode->parent);
}

static bool splitable_attr_domain(
    const struct config* config, const struct attr_domain* attr_domain)
{
    switch(attr_domain->bound.value_type) {
        case BETREE_INTEGER:
        case BETREE_INTEGER_LIST:
            if(attr_domain->bound.imin == INT64_MIN || attr_domain->bound.imax == INT64_MAX) {
                return false;
            }
            return (static_cast<std::uint64_t>(llabs(attr_domain->bound.imax - attr_domain->bound.imin)))
                < config->max_domain_for_split;
        case BETREE_FLOAT:
            if(feq(attr_domain->bound.fmin, -DBL_MAX) || feq(attr_domain->bound.fmax, DBL_MAX)) {
                return false;
            }
            return (static_cast<std::uint64_t>(fabs(attr_domain->bound.fmax - attr_domain->bound.fmin)))
                < config->max_domain_for_split;
        case BETREE_BOOLEAN:
            return true;
        case BETREE_STRING:
        case BETREE_STRING_LIST:
        case BETREE_INTEGER_ENUM:
            if(attr_domain->bound.smax == SIZE_MAX) {
                return false;
            }
            return (attr_domain->bound.smax - attr_domain->bound.smin)
                < config->max_domain_for_split;
        case BETREE_SEGMENTS:
        case BETREE_FREQUENCY_CAPS:
            return false;
        default:
            std::abort();
    }
}

static bool get_next_highest_score_unused_attr(
    const struct config* config, const struct lnode* lnode, betree_var_t* var)
{
    bool found = false;
    double highest_score = 0;
    betree_var_t highest_var;
    for(std::size_t i = 0; i < lnode->sub_count; i++) {
        const struct betree_sub* sub = lnode->subs[i];
        for(std::size_t j = 0; j < config->attr_domain_count; j++) {
            if(test_bit(sub->attr_vars, j) == false) {
                continue;
            }
            betree_var_t current_variable_id = j;
            const struct attr_domain* attr_domain = get_attr_domain(
                (const struct attr_domain**)config->attr_domains, current_variable_id);
            if(splitable_attr_domain(config, attr_domain)
                && !is_attr_used_in_parent_lnode(current_variable_id, lnode)) {
                double current_score = get_lnode_score(
                    (const struct attr_domain**)config->attr_domains, lnode, current_variable_id);
                found = true;
                if(current_score > highest_score) {
                    highest_score = current_score;
                    highest_var = current_variable_id;
                }
            }
        }
    }
    if(found == false) {
        return false;
    }
    *var = highest_var;
    return true;
}

static void update_cluster_capacity(const struct config* config, struct lnode* lnode)
{
    if(lnode == nullptr) {
        return;
    }
    std::size_t count = lnode->sub_count;
    std::size_t max = smax(config->lnode_max_cap,
        ceil(static_cast<double>(count) / static_cast<double>(config->lnode_max_cap)) * config->lnode_max_cap);
    lnode->max = max;
}

static std::size_t count_subs_with_variable(
    const struct betree_sub** subs, std::size_t sub_count, betree_var_t variable_id)
{
    std::size_t count = 0;
    for(std::size_t i = 0; i < sub_count; i++) {
        const struct betree_sub* sub = subs[i];
        if(sub_has_attribute(sub, variable_id)) {
            count++;
        }
    }
    return count;
}

static void space_partitioning(const struct config* config, struct cnode* cnode)
{
    struct lnode* lnode = cnode->lnode;
    while(is_overflowed(lnode) == true) {
        betree_var_t var;
        bool found = get_next_highest_score_unused_attr(config, lnode, &var);
        if(found == false) {
            break;
        }
        std::size_t target_subs_count = count_subs_with_variable(
            (const struct betree_sub**)lnode->subs, lnode->sub_count, var);
        if(target_subs_count < config->partition_min_size) {
            break;
        }
        const char* attr = config->attr_domains[var]->attr_var.attr;
        struct pnode* pnode = create_pdir(config, attr, var, cnode);
        for(std::size_t i = 0; i < lnode->sub_count; i++) {
            const struct betree_sub* sub = lnode->subs[i];
            if(sub_has_attribute(sub, var)) {
                struct cdir* cdir = insert_cdir(config, sub, pnode->cdir);
                move(sub, lnode, cdir->cnode->lnode);
                i--;
            }
        }
        space_clustering(config, pnode->cdir);
    }
    update_cluster_capacity(config, lnode);
}

static bool is_atomic(const struct cdir* cdir)
{
    switch(cdir->bound.value_type) {
        case(BETREE_INTEGER):
        case(BETREE_INTEGER_LIST):
            return cdir->bound.imin == cdir->bound.imax;
        case(BETREE_FLOAT): {
            return feq(cdir->bound.fmin, cdir->bound.fmax);
        }
        case(BETREE_BOOLEAN): {
            return cdir->bound.bmin == cdir->bound.bmax;
        }
        case(BETREE_STRING):
        case(BETREE_STRING_LIST):
        case(BETREE_INTEGER_ENUM):
            return cdir->bound.smin == cdir->bound.smax;
        case(BETREE_SEGMENTS): {
            std::fprintf(stderr, "%s a segments value cdir should never happen for now\n", __func__);
            std::abort();
        }
        case(BETREE_FREQUENCY_CAPS): {
            std::fprintf(stderr, "%s a frequency value cdir should never happen for now\n", __func__);
            std::abort();
        }
        default:
            std::abort();
    }
}

extern "C" {

struct lnode* make_lnode(const struct config* config, struct cnode* parent)
{
    auto lnode = static_cast<struct lnode*>(bcalloc(sizeof(struct lnode)));
    if(lnode == nullptr) {
        std::fprintf(stderr, "%s bcalloc failed\n", __func__);
        std::abort();
    }
    lnode->parent = parent;
    lnode->sub_count = 0;
    lnode->subs = nullptr;
    lnode->max = config->lnode_max_cap;
    return lnode;
}

struct cnode* make_cnode(const struct config* config, struct cdir* parent)
{
    auto cnode = static_cast<struct cnode*>(bcalloc(sizeof(struct cnode)));
    if(cnode == nullptr) {
        std::fprintf(stderr, "%s bcalloc failed\n", __func__);
        std::abort();
    }
    cnode->parent = parent;
    cnode->pdir = nullptr;
    cnode->lnode = make_lnode(config, cnode);
    return cnode;
}

} // extern "C"

struct value_bounds {
    struct value_bound lbound;
    struct value_bound rbound;
};

static struct value_bounds split_value_bound(struct value_bound bound)
{
    struct value_bound lbound = {};
    struct value_bound rbound = {};
    lbound.value_type = bound.value_type;
    rbound.value_type = bound.value_type;
    switch(bound.value_type) {
        case(BETREE_INTEGER):
        case(BETREE_INTEGER_LIST): {
            std::int64_t start = bound.imin, end = bound.imax;
            lbound.imin = start;
            rbound.imax = end;
            if(llabs(end - start) > 2) {
                std::int64_t middle = start + (end - start) / 2;
                lbound.imax = middle;
                rbound.imin = middle;
            }
            else if(llabs(end - start) == 2) {
                std::int64_t middle = start + 1;
                lbound.imax = middle;
                rbound.imin = middle;
            }
            else if(llabs(end - start) == 1) {
                lbound.imax = start;
                rbound.imin = end;
            }
            else {
                std::fprintf(stderr, "%s trying to split an unsplitable bound\n", __func__);
                std::abort();
            }
            break;
        }
        case(BETREE_FLOAT): {
            double start = bound.fmin, end = bound.fmax;
            lbound.fmin = start;
            rbound.fmax = end;
            double middle = start + (end - start) / 2;
            if(feq(middle, start) || feq(middle, end)) {
                middle = std::nextafter(start, end);
            }
            if(feq(middle, start) || feq(middle, end)) {
                std::fprintf(stderr, "%s trying to split an unsplitable bound\n", __func__);
                std::abort();
            }
            lbound.fmax = middle;
            rbound.fmin = middle;
            break;
        }
        case(BETREE_BOOLEAN): {
            bool start = bound.bmin, end = bound.bmax;
            lbound.bmin = start;
            rbound.bmax = end;
            if(abs(end - start) == 1) {
                lbound.bmax = start;
                rbound.bmin = end;
            }
            else {
                std::fprintf(stderr, "%s trying to split an unsplitable bound\n", __func__);
                std::abort();
            }
            break;
        }
        case(BETREE_STRING):
        case(BETREE_STRING_LIST):
        case(BETREE_INTEGER_ENUM): {
            std::size_t start = bound.smin, end = bound.smax;
            lbound.smin = start;
            rbound.smax = end;
            if(end - start > 2) {
                std::size_t middle = start + (end - start) / 2;
                lbound.smax = middle;
                rbound.smin = middle;
            }
            else if(end - start == 2) {
                std::int64_t middle = start + 1;
                lbound.smax = middle;
                rbound.smin = middle;
            }
            else if(end - start == 1) {
                lbound.smax = start;
                rbound.smin = end;
            }
            else {
                std::fprintf(stderr, "%s trying to split an unsplitable bound\n", __func__);
                std::abort();
            }
            break;
        }
        case(BETREE_SEGMENTS): {
            std::fprintf(stderr, "%s a segment value cdir should never happen for now\n", __func__);
            std::abort();
        }
        case(BETREE_FREQUENCY_CAPS): {
            std::fprintf(stderr, "%s a frequency value cdir should never happen for now\n", __func__);
            std::abort();
        }
        default:
            std::abort();
    }
    struct value_bounds bounds = { .lbound = lbound, .rbound = rbound };
    return bounds;
}

static void space_clustering(const struct config* config, struct cdir* cdir)
{
    if(cdir == nullptr || cdir->cnode == nullptr) {
        return;
    }
    struct lnode* lnode = cdir->cnode->lnode;
    if(!is_overflowed(lnode)) {
        return;
    }
    if(!is_leaf(cdir) || is_atomic(cdir)) {
        space_partitioning(config, cdir->cnode);
    }
    else {
        struct value_bounds bounds = split_value_bound(cdir->bound);
        cdir->lchild = create_cdir_with_cdir_parent(config, cdir, bounds.lbound);
        cdir->rchild = create_cdir_with_cdir_parent(config, cdir, bounds.rbound);
        for(std::size_t i = 0; i < lnode->sub_count; i++) {
            const struct betree_sub* sub = lnode->subs[i];
            if(sub_is_enclosed(
                   (const struct attr_domain**)config->attr_domains, sub, cdir->lchild)) {
                move(sub, lnode, cdir->lchild->cnode->lnode);
                i--;
            }
            else if(sub_is_enclosed(
                        (const struct attr_domain**)config->attr_domains, sub, cdir->rchild)) {
                move(sub, lnode, cdir->rchild->cnode->lnode);
                i--;
            }
        }
        space_partitioning(config, cdir->cnode);
        space_clustering(config, cdir->lchild);
        space_clustering(config, cdir->rchild);
    }
    update_cluster_capacity(config, lnode);
}

static void free_pnode(struct pnode* pnode);

static void free_pdir(struct pdir* pdir)
{
    if(pdir == nullptr) {
        return;
    }
    for(std::size_t i = 0; i < pdir->pnode_count; i++) {
        struct pnode* pnode = pdir->pnodes[i];
        free_pnode(pnode);
    }
    bfree(pdir->pnodes);
    pdir->pnodes = nullptr;
    bfree(pdir);
}

static void free_pred(struct betree_variable* pred)
{
    if(pred == nullptr) {
        return;
    }
    bfree((char*)pred->attr_var.attr);
    free_value(pred->value);
    bfree(pred);
}

extern "C" {

void free_sub(struct betree_sub* sub)
{
    if(sub == nullptr) {
        return;
    }
    bfree(sub->attr_vars);
    sub->attr_vars = nullptr;
    free_ast_node((struct ast_node*)sub->expr);
    sub->expr = nullptr;
    bfree(sub->short_circuit.pass);
    bfree(sub->short_circuit.fail);
    bfree(sub);
}

void free_event(struct betree_event* event)
{
    if(event == nullptr) {
        return;
    }
    for(std::size_t i = 0; i < event->variable_count; i++) {
        const struct betree_variable* pred = event->variables[i];
        if(pred != nullptr) {
            free_pred((struct betree_variable*)pred);
        }
    }
    bfree(event->variables);
    bfree(event);
}

void free_lnode(struct lnode* lnode)
{
    if(lnode == nullptr) {
        return;
    }
    for(std::size_t i = 0; i < lnode->sub_count; i++) {
        const struct betree_sub* sub = lnode->subs[i];
        free_sub((struct betree_sub*)sub);
    }
    bfree(lnode->subs);
    lnode->subs = nullptr;
    bfree(lnode);
}

void free_cnode(struct cnode* cnode)
{
    if(cnode == nullptr) {
        return;
    }
    free_lnode(cnode->lnode);
    cnode->lnode = nullptr;
    free_pdir(cnode->pdir);
    cnode->pdir = nullptr;
    bfree(cnode);
}

} // extern "C"

static void free_cdir(struct cdir* cdir)
{
    if(cdir == nullptr) {
        return;
    }
    bfree((char*)cdir->attr_var.attr);
    free_cnode(cdir->cnode);
    cdir->cnode = nullptr;
    free_cdir(cdir->lchild);
    cdir->lchild = nullptr;
    free_cdir(cdir->rchild);
    cdir->rchild = nullptr;
    bfree(cdir);
}

static void free_pnode(struct pnode* pnode)
{
    if(pnode == nullptr) {
        return;
    }
    bfree((char*)pnode->attr_var.attr);
    free_cdir(pnode->cdir);
    pnode->cdir = nullptr;
    bfree(pnode);
}

// Forward declarations for mutual recursion
static std::optional<betree_sub*> find_sub_id_cdir_opt(betree_sub_t id, struct cdir* cdir);

// Internal implementation using std::optional for type safety
static std::optional<betree_sub*> find_sub_id_opt(betree_sub_t id, struct cnode* cnode)
{
    if(cnode == nullptr) {
        return std::nullopt;
    }

    // Check subscriptions in leaf node
    for(std::size_t i = 0; i < cnode->lnode->sub_count; i++) {
        if(cnode->lnode->subs[i]->id == id) {
            return cnode->lnode->subs[i];
        }
    }

    // Check partition directory
    if(cnode->pdir != nullptr) {
        for(std::size_t i = 0; i < cnode->pdir->pnode_count; i++) {
            if(auto in_cdir = find_sub_id_cdir_opt(id, cnode->pdir->pnodes[i]->cdir); in_cdir) {
                return in_cdir;
            }
        }
    }

    return std::nullopt;
}

static std::optional<betree_sub*> find_sub_id_cdir_opt(betree_sub_t id, struct cdir* cdir)
{
    if(cdir == nullptr) {
        return std::nullopt;
    }

    if(auto in_cnode = find_sub_id_opt(id, cdir->cnode); in_cnode) {
        return in_cnode;
    }

    if(auto in_lcdir = find_sub_id_cdir_opt(id, cdir->lchild); in_lcdir) {
        return in_lcdir;
    }

    if(auto in_rcdir = find_sub_id_cdir_opt(id, cdir->rchild); in_rcdir) {
        return in_rcdir;
    }

    return std::nullopt;
}

// C-compatible wrappers
extern "C" {

struct betree_sub* find_sub_id(betree_sub_t id, struct cnode* cnode)
{
    return find_sub_id_opt(id, cnode).value_or(nullptr);
}

struct betree_variable* make_pred(const char* attr, betree_var_t variable_id, struct value value)
{
    auto pred = static_cast<struct betree_variable*>(bcalloc(sizeof(struct betree_variable)));
    if(pred == nullptr) {
        std::fprintf(stderr, "%s bcalloc failed\n", __func__);
        std::abort();
    }
    pred->attr_var.attr = bstrdup(attr);
    pred->attr_var.var = variable_id;
    pred->value = value;
    return pred;
}

} // extern "C"

static void fill_pred_attr_var(struct betree_sub* sub, struct attr_var attr_var)
{
    set_bit(sub->attr_vars, attr_var.var);
}

extern "C" {

void fill_pred(struct betree_sub* sub, const struct ast_node* expr)
{
    switch(expr->type) {
        case AST_TYPE_IS_NULL_EXPR:
            fill_pred_attr_var(sub, expr->is_null_expr.attr_var);
            return;
        case AST_TYPE_SPECIAL_EXPR: {
            switch(expr->special_expr.type) {
                case AST_SPECIAL_FREQUENCY:
                    fill_pred_attr_var(sub, expr->special_expr.frequency.attr_var);
                    return;
                case AST_SPECIAL_GEO:
                    fill_pred_attr_var(sub, expr->special_expr.geo.latitude_var);
                    fill_pred_attr_var(sub, expr->special_expr.geo.longitude_var);
                    return;
                case AST_SPECIAL_STRING:
                    fill_pred_attr_var(sub, expr->special_expr.string.attr_var);
                    return;
                case AST_SPECIAL_SEGMENT:
                    fill_pred_attr_var(sub, expr->special_expr.segment.attr_var);
                    return;
                default:
                    std::abort();
            }
            return;
        }
        case AST_TYPE_BOOL_EXPR: {
            switch(expr->bool_expr.op) {
                case AST_BOOL_AND:
                case AST_BOOL_OR:
                    fill_pred(sub, expr->bool_expr.binary.lhs);
                    fill_pred(sub, expr->bool_expr.binary.rhs);
                    return;
                case AST_BOOL_NOT:
                    fill_pred(sub, expr->bool_expr.unary.expr);
                    return;
                case AST_BOOL_VARIABLE:
                    fill_pred_attr_var(sub, expr->bool_expr.variable);
                    return;
                case AST_BOOL_LITERAL:
                    return;
                default:
                    std::abort();
            }
            return;
        }
        case AST_TYPE_COMPARE_EXPR: {
            fill_pred_attr_var(sub, expr->compare_expr.attr_var);
            return;
        }
        case AST_TYPE_EQUALITY_EXPR: {
            fill_pred_attr_var(sub, expr->equality_expr.attr_var);
            return;
        }
        case AST_TYPE_SET_EXPR: {
            if(expr->set_expr.left_value.value_type == AST_SET_LEFT_VALUE_VARIABLE) {
                fill_pred_attr_var(sub, expr->set_expr.left_value.variable_value);
            }
            else if(expr->set_expr.right_value.value_type == AST_SET_RIGHT_VALUE_VARIABLE) {
                fill_pred_attr_var(sub, expr->set_expr.right_value.variable_value);
            }
            else {
                return;
            }
            return;
        }
        case AST_TYPE_LIST_EXPR: {
            fill_pred_attr_var(sub, expr->list_expr.attr_var);
            return;
        }
        default:
            std::abort();
    }
}

} // extern "C"

static enum short_circuit_e short_circuit_for_attr_var(
    betree_var_t id, bool inverted, struct attr_var attr_var)
{
    if(id == attr_var.var) {
        if(inverted) {
            return SHORT_CIRCUIT_PASS;
        }
        return SHORT_CIRCUIT_FAIL;
    }
    return SHORT_CIRCUIT_NONE;
}

static enum short_circuit_e short_circuit_for_node(
    betree_var_t id, bool inverted, const struct ast_node* node)
{
    switch(node->type) {
        case AST_TYPE_IS_NULL_EXPR:
            switch(node->is_null_expr.type) {
                case AST_IS_NULL:
                    return short_circuit_for_attr_var(id, !inverted, node->is_null_expr.attr_var);
                case AST_IS_NOT_NULL:
                    return short_circuit_for_attr_var(id, inverted, node->is_null_expr.attr_var);
                case AST_IS_EMPTY:
                    return short_circuit_for_attr_var(id, inverted, node->is_null_expr.attr_var);
                default:
                    std::abort();
            }
        case AST_TYPE_COMPARE_EXPR:
            return short_circuit_for_attr_var(id, inverted, node->compare_expr.attr_var);
        case AST_TYPE_EQUALITY_EXPR:
            return short_circuit_for_attr_var(id, inverted, node->equality_expr.attr_var);
        case AST_TYPE_BOOL_EXPR:
            switch(node->bool_expr.op) {
                case AST_BOOL_LITERAL:
                    return SHORT_CIRCUIT_NONE;
                case AST_BOOL_OR: {
                    auto lhs
                        = short_circuit_for_node(id, inverted, node->bool_expr.binary.lhs);
                    auto rhs
                        = short_circuit_for_node(id, inverted, node->bool_expr.binary.rhs);
                    if(lhs == SHORT_CIRCUIT_PASS || rhs == SHORT_CIRCUIT_PASS) {
                        return SHORT_CIRCUIT_PASS;
                    }
                    if(lhs == SHORT_CIRCUIT_FAIL && rhs == SHORT_CIRCUIT_FAIL) {
                        return SHORT_CIRCUIT_FAIL;
                    }
                    return SHORT_CIRCUIT_NONE;
                }
                case AST_BOOL_AND: {
                    auto lhs
                        = short_circuit_for_node(id, inverted, node->bool_expr.binary.lhs);
                    auto rhs
                        = short_circuit_for_node(id, inverted, node->bool_expr.binary.rhs);
                    if(lhs == SHORT_CIRCUIT_FAIL || rhs == SHORT_CIRCUIT_FAIL) {
                        return SHORT_CIRCUIT_FAIL;
                    }
                    if(lhs == SHORT_CIRCUIT_PASS && rhs == SHORT_CIRCUIT_PASS) {
                        return SHORT_CIRCUIT_PASS;
                    }
                    return SHORT_CIRCUIT_NONE;
                }
                case AST_BOOL_NOT:
                    return short_circuit_for_node(id, !inverted, node->bool_expr.unary.expr);
                case AST_BOOL_VARIABLE:
                    return short_circuit_for_attr_var(id, inverted, node->bool_expr.variable);
                default:
                    std::abort();
            }
        case AST_TYPE_SET_EXPR:
            if(node->set_expr.left_value.value_type == AST_SET_LEFT_VALUE_VARIABLE) {
                return short_circuit_for_attr_var(
                    id, inverted, node->set_expr.left_value.variable_value);
            }
            else {
                return short_circuit_for_attr_var(
                    id, inverted, node->set_expr.right_value.variable_value);
            }
        case AST_TYPE_LIST_EXPR:
            return short_circuit_for_attr_var(id, inverted, node->list_expr.attr_var);
        case AST_TYPE_SPECIAL_EXPR:
            switch(node->special_expr.type) {
                case AST_SPECIAL_FREQUENCY: {
                    auto frequency = short_circuit_for_attr_var(
                        id, inverted, node->special_expr.frequency.attr_var);
                    auto now = short_circuit_for_attr_var(
                        id, inverted, node->special_expr.frequency.now);
                    if(frequency == SHORT_CIRCUIT_FAIL || now == SHORT_CIRCUIT_FAIL) {
                        return SHORT_CIRCUIT_FAIL;
                    }
                    if(frequency == SHORT_CIRCUIT_PASS && now == SHORT_CIRCUIT_PASS) {
                        return SHORT_CIRCUIT_PASS;
                    }
                    return SHORT_CIRCUIT_NONE;
                }
                case AST_SPECIAL_SEGMENT: {
                    auto frequency = short_circuit_for_attr_var(
                        id, inverted, node->special_expr.segment.attr_var);
                    auto now
                        = short_circuit_for_attr_var(id, inverted, node->special_expr.segment.now);
                    if(frequency == SHORT_CIRCUIT_FAIL || now == SHORT_CIRCUIT_FAIL) {
                        return SHORT_CIRCUIT_FAIL;
                    }
                    if(frequency == SHORT_CIRCUIT_PASS && now == SHORT_CIRCUIT_PASS) {
                        return SHORT_CIRCUIT_PASS;
                    }
                    return SHORT_CIRCUIT_NONE;
                }
                case AST_SPECIAL_GEO: {
                    auto latitude = short_circuit_for_attr_var(
                        id, inverted, node->special_expr.geo.latitude_var);
                    auto longitude = short_circuit_for_attr_var(
                        id, inverted, node->special_expr.geo.longitude_var);
                    if(latitude == SHORT_CIRCUIT_FAIL || longitude == SHORT_CIRCUIT_FAIL) {
                        return SHORT_CIRCUIT_FAIL;
                    }
                    if(latitude == SHORT_CIRCUIT_PASS && longitude == SHORT_CIRCUIT_PASS) {
                        return SHORT_CIRCUIT_PASS;
                    }
                    return SHORT_CIRCUIT_NONE;
                }
                case AST_SPECIAL_STRING:
                    return short_circuit_for_attr_var(
                        id, inverted, node->special_expr.string.attr_var);
                default:
                    std::abort();
            }
        default:
            std::abort();
    }
    return SHORT_CIRCUIT_NONE;
}

static void fill_short_circuit(struct config* config, struct betree_sub* sub)
{
    for(std::size_t i = 0; i < config->attr_domain_count; i++) {
        struct attr_domain* attr_domain = config->attr_domains[i];
        if(attr_domain->allow_undefined) {
            auto result
                = short_circuit_for_node(attr_domain->attr_var.var, false, sub->expr);
            if(result == SHORT_CIRCUIT_PASS) {
                set_bit(sub->short_circuit.pass, i);
            }
            else if(result == SHORT_CIRCUIT_FAIL) {
                set_bit(sub->short_circuit.fail, i);
            }
        }
    }
}

extern "C" {

struct betree_sub* make_sub(struct config* config, betree_sub_t id, struct ast_node* expr)
{
    auto sub = static_cast<struct betree_sub*>(bcalloc(sizeof(struct betree_sub)));
    if(sub == nullptr) {
        std::fprintf(stderr, "%s bcalloc failed\n", __func__);
        std::abort();
    }
    sub->id = id;
    sub->data = (void *)id;
    std::size_t count = (config->attr_domain_count + 63) / 64;
    sub->attr_vars = static_cast<std::uint64_t*>(bcalloc(count * sizeof(std::uint64_t)));
    sub->expr = expr;
    fill_pred(sub, sub->expr);
    sub->short_circuit.pass = static_cast<std::uint64_t*>(bcalloc(count * sizeof(std::uint64_t)));
    sub->short_circuit.fail = static_cast<std::uint64_t*>(bcalloc(count * sizeof(std::uint64_t)));
    fill_short_circuit(config, sub);
    return sub;
}

struct betree_event* make_empty_event(void)
{
    auto event = static_cast<struct betree_event*>(bcalloc(sizeof(struct betree_event)));
    if(event == nullptr) {
        std::fprintf(stderr, "%s event bcalloc failed\n", __func__);
        std::abort();
    }
    event->config = nullptr;
    event->variable_count = 0;
    event->variables = nullptr;
    return event;
}

} // extern "C"

extern "C" {

const char* get_attr_for_id(const struct config* config, betree_var_t variable_id)
{
    if(variable_id < config->attr_domain_count) {
        return config->attr_domains[variable_id]->attr_var.attr;
    }
    return nullptr;
}

betree_var_t try_get_id_for_attr(const struct config* config, const char* attr)
{
    char* copy = bstrdup(attr);
    for(std::size_t i = 0; copy[i]; i++) {
        copy[i] = tolower(copy[i]);
    }
    for(std::size_t i = 0; i < config->attr_domain_count; i++) {
        if(strcmp(config->attr_domains[i]->attr_var.attr, copy) == 0) {
            bfree(copy);
            return i;
        }
    }
    bfree(copy);
    return INVALID_VAR;
}

} // extern "C"

static bool append_event_text(
    char* buffer, std::size_t capacity, std::size_t* length, const char* format, ...)
{
    if(*length >= capacity) {
        return false;
    }

    va_list args;
    va_start(args, format);
    const int written = std::vsnprintf(buffer + *length, capacity - *length, format, args);
    va_end(args);

    if(written < 0) {
        buffer[*length] = '\0';
        return false;
    }
    if(static_cast<std::size_t>(written) >= capacity - *length) {
        *length = capacity - 1;
        return false;
    }

    *length += static_cast<std::size_t>(written);
    return true;
}

extern "C" {

bool event_to_string(
    const struct betree_event* event, char* buffer, std::size_t capacity)
{
    if(buffer == nullptr || capacity == 0) {
        return false;
    }
    buffer[0] = '\0';
    if(event == nullptr) {
        return false;
    }

    std::size_t length = 0;
    for(std::size_t i = 0; i < event->variable_count; i++) {
        const struct betree_variable* pred = event->variables[i];
        if(i != 0 && !append_event_text(buffer, capacity, &length, ", ")) {
            return false;
        }
        const char* attr = pred->attr_var.attr;
        switch(pred->value.value_type) {
            case(BETREE_INTEGER): {
                if(!append_event_text(buffer, capacity, &length,
                       "%s = %" PRId64, attr, pred->value.integer_value)) {
                    return false;
                }
                break;
            }
            case(BETREE_FLOAT): {
                if(!append_event_text(
                       buffer, capacity, &length, "%s = %.2f", attr, pred->value.float_value)) {
                    return false;
                }
                break;
            }
            case(BETREE_BOOLEAN): {
                const char* value = pred->value.boolean_value ? "true" : "false";
                if(!append_event_text(buffer, capacity, &length, "%s = %s", attr, value)) {
                    return false;
                }
                break;
            }
            case(BETREE_STRING): {
                if(!append_event_text(buffer, capacity, &length,
                       "%s = \"%s\"", attr, pred->value.string_value.string)) {
                    return false;
                }
                break;
            }
            case(BETREE_INTEGER_ENUM): {
                if(!append_event_text(buffer, capacity, &length,
                       "%s = %" PRId64, attr, pred->value.integer_enum_value.integer)) {
                    return false;
                }
                break;
            }
            case(BETREE_INTEGER_LIST): {
                char* integer_list
                    = integer_list_value_to_string(pred->value.integer_list_value);
                const bool appended = append_event_text(
                    buffer, capacity, &length, "%s = (%s)", attr,
                    integer_list == nullptr ? "" : integer_list);
                bfree(integer_list);
                if(!appended) {
                    return false;
                }
                break;
            }
            case(BETREE_SEGMENTS): {
                char* segments = segments_value_to_string(pred->value.segments_value);
                const bool appended = append_event_text(buffer, capacity, &length,
                    "%s = (%s)", attr, segments == nullptr ? "" : segments);
                bfree(segments);
                if(!appended) {
                    return false;
                }
                break;
            }
            case(BETREE_FREQUENCY_CAPS): {
                char* frequency_caps
                    = frequency_caps_value_to_string(pred->value.frequency_caps_value);
                const bool appended = append_event_text(buffer, capacity, &length,
                    "%s = (%s)", attr, frequency_caps == nullptr ? "" : frequency_caps);
                bfree(frequency_caps);
                if(!appended) {
                    return false;
                }
                break;
            }
            case(BETREE_STRING_LIST): {
                char* string_list
                    = string_list_value_to_string(pred->value.string_list_value);
                const bool appended = append_event_text(buffer, capacity, &length,
                    "%s = (%s)", attr, string_list == nullptr ? "" : string_list);
                bfree(string_list);
                if(!appended) {
                    return false;
                }
                break;
            }
            case(BETREE_UNFETCHED): {
                if(!append_event_text(buffer, capacity, &length, "%s = <unfetched>", attr)) {
                    return false;
                }
                break;
            }
            default:
                return false;
        }
    }
    return true;
}

struct memoize make_memoize(std::size_t pred_count)
{
    std::size_t count = (pred_count + 63) / 64;
    struct memoize memoize = {
        .pass = static_cast<std::uint64_t*>(bcalloc(count * sizeof(std::uint64_t))),
        .fail = static_cast<std::uint64_t*>(bcalloc(count * sizeof(std::uint64_t))),
    };
    return memoize;
}

struct memoize make_memoize_with_count(std::size_t pred_count, std::size_t* count)
{
    *count = (pred_count + 63) / 64;
    struct memoize memoize = {
        .pass = static_cast<std::uint64_t*>(bcalloc(*count * sizeof(std::uint64_t))),
        .fail = static_cast<std::uint64_t*>(bcalloc(*count * sizeof(std::uint64_t))),
    };
    return memoize;
}

void free_memoize(struct memoize memoize)
{
    bfree(memoize.pass);
    bfree(memoize.fail);
}

std::uint64_t* make_undefined(std::size_t attr_domain_count, const struct betree_variable** preds)
{
    std::size_t count = (attr_domain_count + 63) / 64;
    auto undefined = static_cast<std::uint64_t*>(bcalloc(count * sizeof(std::uint64_t)));
    for(std::size_t i = 0; i < attr_domain_count; i++) {
        if(preds[i] == nullptr) {
            set_bit(undefined, i);
        }
    }
    return undefined;
}

std::uint64_t* make_undefined_with_count(
    std::size_t attr_domain_count, const struct betree_variable** preds, std::size_t* count)
{
    *count = (attr_domain_count + 63) / 64;
    auto undefined = static_cast<std::uint64_t*>(bcalloc(*count * sizeof(std::uint64_t)));
    for(std::size_t i = 0; i < attr_domain_count; i++) {
        if(preds[i] == nullptr) {
            set_bit(undefined, i);
        }
    }
    return undefined;
}

void add_sub(betree_sub_t id, struct report* report)
{
    append_report_sub_id(id, report);
}

void add_sub_counting(betree_sub_t id, struct report_counting* report)
{
    append_report_sub_id(id, report);
}

bool betree_search_with_preds(const struct config* config,
    const struct betree_variable** preds,
    const struct cnode* cnode,
    struct report* report)
{
    std::size_t dom_cnt = config->attr_domain_count;
    std::size_t pred_count = config->pred_map->memoize_count;
    std::uint64_t* undefined = make_undefined(dom_cnt, preds);
    struct memoize memoize = make_memoize(pred_count);
    struct subs_to_eval subs;
    init_subs_to_eval(&subs);
    match_be_tree(config, preds, cnode, &subs, report);
    if (report->cb != nullptr) {
        if(pred_count > 0) {
            report->memoize_vars
                = static_cast<betree_var_t*>(bmalloc(pred_count * sizeof(betree_var_t)));
            if(report->memoize_vars == nullptr) {
                std::fprintf(stderr, "%s bmalloc failed\n", __func__);
                std::abort();
            }
        }
        void* arg = report->arg;
        evaluate_subs_shared(
            subs,
            report,
            [&](const struct betree_sub* sub) {
                return match_sub_(dom_cnt, preds, sub, report, &memoize, undefined);
            },
            [&](const struct betree_sub* sub, bool) {
                (*report->cb)(arg, sub->data, true, (void*)report->last_var);
            },
            [&](const struct betree_sub* sub, bool) {
                (*report->cb)(arg, sub->data, false, (void*)report->last_var);
            });
    }
    else {
        evaluate_subs_shared(
            subs,
            report,
            [&](const struct betree_sub* sub) {
                return match_sub(dom_cnt, preds, sub, report, &memoize, undefined);
            },
            [&](const struct betree_sub* sub, bool) { add_sub(sub->id, report); },
            [](const struct betree_sub*, bool) {});
    }
    bfree(subs.subs);
    free_memoize(memoize);
    bfree(report->memoize_vars);
    report->memoize_vars = nullptr;
    bfree(undefined);
    bfree(preds);
    return true;
}

} // extern "C"

extern "C" {

bool betree_search_with_preds_ids(const struct config* config,
    const struct betree_variable** preds,
    const struct cnode* cnode,
    struct report* report,
    const std::uint64_t* ids,
    std::size_t sz)
{
    std::uint64_t* undefined = make_undefined(config->attr_domain_count, preds);
    struct memoize memoize = make_memoize(config->pred_map->memoize_count);
    struct subs_to_eval subs;
    init_subs_to_eval(&subs);
    match_be_tree_ids(
        (const struct attr_domain**)config->attr_domains, preds, cnode, &subs, ids, sz);
    evaluate_subs_shared(
        subs,
        report,
        [&](const struct betree_sub* sub) {
            return match_sub(config->attr_domain_count, preds, sub, report, &memoize, undefined);
        },
        [&](const struct betree_sub* sub, bool) { add_sub(sub->id, report); },
        [](const struct betree_sub*, bool) {});
    bfree(subs.subs);
    free_memoize(memoize);
    bfree(undefined);
    bfree(preds);
    return true;
}

bool betree_exists_with_preds(
    const struct config* config, const struct betree_variable** preds, const struct cnode* cnode)
{
    std::uint64_t* undefined = make_undefined(config->attr_domain_count, preds);
    struct memoize memoize = make_memoize(config->pred_map->memoize_count);
    struct subs_to_eval subs;
    init_subs_to_eval(&subs);
    match_be_tree(config, preds, cnode, &subs, nullptr);
    bool result = false;
    for(std::size_t i = 0; i < subs.count; i++) {
        const struct betree_sub* sub = subs.subs[i];
        if(match_sub(config->attr_domain_count, preds, sub, nullptr, &memoize, undefined) == true) {
            result = true;
            break;
        }
    }
    bfree(subs.subs);
    free_memoize(memoize);
    bfree(undefined);
    bfree(preds);
    return result;
}

void sort_event_lists(struct betree_event* event)
{
    for(std::size_t i = 0; i < event->variable_count; i++) {
        struct betree_variable* pred = event->variables[i];
        if(pred == nullptr) {
            continue;
        }
        if(pred->value.value_type == BETREE_INTEGER_LIST) {
            sort_and_remove_duplicate_integer_list(pred->value.integer_list_value);
        }
        else if(pred->value.value_type == BETREE_STRING_LIST) {
            sort_and_remove_duplicate_string_list(pred->value.string_list_value);
        }
    }
}

struct betree_event* make_event_from_string(const struct betree* betree, const char* event_str)
{
    struct betree_event* event = nullptr;
    if(event_parse(event_str, &event) != 0) {
        return nullptr;
    }
    fill_event(betree->config, event);
    sort_event_lists(event);
    return event;
}

} // extern "C"

extern "C" {

struct attr_var make_attr_var(const char* attr, struct config* config)
{
    struct attr_var attr_var;
    attr_var.attr = attr == nullptr ? nullptr : bstrdup(attr);
    if(config == nullptr) {
        attr_var.var = INVALID_VAR;
    }
    else {
        attr_var.var = try_get_id_for_attr(config, attr);
    }
    return attr_var;
}

void free_attr_var(struct attr_var attr_var)
{
    bfree((char*)attr_var.attr);
}

struct attr_var copy_attr_var(struct attr_var attr_var)
{
    struct attr_var copy = {};
    copy.attr = bstrdup(attr_var.attr);
    copy.var = attr_var.var;
    return copy;
}

} // extern "C"

extern "C" {

void add_variable(struct betree_variable* variable, struct betree_event* event)
{
    if(variable == nullptr) {
        return;
    }
    if(event->variable_count == 0) {
        event->variables = static_cast<struct betree_variable**>(bcalloc(sizeof(struct betree_variable*)));
        if(event->variables == nullptr) {
            std::fprintf(stderr, "%s bcalloc failed\n", __func__);
            std::abort();
        }
    }
    else {
        auto variables
            = static_cast<struct betree_variable**>(brealloc(event->variables, sizeof(struct betree_variable*) * (event->variable_count + 1)));
        if(variables == nullptr) {
            std::fprintf(stderr, "%s brealloc failed\n", __func__);
            std::abort();
        }
        event->variables = variables;
    }
    event->variables[event->variable_count] = variable;
    event->variable_count++;
}

void fill_event(const struct config* config, struct betree_event* event)
{
    for(std::size_t i = 0; i < event->variable_count; i++) {
        struct betree_variable* pred = event->variables[i];
        if(pred == nullptr) {
            continue;
        }
        betree_var_t var = try_get_id_for_attr(config, pred->attr_var.attr);
        if(unlikely(var == INVALID_VAR)) {
            std::fprintf(stderr, "Cannot find variable %s in config, aborting", pred->attr_var.attr);
            std::abort();
        }
        pred->attr_var.var = var;
        struct attr_domain* domain = config->attr_domains[var];
        enum betree_value_type_e expected_type = domain->bound.value_type;
        enum betree_value_type_e actual_type = pred->value.value_type;
        if(actual_type == BETREE_INTEGER && expected_type == BETREE_INTEGER_ENUM) {
            pred->value.value_type = BETREE_INTEGER_ENUM;
            actual_type = BETREE_INTEGER_ENUM;
        }
        else if(actual_type == BETREE_INTEGER_LIST
            && pred->value.integer_list_value != nullptr
            && pred->value.integer_list_value->count == 0
            && (expected_type == BETREE_STRING_LIST
                || expected_type == BETREE_SEGMENTS
                || expected_type == BETREE_FREQUENCY_CAPS)) {
            free_integer_list(pred->value.integer_list_value);
            pred->value.integer_list_value = nullptr;
            if(expected_type == BETREE_STRING_LIST) {
                pred->value.string_list_value = make_string_list();
            }
            else if(expected_type == BETREE_SEGMENTS) {
                pred->value.segments_value = make_segments();
            }
            else {
                pred->value.frequency_caps_value = make_frequency_caps();
            }
            pred->value.value_type = expected_type;
            actual_type = expected_type;
        }
        else if(actual_type != expected_type) {
            continue;
        }
        switch(actual_type) {
            case BETREE_BOOLEAN:
            case BETREE_INTEGER:
            case BETREE_FLOAT:
            case BETREE_INTEGER_LIST:
            case BETREE_SEGMENTS:
                break;
            case BETREE_INTEGER_ENUM: {
                pred->value.integer_enum_value.integer = pred->value.integer_value;
                betree_ienum_t ienum = try_get_id_for_ienum(
                    config, pred->attr_var, pred->value.integer_enum_value.integer);
                pred->value.integer_enum_value.var = pred->attr_var.var;
                pred->value.integer_enum_value.ienum = ienum;
                break;
            }
            case BETREE_STRING: {
                betree_str_t str = try_get_id_for_string(
                    config, pred->attr_var, pred->value.string_value.string);
                pred->value.string_value.var = pred->attr_var.var;
                pred->value.string_value.str = str;
                break;
            }
            case BETREE_STRING_LIST: {
                for(std::size_t j = 0; j < pred->value.string_list_value->count; j++) {
                    betree_str_t str = try_get_id_for_string(
                        config, pred->attr_var, pred->value.string_list_value->strings[j].string);
                    pred->value.string_list_value->strings[j].var = pred->attr_var.var;
                    pred->value.string_list_value->strings[j].str = str;
                }
                break;
            }
            case BETREE_FREQUENCY_CAPS: {
                for(std::size_t j = 0; j < pred->value.frequency_caps_value->size; j++) {
                    betree_str_t str = try_get_id_for_string(config,
                        pred->attr_var,
                        pred->value.frequency_caps_value->content[j]->ns.string);
                    pred->value.frequency_caps_value->content[j]->ns.var
                        = pred->attr_var.var;
                    pred->value.frequency_caps_value->content[j]->ns.str = str;
                }
                break;
            }
            default:
                std::abort();
        }
    }
}

bool validate_variables(const struct config* config, const struct betree_variable* variables[])
{
    for(std::size_t i = 0; i < config->attr_domain_count; i++) {
        const struct attr_domain* attr_domain = config->attr_domains[i];
        const struct betree_variable* variable = variables[i];
        if(attr_domain->allow_undefined == false && variable == nullptr) {
            return false;
        }
        if(variable != nullptr && variable->value.value_type != attr_domain->bound.value_type) {
            return false;
        }
    }
    return true;
}

} // extern "C"

static void extract_cdir_subs(struct cdir* cdir, struct subs_data* acc);

static void extract_pnode_subs(struct pnode* pnode, struct subs_data* acc) {
    if (pnode != nullptr && pnode->cdir != nullptr) extract_cdir_subs(pnode->cdir, acc);
}

static void extract_pdir_subs(struct pdir* pdir, struct subs_data* acc) {
    for (std::size_t i=0; i<pdir->pnode_count; i++) extract_pnode_subs(pdir->pnodes[i], acc);
}

static void extract_lnode_subs(struct lnode* lnode, struct subs_data* acc) {
    for (std::size_t i=0; i<lnode->sub_count; i++) {
        struct betree_sub* sub = lnode->subs[i];
        if (sub != nullptr) {
            assert(acc->count < acc->limit);
            acc->array[acc->count++] = sub->data;
        }
    }
}

static void extract_cnode_subs(struct cnode* cnode, struct subs_data* acc) {
    if (cnode->lnode != nullptr) extract_lnode_subs(cnode->lnode, acc);
    if (cnode->pdir != nullptr) extract_pdir_subs(cnode->pdir, acc);
}

static void extract_cdir_subs(struct cdir* cdir, struct subs_data* acc) {
    std::size_t offset = acc->count;
    cdir->subs_data_array = &acc->array[offset];
    if (cdir->cnode != nullptr) extract_cnode_subs(cdir->cnode, acc);
    if (cdir->lchild != nullptr) extract_cdir_subs(cdir->lchild, acc);
    if (cdir->rchild != nullptr) extract_cdir_subs(cdir->rchild, acc);
    cdir->subs_data_count = acc->count - offset;
}

extern "C" {

void prepare_cnode_subs(struct cnode* cnode, struct subs_data* data) {
    extract_cnode_subs(cnode, data);
}

} // extern "C"
