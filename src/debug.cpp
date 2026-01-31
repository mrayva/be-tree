#include <cstdint>
#include <cstdio>
#include <cstring>
#include <assert.h>

#include "alloc.h"
#include "ast.h"
#include "betree.h"
#include "betree_err.h"
#include "tree.h"
#include "utils.h"

static bool should_print_cnode(const struct cnode* cnode)
{
    return cnode->pdir != nullptr || cnode->lnode->sub_count > 0;
}

static const char* get_path_cnode(const struct config* config, const struct cnode* cnode);

static const char* escape_name(const char* input)
{
    std::size_t len = std::strlen(input);
    auto* escaped = static_cast<char*>(bcalloc((len + 1) * sizeof(char)));
    if(escaped == nullptr) {
        std::fprintf(stderr, "%s bcalloc failed", __func__);
        std::abort();
    }
    for(std::size_t i = 0; i < len; i++) {
        if(input[i] == '-') {
            escaped[i] = '_';
        }
        else {
            escaped[i] = input[i];
        }
    }
    escaped[len] = '\0';
    return escaped;
}

static const char* get_path_pnode(const struct config* config, const struct pnode* pnode)
{
    char* name;
    const char* parent_path = get_path_cnode(config, pnode->parent->parent);
    const char* escaped_attr = escape_name(pnode->attr_var.attr);
    if(basprintf(&name, "%s_%s", parent_path, escaped_attr) < 0) {
        std::abort();
    }
    bfree((char*)parent_path);
    bfree((char*)escaped_attr);
    return name;
}

static const char* get_path_cdir(const struct config* config, const struct cdir* cdir, bool first)
{
    const char* parent_path = nullptr;
    switch(cdir->parent_type) {
        case(CNODE_PARENT_CDIR): {
            if(first) {
                parent_path = get_path_cdir(config, cdir->cdir_parent, false);
                break;
            }
            return get_path_cdir(config, cdir->cdir_parent, false);
        }
        case(CNODE_PARENT_PNODE): {
            if(first) {
                parent_path = get_path_pnode(config, cdir->pnode_parent);
                break;
            }
            return get_path_pnode(config, cdir->pnode_parent);
        }
        default:
            std::abort();
    }
    char* name;
    switch(cdir->bound.value_type) {
        case(BETREE_INTEGER):
        case(BETREE_INTEGER_LIST):
            if(basprintf(&name, "%s_%lu_%lu", parent_path, cdir->bound.imin, cdir->bound.imax)
                < 0) {
                std::abort();
            }
            break;
        case(BETREE_FLOAT): {
            if(basprintf(&name, "%s_%.0f_%.0f", parent_path, cdir->bound.fmin, cdir->bound.fmax)
                < 0) {
                std::abort();
            }
            break;
        }
        case(BETREE_BOOLEAN): {
            const char* min = cdir->bound.bmin ? "true" : "false";
            const char* max = cdir->bound.bmax ? "true" : "false";
            if(basprintf(&name, "%s_%s_%s", parent_path, min, max) < 0) {
                std::abort();
            }
            break;
        }
        case(BETREE_STRING):
        case(BETREE_STRING_LIST):
        case(BETREE_INTEGER_ENUM):
            if(basprintf(&name, "%s_%zu_%zu", parent_path, cdir->bound.smin, cdir->bound.smax)
                < 0) {
                std::abort();
            }
            break;
        case(BETREE_SEGMENTS): {
            std::fprintf(stderr, "%s a segments value cdir should never happen for now", __func__);
            std::abort();
        }
        case(BETREE_FREQUENCY_CAPS): {
            std::fprintf(stderr, "%s a frequency value cdir should never happen for now", __func__);
            std::abort();
        }
        default:
            std::abort();
    }
    bfree((char*)parent_path);
    return name;
}

static const char* get_path_cnode(const struct config* config, const struct cnode* cnode)
{
    if(cnode->parent == nullptr) {
        return bstrdup("");
    }
    return get_path_cdir(config, cnode->parent, true);
}

