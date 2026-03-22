#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>

#include "alloc.h"
#include "betree.h"

template <typename Report>
inline void append_report_sub_id(betree_sub_t id, Report* report)
{
    if(report->matched == 0) {
        report->subs = static_cast<betree_sub_t*>(bcalloc(sizeof(betree_sub_t)));
        if(report->subs == nullptr) {
            std::fprintf(stderr, "%s bcalloc failed", __func__);
            std::abort();
        }
    }
    else {
        auto* subs = static_cast<betree_sub_t*>(
            brealloc(report->subs, sizeof(betree_sub_t) * (report->matched + 1)));
        if(subs == nullptr) {
            std::fprintf(stderr, "%s brealloc failed", __func__);
            std::abort();
        }
        report->subs = subs;
    }
    report->subs[report->matched] = id;
    report->matched++;
}

template <typename Report, typename EvalFn, typename OnMatchFn, typename OnMissFn>
inline void evaluate_subs_shared(const struct subs_to_eval& subs,
    Report* report,
    EvalFn&& eval_sub,
    OnMatchFn&& on_match,
    OnMissFn&& on_miss)
{
    for(std::size_t i = 0; i < subs.count; i++) {
        const struct betree_sub* sub = subs.subs[i];
        report->evaluated++;
        auto result = eval_sub(sub);
        if(result) {
            on_match(sub, result);
        }
        else {
            on_miss(sub, result);
        }
    }
}
