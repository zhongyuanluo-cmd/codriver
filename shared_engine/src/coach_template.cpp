#include "codriver/coach_template.h"
#include <cstdio>
#include <cstring>
#include <string>

namespace codriver {

struct CoachTemplate::Impl {
    std::string buffer;  // P1-4: use std::string, no dangling pointer
};

CoachTemplate::CoachTemplate() : impl_(new Impl()) {}
CoachTemplate::~CoachTemplate() { delete impl_; }

CoachMessage CoachTemplate::generate(const char* segment_id,
                                       const char* root_cause,
                                       double delta_value,
                                       int tier)
{
    CoachMessage msg{};
    char tmp[256];

    // 8-rule template library (analysis-framework.md)
    if (std::strcmp(root_cause, "entry_too_early") == 0) {
        std::snprintf(tmp, sizeof(tmp), "%s: brake too early, delay by %.0f m", segment_id, delta_value);
    } else if (std::strcmp(root_cause, "entry_too_hot") == 0) {
        std::snprintf(tmp, sizeof(tmp), "%s: entry too hot, understeer - brake earlier", segment_id);
    } else if (std::strcmp(root_cause, "throttle_late") == 0) {
        std::snprintf(tmp, sizeof(tmp), "%s: throttle on too late, apply earlier", segment_id);
    } else if (std::strcmp(root_cause, "conservative") == 0) {
        std::snprintf(tmp, sizeof(tmp), "%s: driving too conservatively, push harder", segment_id);
    } else if (std::strcmp(root_cause, "braking_instability") == 0) {
        std::snprintf(tmp, sizeof(tmp), "%s: braking unstable, apply smooth pressure", segment_id);
    } else if (std::strcmp(root_cause, "exit_wide") == 0) {
        std::snprintf(tmp, sizeof(tmp), "%s: exit line too wide, tighten your line", segment_id);
    } else if (std::strcmp(root_cause, "lack_trail_brake") == 0) {
        std::snprintf(tmp, sizeof(tmp), "%s: try trail braking into the corner", segment_id);
    } else if (std::strcmp(root_cause, "exit_over_throttle") == 0) {
        std::snprintf(tmp, sizeof(tmp), "%s: too much throttle at exit, be smoother", segment_id);
    } else {
        std::snprintf(tmp, sizeof(tmp), "%s: analysis pending", segment_id);
    }

    impl_->buffer = tmp;
    msg.text = impl_->buffer.c_str();
    msg.tier = tier;
    msg.priority = (tier == 1) ? 0 : (tier == 2 ? 1 : 2);

    return msg;
}

} // namespace codriver
