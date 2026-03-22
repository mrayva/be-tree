/**
 * The MIT License (MIT).
 *
 * https://github.com/clibs/red-black-tree
 *
 * Copyright (c) 2003-2014 Julienne Walker (happyfrosty@hotmail.com) [Author]
 * Copyright (c) 2014 clibs, Jonathan Barronville (jonathan@scrapum.photos) [Maintainer], and contributors.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "jsw_rbtree.hpp"

#include <cstdlib>

#include "alloc.h"

#ifndef HEIGHT_LIMIT
#define HEIGHT_LIMIT 64 /* Tallest allowable tree */
#endif

struct jsw_rbnode {
    int red; // 1 = red, 0 = black
    void* data;
    struct jsw_rbnode* link[2]; // left, right
};

struct jsw_rbtree {
    struct jsw_rbnode* root;
    cmp_f cmp;
    std::size_t size;
};

static bool is_red(struct jsw_rbnode* root)
{
    return root != nullptr && root->red == 1;
}

static struct jsw_rbnode* jsw_single(struct jsw_rbnode* root, int dir)
{
    auto* save = root->link[!dir];

    root->link[!dir] = save->link[dir];
    save->link[dir] = root;

    root->red = 1;
    save->red = 0;

    return save;
}

static struct jsw_rbnode* jsw_double(struct jsw_rbnode* root, int dir)
{
    root->link[!dir] = jsw_single(root->link[!dir], !dir);

    return jsw_single(root, dir);
}

static struct jsw_rbnode* new_node(struct jsw_rbtree* tree, void* data)
{
    (void)tree;
    auto* rn = static_cast<struct jsw_rbnode*>(bmalloc(sizeof(struct jsw_rbnode)));

    if (rn == nullptr) {
        return nullptr;
    }

    rn->red = 1;
    rn->data = data;
    rn->link[0] = rn->link[1] = nullptr;

    return rn;
}

extern "C" {

struct jsw_rbtree* jsw_rbnew(cmp_f cmp)
{
    auto* rt = static_cast<struct jsw_rbtree*>(bmalloc(sizeof(struct jsw_rbtree)));

    if (rt == nullptr) {
        return nullptr;
    }

    rt->root = nullptr;
    rt->cmp = cmp;
    rt->size = 0;

    return rt;
}

void jsw_rbdelete(struct jsw_rbtree* tree)
{
    auto* it = tree->root;
    struct jsw_rbnode* save;

    while (it != nullptr) {
        if (it->link[0] == nullptr) {
            save = it->link[1];
            bfree(it);
        }
        else {
            save = it->link[0];
            it->link[0] = save->link[1];
            save->link[1] = it;
        }

        it = save;
    }

    bfree(tree);
}

void* jsw_rbfind(struct jsw_rbtree* tree, void* data)
{
    auto* it = tree->root;

    while (it != nullptr) {
        int cmp = tree->cmp(it->data, data);

        if (cmp == 0) {
            break;
        }

        it = it->link[cmp < 0];
    }

    return it == nullptr ? nullptr : it->data;
}

int jsw_rbinsert(struct jsw_rbtree* tree, void* data)
{
    if (tree->root == nullptr) {
        tree->root = new_node(tree, data);

        if (tree->root == nullptr) {
            return 0;
        }
    }
    else {
        struct jsw_rbnode head = {};
        struct jsw_rbnode *g, *t;
        struct jsw_rbnode *p, *q;
        int dir = 0, last = 0;

        t = &head;
        g = p = nullptr;
        q = t->link[1] = tree->root;

        for (;;) {
            if (q == nullptr) {
                p->link[dir] = q = new_node(tree, data);

                if (q == nullptr) {
                    return 0;
                }
            }
            else if (is_red(q->link[0]) && is_red(q->link[1])) {
                q->red = 1;
                q->link[0]->red = 0;
                q->link[1]->red = 0;
            }

            if (is_red(q) && is_red(p)) {
                int dir2 = t->link[1] == g;

                if (q == p->link[last]) {
                    t->link[dir2] = jsw_single(g, !last);
                }
                else {
                    t->link[dir2] = jsw_double(g, !last);
                }
            }

            if (tree->cmp(q->data, data) == 0) {
                break;
            }

            last = dir;
            dir = tree->cmp(q->data, data) < 0;

            if (g != nullptr) {
                t = g;
            }

            g = p, p = q;
            q = q->link[dir];
        }

        tree->root = head.link[1];
    }

    tree->root->red = 0;
    tree->size++;

    return 1;
}

int jsw_rberase(struct jsw_rbtree* tree, void* data)
{
    if (tree->root != nullptr) {
        struct jsw_rbnode head = {}; /* False tree root */
        struct jsw_rbnode *q, *p, *g; /* Helpers */
        struct jsw_rbnode *f = nullptr;  /* Found item */
        int dir = 1;

        q = &head;
        g = p = nullptr;
        q->link[1] = tree->root;

        while (q->link[dir] != nullptr) {
            int last = dir;

            g = p, p = q;
            q = q->link[dir];
            dir = tree->cmp(q->data, data) < 0;

            if (tree->cmp(q->data, data) == 0) {
                f = q;
            }

            if (!is_red(q) && !is_red(q->link[dir])) {
                if (is_red(q->link[!dir])) {
                    p = p->link[last] = jsw_single(q, dir);
                }
                else if (!is_red(q->link[!dir])) {
                    auto* s = p->link[!last];

                    if (s != nullptr) {
                        if (!is_red(s->link[!last]) && !is_red(s->link[last])) {
                            p->red = 0;
                            s->red = 1;
                            q->red = 1;
                        }
                        else {
                            int dir2 = g->link[1] == p;

                            if (is_red(s->link[last])) {
                                g->link[dir2] = jsw_double(p, last);
                            }
                            else if (is_red(s->link[!last])) {
                                g->link[dir2] = jsw_single(p, last);
                            }

                            q->red = g->link[dir2]->red = 1;
                            g->link[dir2]->link[0]->red = 0;
                            g->link[dir2]->link[1]->red = 0;
                        }
                    }
                }
            }
        }

        if (f != nullptr) {
            f->data = q->data;
            p->link[p->link[1] == q] = q->link[q->link[0] == nullptr];
            bfree(q);
        }

        tree->root = head.link[1];

        if (tree->root != nullptr) {
            tree->root->red = 0;
        }

        tree->size--;
    }

    return 1;
}

std::size_t jsw_rbsize(struct jsw_rbtree* tree)
{
    return tree->size;
}

} // extern "C"