static const char* get_name_pnode(const struct config* config, const struct pnode* pnode)
{
    char* name;
    const char* path = get_path_pnode(config, pnode);
    if(basprintf(&name, "pnode_%s", path) < 0) {
        std::abort();
    }
    bfree((char*)path);
    return name;
}

static const char* get_name_cdir(const struct config* config, const struct cdir* cdir)
{
    char* name;
    const char* path = get_path_cdir(config, cdir, true);
    if(basprintf(&name, "cdir_%s", path) < 0) {
        std::abort();
    }
    bfree((char*)path);
    return name;
}

static const char* get_name_cnode(const struct config* config, const struct cnode* cnode)
{
    if(cnode->parent == nullptr) {
        return bstrdup("cnode_root");
    }
    char* name;
    const char* path = get_path_cnode(config, cnode);
    if(basprintf(&name, "cnode_%s", path) < 0) {
        std::abort();
    }
    bfree((char*)path);
    return name;
}

static const char* get_name_lnode(const struct config* config, const struct lnode* lnode)
{
    char* name;
    const char* path = get_path_cnode(config, lnode->parent);
    if(basprintf(&name, "lnode_%s", path) < 0) {
        std::abort();
    }
    bfree((char*)path);
    return name;
}

static const char* get_name_pdir(const struct config* config, const struct pdir* pdir)
{
    char* name;
    const char* path = get_path_cnode(config, pdir->parent);
    if(basprintf(&name, "pdir_%s", path) < 0) {
        std::abort();
    }
    bfree((char*)path);
    return name;
}

static void print_spaces(FILE* f, std::uint64_t level)
{
    std::fprintf(f, "%*s", (int)level * 4, "");
}

static void write_dot_file_lnode_names(
    FILE* f, const struct config* config, const struct lnode* lnode, std::uint64_t level)
{
    const char* name = get_name_lnode(config, lnode);
    print_spaces(f, level);
    std::fprintf(f,
        "\"%s\" [label=\"l-node\", fillcolor=black, style=filled, fontcolor=white, shape=circle, "
        "fixedsize=true, width=0.8]\n",
        name);
    if(lnode->sub_count > 0) {
        print_spaces(f, level);
        std::fprintf(f, "\"%s_subs\" [label=<\\\{", name);
        for(std::size_t i = 0; i < lnode->sub_count; i++) {
            if(i != 0) {
                if(i % 5 == 0) {
                    std::fprintf(f, "<br/>");
                }
                else {
                    std::fprintf(f, ", ");
                }
            }
            std::fprintf(f, "S<sub>%lu</sub>", lnode->subs[i]->id);
        }
        std::fprintf(f, "\\}>, color=lightblue1, fillcolor=lightblue1, style=filled, shape=record]\n");
    }
    bfree((char*)name);
}

static void write_dot_file_cnode_names(
    FILE* f, const struct config* config, const struct cnode* cnode, std::uint64_t level);

static std::uint64_t colspan_value(std::uint64_t level)
{
    if(level == 0) {
        return 1;
    }
    return colspan_value(level - 1) * 2;
}

