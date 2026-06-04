#include "codriver/coach_engine.h"
#include <algorithm>
#include <cstdio>
#include <string>
#include <map>

namespace codriver {

struct CoachEngine::Impl {
    std::vector<PipelineResult> results;
    std::vector<CoachMessage> tier1;
    std::vector<CoachMessage> tier2;
    std::vector<CoachMessage> tier3;

    // Owned strings for coach messages (avoid dangling pointers)
    std::vector<std::string> buffers;

    void classify(const PipelineResult& r) {
        CoachMessage msg{};
        msg.tier = r.coach_tier;
        msg.priority = r.coach_priority;

        // Copy coach message text into owned buffer
        buffers.emplace_back(r.coach_message);
        msg.text = buffers.back().c_str();

        switch (r.coach_tier) {
            case 1: tier1.push_back(msg); break;
            case 2: tier2.push_back(msg); break;
            default: tier3.push_back(msg); break;
        }
    }

    void sortAll() {
        auto cmp = [](const CoachMessage& a, const CoachMessage& b) {
            if (a.tier != b.tier) return a.tier < b.tier;
            return a.priority < b.priority;
        };
        std::sort(tier1.begin(), tier1.end(), cmp);
        std::sort(tier2.begin(), tier2.end(), cmp);
        std::sort(tier3.begin(), tier3.end(), cmp);
    }
};

CoachEngine::CoachEngine() : impl_(new Impl()) {}
CoachEngine::~CoachEngine() { delete impl_; }

void CoachEngine::feed(const PipelineResult& result) {
    impl_->results.push_back(result);
    impl_->classify(result);
    impl_->sortAll();
}

void CoachEngine::feedBatch(const std::vector<PipelineResult>& results) {
    for (const auto& r : results) {
        impl_->results.push_back(r);
        impl_->classify(r);
    }
    impl_->sortAll();
}

std::vector<CoachMessage> CoachEngine::tier1Messages() const {
    return impl_->tier1;
}

std::vector<CoachMessage> CoachEngine::tier2Messages() const {
    return impl_->tier2;
}

std::vector<CoachMessage> CoachEngine::tier3Messages() const {
    return impl_->tier3;
}

std::vector<CoachMessage> CoachEngine::allMessages() const {
    std::vector<CoachMessage> all;
    all.insert(all.end(), impl_->tier1.begin(), impl_->tier1.end());
    all.insert(all.end(), impl_->tier2.begin(), impl_->tier2.end());
    all.insert(all.end(), impl_->tier3.begin(), impl_->tier3.end());
    return all;
}

CoachMessage CoachEngine::generateLapSummary(int lapNumber, int64_t lapTimeMs) const {
    CoachMessage msg{};
    msg.tier = 3;
    msg.priority = 0;

    // Count root causes
    std::map<std::string, int> causeCount;
    double totalLoss = 0;
    for (const auto& r : impl_->results) {
        if (r.root_cause[0] != '\0') {
            causeCount[r.root_cause]++;
        }
        totalLoss += r.time_loss_ms;
    }

    // Find the most frequent issue
    std::string topCause = "无";
    int topCount = 0;
    for (const auto& kv : causeCount) {
        if (kv.second > topCount) {
            topCount = kv.second;
            topCause = kv.first;
        }
    }

    int totalCorners = (int)impl_->results.size();
    int lapSec = (int)(lapTimeMs / 1000);
    int lapMin = lapSec / 60;
    lapSec %= 60;

    char buf[256];
    if (totalCorners == 0) {
        std::snprintf(buf, sizeof(buf),
            "第 %d 圈完成，圈时 %d:%02d。本圈无弯道数据。",
            lapNumber, lapMin, lapSec);
    } else {
        std::snprintf(buf, sizeof(buf),
            "第 %d 圈完成，圈时 %d:%02d。共 %d 个弯道，累计损失 %.0f ms。主要问题：%s。",
            lapNumber, lapMin, lapSec, totalCorners, totalLoss, topCause.c_str());
    }

    impl_->buffers.emplace_back(buf);
    msg.text = impl_->buffers.back().c_str();

    return msg;
}

void CoachEngine::clear() {
    impl_->results.clear();
    impl_->tier1.clear();
    impl_->tier2.clear();
    impl_->tier3.clear();
    impl_->buffers.clear();
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
