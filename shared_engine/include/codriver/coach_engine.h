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
// tier 为主排序键（决定播报时机），priority 为同 tier 内次排序键（0=最高优）
// 消息按 (tier, priority) 升序排列：tier 越小越紧急，同 tier 内 priority 越小越优先
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
    // 按 tier 获取消息队列（延迟排序：首次访问时按需排序，O(1) 摊销）
    const std::vector<CoachMessage>& tier1Messages() const;
    const std::vector<CoachMessage>& tier2Messages() const;
    const std::vector<CoachMessage>& tier3Messages() const;

    // 全部消息（按 tier 再按 priority 排序）
    std::vector<CoachMessage> allMessages() const;

    // 生成圈后摘要（基于收集到的所有 PipelineResult）
    // 重复调用返回相同结果（缓存），feed()/clear() 使缓存失效
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