static void write_dot_file_cdir_td(FILE* f,
    const struct config* config,
    const struct cdir* cdir,
    std::uint64_t level,
    std::size_t current_depth,
    std::uint64_t colspan)
{
    const struct cdir* left = cdir != nullptr ? cdir->lchild : nullptr;
    const struct cdir* right = cdir != nullptr ? cdir->rchild : nullptr;

    if(current_depth == 0) {
        print_spaces(f, level);
        if(cdir == nullptr) {
            std::fprintf(f, "<td colspan=\"%lu\"></td>\n", colspan);
        }
        else {
            const char* name = get_name_cdir(config, cdir);
            switch(cdir->bound.value_type) {
                case(BETREE_INTEGER):
                case(BETREE_INTEGER_LIST):
                    std::fprintf(f,
                        "<td colspan=\"%lu\" port=\"%s\">[%ld, %ld]</td>\n",
                        colspan,
                        name,
                        cdir->bound.imin,
                        cdir->bound.imax);
                    break;
                case(BETREE_FLOAT): {
                    std::fprintf(f,
                        "<td colspan=\"%lu\" port=\"%s\">[%.0f, %.0f]</td>\n",
                        colspan,
                        name,
                        cdir->bound.fmin,
                        cdir->bound.fmax);
                    break;
                }
                case(BETREE_BOOLEAN): {
                    const char* min = cdir->bound.bmin ? "true" : "false";
                    const char* max = cdir->bound.bmax ? "true" : "false";
                    std::fprintf(f,
                        "<td colspan=\"%lu\" port=\"%s\">[%s, %s]</td>\n",
                        colspan,
                        name,
                        min,
                        max);
                    break;
                }
                case(BETREE_STRING):
                case(BETREE_STRING_LIST):
                case(BETREE_INTEGER_ENUM):
                    std::fprintf(f,
                        "<td colspan=\"%lu\" port=\"%s\">[%zu, %zu]</td>\n",
                        colspan,
                        name,
                        cdir->bound.smin,
                        cdir->bound.smax);
                    break;
                case(BETREE_SEGMENTS): {
                    std::fprintf(
                        stderr, "%s a segment value cdir should never happen for now", __func__);
                    std::abort();
                }
                case(BETREE_FREQUENCY_CAPS): {
                    std::fprintf(
                        stderr, "%s a frequency value cdir should never happen for now", __func__);
                    std::abort();
                }
                default:
                    std::abort();
            }
            bfree((char*)name);
        }
    }
    else {
        write_dot_file_cdir_td(f, config, left, level, current_depth - 1, colspan);
        write_dot_file_cdir_td(f, config, right, level, current_depth - 1, colspan);
    }
}

static void write_dot_file_cdir_inner_names(FILE* f,
    const struct config* config,
    const struct cdir* cdir,
    std::uint64_t level,
    std::size_t depth_of_cdir)
{
    for(std::size_t current_depth = 0; current_depth < depth_of_cdir; current_depth++) {
        print_spaces(f, level);
        std::fprintf(f, "<tr>\n");
        level++;
        std::uint64_t colspan = colspan_value(depth_of_cdir - current_depth) / 2;
        write_dot_file_cdir_td(f, config, cdir, level, current_depth, colspan);
        level--;
        print_spaces(f, level);
        std::fprintf(f, "</tr>\n");
    }
}

static void write_dot_file_cdir_cnode_names(
    FILE* f, const struct config* config, const struct cdir* cdir, std::uint64_t level)
{
    if(cdir->cnode != nullptr && should_print_cnode(cdir->cnode)) {
        write_dot_file_cnode_names(f, config, cdir->cnode, level);
    }
    if(cdir->lchild != nullptr) {
        write_dot_file_cdir_cnode_names(f, config, cdir->lchild, level);
    }
    if(cdir->rchild != nullptr) {
        write_dot_file_cdir_cnode_names(f, config, cdir->rchild, level);
    }
}

/*
static void write_dot_file_cdir_cdir_ranks(
    FILE* f, const struct config* config, const struct cdir* cdir, std::uint64_t level);
*/

static std::size_t depth_of_cdir(const struct cdir* cdir)
{
    if(cdir == nullptr) {
        return 0;
    }
    std::size_t current = 1;
    const struct cdir* parent = cdir->parent_type == CNODE_PARENT_CDIR ? cdir->cdir_parent : nullptr;
    while(parent != nullptr) {
        current++;
        if(parent->parent_type == CNODE_PARENT_PNODE) {
            break;
        }
        parent = parent->cdir_parent;
    }
    std::size_t left = depth_of_cdir(cdir->lchild);
    std::size_t right = depth_of_cdir(cdir->rchild);
    std::size_t ret = smax(left, right);
    return smax(ret, current);
}

