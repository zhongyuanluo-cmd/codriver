#include "codriver/root_cause.h"
#include <cmath>
#include <cstring>
#include <algorithm>

namespace codriver {

struct RuleResult { const char* cause; const char* label; const char* conf; const char* suggestion; double loss; };
struct RootCauseEngine::Impl { RuleResult rules[8]; int count=0; };

RootCauseEngine::RootCauseEngine() : impl_(new Impl()) {}
RootCauseEngine::~RootCauseEngine() { delete impl_; }

RootCauseResult RootCauseEngine::analyze(
    double brake_delta, double entry_delta, double min_delta, double exit_delta,
    double lat_g_ratio, double trail_ms, double line_dev)
{
    auto& r = impl_->rules;
    impl_->count = 0;

    // Rule 1: entry_too_early — brake early + entry slow + min slow + lat_g normal
    if (brake_delta > 3.0 && entry_delta < -2.0 && min_delta < -1.0 && lat_g_ratio > 0.7)
        r[impl_->count++] = {"entry_too_early","入弯太早","high","推迟刹车点", std::abs(brake_delta)*0.05};

    // Rule 2: entry_too_hot_understeer — brake late + entry fast + min slow + lat_g low
    if (brake_delta < -2.0 && entry_delta > 3.0 && min_delta < -1.0 && lat_g_ratio < 0.6)
        r[impl_->count++] = {"entry_too_hot","入弯太快推头","high","提前刹车，入弯减速", std::abs(min_delta)*0.1};

    // Rule 3: throttle_late — brake normal + entry normal + exit slow + accel late
    if (std::abs(brake_delta)<3.0 && std::abs(entry_delta)<3.0 && exit_delta < -2.0)
        r[impl_->count++] = {"throttle_late","出弯给油晚了","medium","早给油，利用出弯抓地力", std::abs(exit_delta)*0.08};

    // Rule 4: conservative_driving — all slow + lat_g low + brake low
    if (entry_delta < -3.0 && min_delta < -3.0 && lat_g_ratio < 0.5 && std::abs(brake_delta)<2.0)
        r[impl_->count++] = {"conservative","保守驾驶","low","用满轮胎抓地力，提升整体节奏", std::abs(min_delta)*0.15};

    // Rule 5: braking_instability — brake pressure fluctuates (no direct metric, use decel stability proxy)
    // We use trail_ms as a proxy: too short trail = unstable
    if (trail_ms < 0.2 && brake_delta > 0 && std::abs(entry_delta) < 5)
        r[impl_->count++] = {"braking_instability","制动不稳定","medium","平稳施压，避免点刹", 0.1};

    // Rule 6: exit_wide_line — min normal + exit slow + lat_g normal + line dev high
    if (std::abs(min_delta)<2.0 && exit_delta < -2.0 && lat_g_ratio > 0.7 && line_dev > 0.5)
        r[impl_->count++] = {"exit_wide","出弯走线偏外","medium","收紧出弯走线，早回正方向", std::abs(exit_delta)*0.08};

    // Rule 7: lack_trail_brake — trail short + entry not fast + min normal
    if (trail_ms < 0.5 && entry_delta < 5.0 && std::abs(min_delta) < 3.0)
        r[impl_->count++] = {"lack_trail_brake","缺少Trail Brake","medium","带刹车入弯，帮车头转向", 0.1};

    // Rule 8: exit_over_throttle — exit speed high but lat_g spikes then drops
    if (exit_delta > 3.0 && lat_g_ratio > 0.9 && line_dev > 0.3)
        r[impl_->count++] = {"exit_over_throttle","出弯油门过大","low","渐进给油，避免后轮打滑", 0.05};

    // Sort by loss descending, then confidence priority
    std::sort(r, r+impl_->count, [](auto& a, auto& b) {
        int pa = (a.conf[0]=='h'?3:(a.conf[0]=='m'?2:1));
        int pb = (b.conf[0]=='h'?3:(b.conf[0]=='m'?2:1));
        return (pa!=pb) ? (pa>pb) : (a.loss>b.loss);
    });

    if (impl_->count > 0) {
        RootCauseResult res;
        res.segment_id = "";  // P0-2: initialize to avoid UB (C API doesn't expose this field)
        res.root_cause = r[0].cause;
        res.root_cause_label = r[0].label;
        res.confidence = r[0].conf;
        res.suggestion = r[0].suggestion;
        res.time_loss_ms = r[0].loss;
        return res;
    }
    return {};
}

} // namespace codriver
