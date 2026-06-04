#include "codriver/coach_engine.h"
#include <algorithm>
#include <cstdio>
#include <string>
#include <map>

namespace codriver {

struct CoachEngine::Impl {
    std::vector<PipelineResult> results;
    // vectors must be mutable for lazy sort in const methods
    mutable std::vector<CoachMessage> tier1;
    mutable std::vector<CoachMessage> tier2;
    mutable std::vector<CoachMessage> tier3;

    // Lazy sort: set dirty_ on feed, sort on first access
    mutable bool dirty_ = false;

    // generateLapSummary cache (P0-2: avoid const violation + unbounded growth)
    mutable CoachMessage summary_cache_;
    mutable bool summary_dirty_ = true;

    void classify(const PipelineResult& r) {
        CoachMessage msg;
        msg.tier = r.coach_tier;
        msg.priority = r.coach_priority;
        msg.text = r.coach_message;  // std::string own the text (P0-1)

        switch (r.coach_tier) {
            case 1: tier1.push_back(std::move(msg)); break;
            case 2: tier2.push_back(std::move(msg)); break;
            default: tier3.push_back(std::move(msg)); break;
        }
    }

    void ensureSorted() const {
        if (!dirty_) return;
        auto cmp = [](const CoachMessage& a, const CoachMessage& b) {
            if (a.tier != b.tier) return a.tier < b.tier;
            return a.priority < b.priority;
        };
        std::sort(tier1.begin(), tier1.end(), cmp);
        std::sort(tier2.begin(), tier2.end(), cmp);
        std::sort(tier3.begin(), tier3.end(), cmp);
        dirty_ = false;
    }

    CoachMessage buildSummary(int lapNumber, int64_t lapTimeMs) const {
        CoachMessage msg;
        msg.tier = 3;
        msg.priority = 0;

        // Count root causes — use root_cause_label, fallback to root_cause (P2-1)
        std::map<std::string, int> causeCount;
        double totalLoss = 0;
        for (const auto& r : results) {
            const char* key = r.root_cause_label[0] != '\0'
                ? r.root_cause_label : r.root_cause;
            if (key[0] != '\0') {
                causeCount[key]++;
            }
            totalLoss += r.time_loss_ms;
        }

        // Find the most frequent issue
        std::string topCause = "\u65e0";  // "无"
        int topCount = 0;
        for (const auto& kv : causeCount) {
            if (kv.second > topCount) {
                topCount = kv.second;
                topCause = kv.first;
            }
        }

        int totalCorners = (int)results.size();
        int lapSec = (int)(lapTimeMs / 1000);
        int lapMin = lapSec / 60;
        lapSec %= 60;

        char buf[256];
        if (totalCorners == 0) {
            std::snprintf(buf, sizeof(buf),
                "\u7b2c %d \u5708\u5b8c\u6210\uff0c\u5708\u65f6 %d:%02d\u3002\u672c\u5708\u65e0\u5f2f\u9053\u6570\u636e\u3002",
                lapNumber, lapMin, lapSec);
        } else {
            std::snprintf(buf, sizeof(buf),
                "\u7b2c %d \u5708\u5b8c\u6210\uff0c\u5708\u65f6 %d:%02d\u3002\u5171 %d \u4e2a\u5f2f\u9053\uff0c\u7d2f\u8ba1\u635f\u5931 %.0f ms\u3002\u4e3b\u8981\u95ee\u9898\uff1a%s\u3002",
                lapNumber, lapMin, lapSec, totalCorners, totalLoss, topCause.c_str());
        }

        msg.text = buf;
        return msg;
    }
};

CoachEngine::CoachEngine() : impl_(new Impl()) {}
CoachEngine::~CoachEngine() { delete impl_; }

void CoachEngine::feed(const PipelineResult& result) {
    // P1-3: reserve space for batch feeding
    impl_->results.reserve(impl_->results.size() + 1);
    impl_->results.push_back(result);
    impl_->classify(result);
    impl_->dirty_ = true;       // P1-1: lazy sort
    impl_->summary_dirty_ = true; // P0-2: invalidate cache
}

void CoachEngine::feedBatch(const std::vector<PipelineResult>& results) {
    if (results.empty()) return;
    // P1-3: pre-allocate
    impl_->results.reserve(impl_->results.size() + results.size());
    for (const auto& r : results) {
        impl_->results.push_back(r);
        impl_->classify(r);
    }
    impl_->dirty_ = true;
    impl_->summary_dirty_ = true;
}

const std::vector<CoachMessage>& CoachEngine::tier1Messages() const {
    impl_->ensureSorted();
    return impl_->tier1;
}

const std::vector<CoachMessage>& CoachEngine::tier2Messages() const {
    impl_->ensureSorted();
    return impl_->tier2;
}

const std::vector<CoachMessage>& CoachEngine::tier3Messages() const {
    impl_->ensureSorted();
    return impl_->tier3;
}

std::vector<CoachMessage> CoachEngine::allMessages() const {
    impl_->ensureSorted();
    std::vector<CoachMessage> all;
    all.reserve(impl_->tier1.size() + impl_->tier2.size() + impl_->tier3.size());
    all.insert(all.end(), impl_->tier1.begin(), impl_->tier1.end());
    all.insert(all.end(), impl_->tier2.begin(), impl_->tier2.end());
    all.insert(all.end(), impl_->tier3.begin(), impl_->tier3.end());
    return all;
}

CoachMessage CoachEngine::generateLapSummary(int lapNumber, int64_t lapTimeMs) const {
    // P0-2: cache + dirty flag, no const violation
    if (!impl_->summary_dirty_
        && impl_->summary_cache_.tier == 3
        && !impl_->summary_cache_.text.empty()) {
        return impl_->summary_cache_;
    }
    impl_->summary_cache_ = impl_->buildSummary(lapNumber, lapTimeMs);
    impl_->summary_dirty_ = false;
    return impl_->summary_cache_;
}

void CoachEngine::clear() {
    impl_->results.clear();
    impl_->tier1.clear();
    impl_->tier2.clear();
    impl_->tier3.clear();
    impl_->dirty_ = false;
    impl_->summary_cache_ = CoachMessage{};
    impl_->summary_dirty_ = true;
}

int CoachEngine::messageCount() const {
    return (int)(impl_->tier1.size() + impl_->tier2.size() + impl_->tier3.size());
}

int CoachEngine::tierMessageCount(int tier) const {
    switch (tier) {
        case 1: return (int)impl_->tier1.size();
        case 2: return (int)impl_->tier2.size();
        case 3: return (int)impl_->tier3.size();
        default: return 0;
    }
}

} // namespace codriver