static void write_dot_file_cdir_names(
    FILE* f, const struct config* config, const struct cdir* cdir, std::uint64_t level)
{
    const char* name = get_name_cdir(config, cdir);
    print_spaces(f, level);
    std::fprintf(f, "subgraph \"cluster%s\" {\n", name);
    level++;
    print_spaces(f, level);
    std::fprintf(f,
        "color=orange; fillcolor=orange; style=filled; label=\"c-directory\"; fontsize=20; "
        "fontname=\"Verdana\"\n");
    print_spaces(f, level);
    std::fprintf(f, "%s [fillcolor=darkolivegreen3, style=filled, shape=box, label=<\n", name);
    level++;
    print_spaces(f, level);
    std::fprintf(f, "<table>\n");
    level++;
    std::size_t depth = depth_of_cdir(cdir);
    write_dot_file_cdir_inner_names(f, config, cdir, level, depth);
    level--;
    print_spaces(f, level);
    std::fprintf(f, "</table>\n");
    level--;
    print_spaces(f, level);
    std::fprintf(f, ">]\n");
    level--;
    print_spaces(f, level);
    std::fprintf(f, "}\n");
    bfree((char*)name);
    write_dot_file_cdir_cnode_names(f, config, cdir, level);
}

static void write_dot_file_pnode_names(
    FILE* f, const struct config* config, const struct pnode* pnode, std::uint64_t level)
{
    const char* name = get_name_pnode(config, pnode);
    print_spaces(f, level);
    std::fprintf(f,
        "\"%s\" [label=\"%s\", color=cyan2, fillcolor=cyan2, style=filled, shape=record]\n",
        name,
        pnode->attr_var.attr);
    print_spaces(f, level);
    std::fprintf(f,
        "\"%s_fake\" [label=\"p-node\", color=cyan2, fillcolor=cyan2, style=filled, shape=circle, "
        "fixedsize=true, width=0.8]\n",
        name);
    bfree((char*)name);
    if(pnode->cdir != nullptr) {
        write_dot_file_cdir_names(f, config, pnode->cdir, level);
    }
}

static void write_dot_file_pdir_inner_names(
    FILE* f, const struct config* config, const struct pdir* pdir, std::uint64_t level)
{
    for(std::size_t i = 0; i < pdir->pnode_count; i++) {
        const struct pnode* pnode = pdir->pnodes[i];
        const char* name = get_name_pnode(config, pnode);
        print_spaces(f, level);
        std::fprintf(f,
            "\"%s\" [label=\"%s\", color=cyan2, fillcolor=cyan2, style=filled, shape=record]\n",
            name,
            pnode->attr_var.attr);
        bfree((char*)name);
    }
}

static void write_dot_file_pdir_names(
    FILE* f, const struct config* config, const struct pdir* pdir, std::uint64_t level)
{
    const char* name = get_name_pdir(config, pdir);
    print_spaces(f, level);
    std::fprintf(f, "subgraph \"cluster%s\" {\n", name);
    level++;
    print_spaces(f, level);
    std::fprintf(f,
        "color=lightpink; fillcolor=lightpink; style=filled; label=\"p-directory\"; fontsize=20; "
        "fontname=\"Verdana\"\n");
    write_dot_file_pdir_inner_names(f, config, pdir, level);
    level--;
    print_spaces(f, level);
    std::fprintf(f, "}\n");
    bfree((char*)name);
    for(std::size_t i = 0; i < pdir->pnode_count; i++) {
        write_dot_file_pnode_names(f, config, pdir->pnodes[i], level);
    }
}

static void write_dot_file_cnode_names(
    FILE* f, const struct config* config, const struct cnode* cnode, std::uint64_t level)
{
    const char* name = get_name_cnode(config, cnode);
    print_spaces(f, level);
    std::fprintf(f,
        "\"%s\" [label=\"c-node\", color=darkolivegreen3, fillcolor=darkolivegreen3, style=filled, "
        "shape=circle, fixedsize=true, width=0.8]\n",
        name);
    bfree((char*)name);
    if(cnode->lnode != nullptr) {
        write_dot_file_lnode_names(f, config, cnode->lnode, level);
    }
    if(cnode->pdir != nullptr) {
        write_dot_file_pdir_names(f, config, cnode->pdir, level);
    }
}

