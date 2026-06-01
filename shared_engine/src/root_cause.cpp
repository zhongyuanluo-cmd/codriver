#include "codriver/root_cause.h"
#include <cmath>
#include <cstring>

namespace codriver {

class RootCauseEngine::Impl {};

RootCauseEngine::RootCauseEngine() : impl_(new Impl()) {}
RootCauseEngine::~RootCauseEngine() { delete impl_; }

RootCauseResult RootCauseEngine::analyze(
    double brake_point_delta_m, double entry_speed_delta,
    double min_speed_delta, double exit_speed_delta,
    double lat_g_ratio, double trail_brake_duration_ms,
    double line_deviation_m)
{
    RootCauseResult result{};

    // TODO: 8-rule inference engine from analysis-framework.md
    // Rule 1: brake_point early + entry slow + min slow + lat_g normal → entry_too_early
    if (brake_point_delta_m > 3.0 && entry_speed_delta < -2.0 &&
        min_speed_delta < -1.0 && lat_g_ratio > 0.7) {
        result.root_cause = "entry_too_early";
        result.root_cause_label = "入弯太早";
        result.confidence = "high";
    }
    // Rule 2-8: TODO

    return result;
}

} // namespace codriver
