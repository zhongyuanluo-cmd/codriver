#ifndef CODRIVER_COACH_ENGINE_H
#define CODRIVER_COACH_ENGINE_H

#include "codriver/analysis_pipeline.h"
#include "codriver/types.h"
#include <vector>
#include <cstdint>

namespace codriver {

// --- Coach Engine ---
// 反馈分层引擎：收集 PipelineResult，按 Tier 分类，为 TTS 和调度器提供消息队列。
//
// Tier 1 (<200ms 即时): 弯道入弯/弯速警告 — 车手需要马上反应
// Tier 2 (直道期间): 制动/走线分析建议 — 可以在直道段播报
// Tier 3 (圈后): 摘要总结 — 圈结束后统一播报
class CoachEngine {
public:
    CoachEngine();
    ~CoachEngine();

    // --- 输入 ---
    // 喂入单条 Pipeline 分析结果
    void feed(const PipelineResult& result);

    // 批量喂入（通常是一整圈的结果）
    void feedBatch(const std::vector<PipelineResult>& results);

    // --- 输出 ---
    // 按 Tier 获取消息队列（已按优先级排序）
    std::vector<CoachMessage> tier1Messages() const;
    std::vector<CoachMessage> tier2Messages() const;
    std::vector<CoachMessage> tier3Messages() const;

    // 获取全部消息（按 tier 再按 priority 排序）
    std::vector<CoachMessage> allMessages() const;

    // 生成圈后摘要（基于收集到的所有 PipelineResult）
    CoachMessage generateLapSummary(int lapNumber, int64_t lapTimeMs) const;

    // --- 管理 ---
    void clear();
    int messageCount() const;
    int tierMessageCount(int tier) const;

private:
    class Impl;
    Impl* impl_;
};

} // namespace codriver

#endif