static void write_dot_file_cnode_links(
    FILE* f, const struct config* config, const struct cnode* cnode, std::uint64_t level);

static void write_dot_file_cdir_links(FILE* f,
    const struct config* config,
    const struct cdir* cdir,
    std::uint64_t level,
    const char* table_name)
{
    const char* cdir_name = get_name_cdir(config, cdir);
    if(cdir->cnode != nullptr && should_print_cnode(cdir->cnode)) {
        const char* cnode_name = get_name_cnode(config, cdir->cnode);
        print_spaces(f, level);
        std::fprintf(f, "%s:%s -> \"%s\"\n", table_name, cdir_name, cnode_name);
        bfree((char*)cnode_name);
        write_dot_file_cnode_links(f, config, cdir->cnode, level);
    }
    if(cdir->lchild != nullptr) {
        write_dot_file_cdir_links(f, config, cdir->lchild, level, table_name);
    }
    if(cdir->rchild != nullptr) {
        write_dot_file_cdir_links(f, config, cdir->rchild, level, table_name);
    }
    bfree((char*)cdir_name);
}

static void write_dot_file_pnode_links(
    FILE* f, const struct config* config, const struct pnode* pnode, std::uint64_t level)
{
    if(pnode->cdir != nullptr) {
        const char* pnode_name = get_name_pnode(config, pnode);
        const char* cdir_name = get_name_cdir(config, pnode->cdir);
        print_spaces(f, level);
        std::fprintf(f, "\"%s\" -> \"%s_fake\"\n", pnode_name, pnode_name);
        print_spaces(f, level);
        std::fprintf(
            f, "\"%s_fake\" -> \"%s\" [lhead=\"cluster%s\"]\n", pnode_name, cdir_name, cdir_name);
        bfree((char*)pnode_name);
        write_dot_file_cdir_links(f, config, pnode->cdir, level, cdir_name);
        bfree((char*)cdir_name);
    }
}

static void write_dot_file_pdir_links(
    FILE* f, const struct config* config, const struct pdir* pdir, std::uint64_t level)
{
    for(std::size_t i = 0; i < pdir->pnode_count; i++) {
        const struct pnode* pnode = pdir->pnodes[i];
        write_dot_file_pnode_links(f, config, pnode, level);
    }
}

static void write_dot_file_cnode_links(
    FILE* f, const struct config* config, const struct cnode* cnode, std::uint64_t level)
{
    const char* cnode_name = get_name_cnode(config, cnode);
    if(cnode->lnode != nullptr) {
        const char* lnode_name = get_name_lnode(config, cnode->lnode);
        print_spaces(f, level);
        std::fprintf(f, "\"%s\" -> \"%s\"\n", cnode_name, lnode_name);
        if(cnode->lnode->sub_count > 0) {
            print_spaces(f, level);
            std::fprintf(f, "\"%s\" -> \"%s_subs\"\n", lnode_name, lnode_name);
        }
        bfree((char*)lnode_name);
    }
    if(cnode->pdir != nullptr && cnode->pdir->pnode_count > 0) {
        const char* pdir_name = get_name_pdir(config, cnode->pdir);
        const char* pnode_name = get_name_pnode(config, cnode->pdir->pnodes[0]);
        print_spaces(f, level);
        std::fprintf(f, "\"%s\" -> \"%s\" [lhead=\"cluster%s\"]\n", cnode_name, pnode_name, pdir_name);
        bfree((char*)pdir_name);
        bfree((char*)pnode_name);
        write_dot_file_pdir_links(f, config, cnode->pdir, level);
    }
    bfree((char*)cnode_name);
}

static void write_dot_file_cdir_cnode_ranks(
    FILE* f, const struct config* config, const struct cdir* cdir, std::uint64_t level, bool first)
{
    if(cdir->cnode != nullptr && should_print_cnode(cdir->cnode)) {
        const char* cnode_name = get_name_cnode(config, cdir->cnode);
        if(!first) {
            std::fprintf(f, ", ");
        }
        else {
            first = false;
        }
        std::fprintf(f, "\"%s\"", cnode_name);
        bfree((char*)cnode_name);
    }
    if(cdir->lchild != nullptr) {
        write_dot_file_cdir_cnode_ranks(f, config, cdir->lchild, level, first);
    }
    if(cdir->rchild != nullptr) {
        write_dot_file_cdir_cnode_ranks(f, config, cdir->rchild, level, first);
    }
}

