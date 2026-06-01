#include "codriver/coach_template.h"
#include <cstdio>
#include <cstring>

namespace codriver {

class CoachTemplate::Impl {
public:
    char buffer[256];
};

CoachTemplate::CoachTemplate() : impl_(new Impl()) {}
CoachTemplate::~CoachTemplate() { delete impl_; }

CoachMessage CoachTemplate::generate(const char* segment_id,
                                       const char* root_cause,
                                       double delta_value,
                                       int tier)
{
    CoachMessage msg{};

    // 8-rule template library (analysis-framework.md)
    // Chinese UI layer will handle localization; engine outputs template IDs
    if (std::strcmp(root_cause, "entry_too_early") == 0) {
        std::snprintf(impl_->buffer, sizeof(impl_->buffer),
                      "%s: brake too early, delay by %.0f m", segment_id, delta_value);
    } else if (std::strcmp(root_cause, "entry_too_hot") == 0) {
        std::snprintf(impl_->buffer, sizeof(impl_->buffer),
                      "%s: entry too hot, understeer detected", segment_id);
    } else if (std::strcmp(root_cause, "throttle_late") == 0) {
        std::snprintf(impl_->buffer, sizeof(impl_->buffer),
                      "%s: throttle on too late", segment_id);
    } else {
        std::snprintf(impl_->buffer, sizeof(impl_->buffer),
                      "%s: analysis pending", segment_id);
    }

    msg.text = impl_->buffer;
    msg.tier = tier;
    msg.priority = (tier == 1) ? 0 : 1;

    return msg;
}

} // namespace codriver