/*
struct cdir_acc {
    std::size_t count;
    struct cdir** cdirs;
};

static void add_cdir(const struct cdir* cdir, struct cdir_acc* acc)
{
    if(acc->count == 0) {
        acc->cdirs = static_cast<struct cdir**>(bcalloc(1, sizeof(*acc->cdirs)));
        if(acc->cdirs == nullptr) {
            std::fprintf(stderr, "%s bcalloc failed", __func__);
            std::abort();
        }
    }
    else {
        auto next_cdirs = static_cast<struct cdir**>(brealloc(acc->cdirs, sizeof(*next_cdirs) * (acc->count + 1)));
        if(next_cdirs == nullptr) {
            std::fprintf(stderr, "%s brealloc failed", __func__);
            std::abort();
        }
        acc->cdirs = next_cdirs;
    }
    acc->cdirs[acc->count] = (struct cdir*)cdir;
    acc->count++;
}

static void get_inner_cdir(const struct cdir* cdir, std::size_t remaining, struct cdir_acc* acc)
{
    if(remaining == 0) {
        add_cdir(cdir, acc);
    }
    else {
        if(cdir->lchild != nullptr) {
            get_inner_cdir(cdir->lchild, remaining - 1, acc);
        }
        if(cdir->rchild != nullptr) {
            get_inner_cdir(cdir->rchild, remaining - 1, acc);
        }
    }
}

static void write_dot_file_cdir_cdir_ranks(
    FILE* f, const struct config* config, const struct cdir* cdir, std::uint64_t level)
{
    std::size_t remaining = 0;
    while(true) {
        struct cdir_acc acc = { .count = 0, .cdirs = nullptr };
        get_inner_cdir(cdir, remaining, &acc);
        if(acc.count > 0) {
            print_spaces(f, level);
            std::fprintf(f, "{ rank=same; ");
            for(std::size_t i = 0; i < acc.count; i++) {
                const char* cdir_name = get_name_cdir(config, acc.cdirs[i]);
                if(i != 0) {
                    std::fprintf(f, ", ");
                }
                std::fprintf(f, "\"%s\"", cdir_name);
            }
            std::fprintf(f, " }\n");
            remaining++;
        }
        else {
            bfree(acc.cdirs);
            return;
        }
    }
}
*/

static void write_dot_file_cnode_ranks(
    FILE* f, const struct config* config, const struct cnode* cnode, std::uint64_t level);

static void write_dot_file_cdir_ranks(
    FILE* f, const struct config* config, const struct cdir* cdir, std::uint64_t level, bool first)
{
    if(first) {
        print_spaces(f, level);
        std::fprintf(f, "{ rank=same; ");
        write_dot_file_cdir_cnode_ranks(f, config, cdir, level, true);
        std::fprintf(f, " }\n");
    }
    if(cdir->cnode != nullptr && should_print_cnode(cdir->cnode)) {
        write_dot_file_cnode_ranks(f, config, cdir->cnode, level);
    }
    if(cdir->lchild != nullptr) {
        write_dot_file_cdir_ranks(f, config, cdir->lchild, level, false);
    }
    if(cdir->rchild != nullptr) {
        write_dot_file_cdir_ranks(f, config, cdir->rchild, level, false);
    }
}

static void write_dot_file_pnode_ranks(
    FILE* f, const struct config* config, const struct pnode* pnode, std::uint64_t level)
{
    if(pnode->cdir != nullptr) {
        write_dot_file_cdir_ranks(f, config, pnode->cdir, level, true);
    }
}

static void write_dot_file_pdir_ranks(
    FILE* f, const struct config* config, const struct pdir* pdir, std::uint64_t level)
{
    print_spaces(f, level);
    std::fprintf(f, "{ rank=same; ");
    for(std::size_t i = 0; i < pdir->pnode_count; i++) {
        const char* pnode_name = get_name_pnode(config, pdir->pnodes[i]);
        if(i != 0) {
            std::fprintf(f, ", ");
        }
        std::fprintf(f, "\"%s_fake\"", pnode_name);
        bfree((char*)pnode_name);
    }
    std::fprintf(f, " }\n");
    for(std::size_t i = 0; i < pdir->pnode_count; i++) {
        write_dot_file_pnode_ranks(f, config, pdir->pnodes[i], level);
    }
}

static void write_dot_file_cnode_ranks(
    FILE* f, const struct config* config, const struct cnode* cnode, std::uint64_t level)
{
    if(cnode->pdir != nullptr) {
        write_dot_file_pdir_ranks(f, config, cnode->pdir, level);
    }
}

struct gathered_subs {
    std::size_t count;
    struct betree_sub** subs;
};

static void add_sub_debug(const struct betree_sub* sub, struct gathered_subs* gatherer)
{
    if(gatherer->count == 0) {
        gatherer->subs = static_cast<struct betree_sub**>(bcalloc(sizeof(*gatherer->subs)));
        if(gatherer->subs == nullptr) {
            std::fprintf(stderr, "%s bcalloc failed", __func__);
            std::abort();
        }
    }
    else {
        auto* subs = static_cast<struct betree_sub**>(brealloc(gatherer->subs, sizeof(struct betree_sub*) * (gatherer->count + 1)));
        if(subs == nullptr) {
            std::fprintf(stderr, "%s brealloc failed", __func__);
            std::abort();
        }
        gatherer->subs = subs;
    }
    gatherer->subs[gatherer->count] = (struct betree_sub*)sub;
    gatherer->count++;
}

static void gather_subs_cnode(const struct cnode* cnode, struct gathered_subs* gatherer);

static void gather_subs_cdir(const struct cdir* cdir, struct gathered_subs* gatherer)
{
    if(cdir->cnode != nullptr) {
        gather_subs_cnode(cdir->cnode, gatherer);
    }
    if(cdir->lchild != nullptr) {
        gather_subs_cdir(cdir->lchild, gatherer);
    }
    if(cdir->rchild != nullptr) {
        gather_subs_cdir(cdir->rchild, gatherer);
    }
}

static void gather_subs_pnode(const struct pnode* pnode, struct gathered_subs* gatherer)
{
    if(pnode->cdir != nullptr) {
        gather_subs_cdir(pnode->cdir, gatherer);
    }
}

static void gather_subs_pdir(const struct pdir* pdir, struct gathered_subs* gatherer)
{
    for(std::size_t i = 0; i < pdir->pnode_count; i++) {
        gather_subs_pnode(pdir->pnodes[i], gatherer);
    }
}

static void gather_subs_cnode(const struct cnode* cnode, struct gathered_subs* gatherer)
{
    for(std::size_t i = 0; i < cnode->lnode->sub_count; i++) {
        add_sub_debug(cnode->lnode->subs[i], gatherer);
    }
    if(cnode->pdir != nullptr) {
        gather_subs_pdir(cnode->pdir, gatherer);
    }
}

static unsigned total;

static unsigned count_slots(const struct cdir* cdir) {
    if (cdir == nullptr) return 0;
    return 1 + count_slots(cdir->lchild) + count_slots(cdir->rchild);
}

static unsigned count_parts(const struct cdir* cdir) {
    if (cdir == nullptr) return 0;
    const struct cnode* cnode = cdir->cnode;
    const struct pdir* pdir = cnode ? cnode->pdir : nullptr;
    unsigned count = pdir ? pdir->pnode_count : 0;
    return count + count_parts(cdir->lchild) + count_parts(cdir->rchild);
}

static unsigned count_subs(const struct cdir* cdir) {
    if (cdir == nullptr) return 0;
    const struct cnode* cnode = cdir->cnode;
    const struct lnode* lnode = cnode ? cnode->lnode : nullptr;
    unsigned count = lnode ? lnode->sub_count : 0;
    return count + count_subs(cdir->lchild) + count_subs(cdir->rchild);
}

static void print_lnode(const struct lnode* lnode, unsigned indent) {
    if (lnode == nullptr || lnode->sub_count == 0) return;
    std::fprintf(stderr, "%*sEXPRs [%lu]\r\n", indent, "", lnode->sub_count);
}

static void print_cnode(const struct cnode* cnode, unsigned indent);
static void print_pdir(const struct pdir* pdir, unsigned indent);

static void print_cdir(const struct cdir* cdir, unsigned indent) {
    if (cdir == nullptr) return;
    if (cdir->parent_type == CNODE_PARENT_PNODE) {
        assert(cdir->attr_var.var == cdir->pnode_parent->attr_var.var);
        unsigned p = count_parts(cdir);
        unsigned n = count_subs(cdir);
        unsigned s = count_slots(cdir);
        if (n > 0) {
            total += n;
            if (p > 0)
                std::fprintf(stderr, " -> %u part(s), %u sub(s) / %u slots\r\n", p, n, s);
            else
                std::fprintf(stderr, " -> %u sub(s) / %u slots\r\n", n, s);
        }
        else if (s > 0) {
            if (p > 0)
                std::fprintf(stderr, " -> %u part(s) / %u slots\r\n", p, s);
            else
                std::fprintf(stderr, " -> %u slots\r\n", s);
        }
        else
            std::fprintf(stderr, "\r\n");
    }
    print_cdir(cdir->lchild, indent);
    print_cdir(cdir->rchild, indent);
    if (cdir->cnode) print_pdir(cdir->cnode->pdir, indent + 2);
}

static void print_pnode(const struct pnode* pnode, unsigned indent) {
    if (pnode == nullptr) return;
    std::fprintf(stderr, "%*sP %s : %f", indent, "", pnode->attr_var.attr, pnode->score);
    print_cdir(pnode->cdir, indent);
}

static void print_pdir(const struct pdir* pdir, unsigned indent) {
    if (pdir == nullptr) return;
    for (std::size_t i = 0; i < pdir->pnode_count; i++)
        print_pnode(pdir->pnodes[i], indent + 2);
}

static void print_cnode(const struct cnode* cnode, unsigned indent) {
    if (cnode == nullptr) return;
    print_lnode(cnode->lnode, indent);
    print_pdir(cnode->pdir, indent);
}

extern "C" {

void wrt_dot_to_file(const struct betree* tree, FILE* f)
{
    const struct config* config = tree->config;
    const struct cnode* root = tree->cnode;
    std::fprintf(f, "digraph {\n");
    std::fprintf(f, "    compound=true");
    std::fprintf(f, "    node [fontsize=20, fontname=\"Verdana\"];\n");
    write_dot_file_cnode_names(f, config, root, 1);
    // write_dot_file_root_subs(f, config, root, 1);
    write_dot_file_cnode_links(f, config, root, 1);
    write_dot_file_cnode_ranks(f, config, root, 1);
    std::fprintf(f, "}\n");
}

void write_dot_to_file(const struct betree* tree, const char* fname)
{
    auto f = std::fopen(fname, "w");
    if(f == nullptr) {
        std::fprintf(stderr, "Can't open file %s  to write the dot_file", fname);
        std::abort();
    }
    wrt_dot_to_file(tree, f);
}

// legacy
void write_dot_file(const struct betree* tree)
{
    auto f = std::fopen("data/betree.dot", "we");
    if(f == nullptr) {
        std::fprintf(stderr, "Can't open file data/betree.dot to write the dot_file");
        std::abort();
    }
    wrt_dot_to_file(tree, f);
    std::fclose(f);
}

void print_betree(const struct betree* betree) {
    if (betree == nullptr) return;
    std::fprintf(stderr, "\r\n");
    total = 0;
    print_cnode(betree->cnode, 0);
    std::fprintf(stderr, "total %u subs\r\n", total);
}

} // extern "C"

#include "debug.h"
