#include "codriver/analysis_pipeline.h"
#include "codriver/corner_detector.h"
#include "codriver/root_cause.h"
#include "codriver/coach_template.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <vector>

namespace codriver {

class AnalysisPipeline::Impl {
public:
    CornerDetector corner_detector;
    RootCauseEngine root_cause;
    CoachTemplate coach;

    std::vector<PipelineResult> results;

    // Track per-corner data accumulation
    double corner_entry_speed = 0;
    double corner_min_speed = 999.0;
    double corner_exit_speed = 0;
    double corner_max_lat_g = 0;
    bool in_corner = false;
};

AnalysisPipeline::AnalysisPipeline() : impl_(new Impl()) {}
AnalysisPipeline::~AnalysisPipeline() { delete impl_; }

bool AnalysisPipeline::processPoint(const FusedPoint& point) {
    // Feed to corner detector
    int prev_count = impl_->corner_detector.getSegmentCount();
    impl_->corner_detector.processPoint(
        point.track_distance_m, point.latitude, point.longitude, point.speed_kmh);
    int new_count = impl_->corner_detector.getSegmentCount();

    // Track corner speed data
    if (!impl_->in_corner && new_count > prev_count) {
        // Entering a new corner
        impl_->in_corner = true;
        impl_->corner_entry_speed = point.speed_kmh;
        impl_->corner_min_speed = point.speed_kmh;
        impl_->corner_max_lat_g = std::abs(point.lat_g);
    }

    if (impl_->in_corner) {
        // Track min speed and max lat_g during corner
        if (point.speed_kmh < impl_->corner_min_speed) {
            impl_->corner_min_speed = point.speed_kmh;
        }
        if (std::abs(point.lat_g) > impl_->corner_max_lat_g) {
            impl_->corner_max_lat_g = std::abs(point.lat_g);
        }
    }

    // When a corner is completed (new segment detected, previous corner finished)
    if (impl_->in_corner && prev_count > 0 && new_count == prev_count) {
        // Corner exit detected (no new segment, still in corner area)
        impl_->corner_exit_speed = point.speed_kmh;
    }

    // Check if we've exited the corner (state machine would be better,
    // but simplified: detect when corner_detector enters straight)
    // For now, produce result when a new corner is detected
    // (meaning the previous corner is complete)
    if (impl_->in_corner && new_count > prev_count && prev_count > 0) {
        // Previous corner completed, new corner starting
        impl_->in_corner = false;

        // Get the last completed segment
        auto segs = impl_->corner_detector.getSegments();
        if (!segs.empty()) {
            const auto& seg = segs.back();

            PipelineResult r{};
            std::snprintf(r.segment_id, sizeof(r.segment_id), "%s",
                seg.segment_id ? seg.segment_id : "");

            // Speed data
            r.entry_speed_kmh = impl_->corner_entry_speed;
            r.min_speed_kmh = impl_->corner_min_speed;
            r.exit_speed_kmh = impl_->corner_exit_speed;
            r.max_lat_g = impl_->corner_max_lat_g;

            // Speed deltas (NaN-safe)
            r.entry_delta_kmh = std::isnan(seg.reference_entry_speed_kmh) ? 0.0
                : r.entry_speed_kmh - seg.reference_entry_speed_kmh;
            r.min_delta_kmh = std::isnan(seg.reference_speed_kmh) ? 0.0
                : r.min_speed_kmh - seg.reference_speed_kmh;
            r.exit_delta_kmh = std::isnan(seg.reference_exit_speed_kmh) ? 0.0
                : r.exit_speed_kmh - seg.reference_exit_speed_kmh;
            r.lat_g_delta = std::isnan(seg.reference_lateral_g) ? 0.0
                : r.max_lat_g - seg.reference_lateral_g;

            // Root cause analysis
            auto rc = impl_->root_cause.analyze(
                0.0,                    // brake_delta (not available here)
                r.entry_delta_kmh,      // entry speed delta
                r.min_delta_kmh,        // min speed delta
                r.exit_delta_kmh,       // exit speed delta
                seg.reference_lateral_g > 0 ? r.max_lat_g / seg.reference_lateral_g : 1.0,
                0.5,                    // trail_brake (placeholder)
                0.3);                   // line_deviation (placeholder)

            std::snprintf(r.root_cause, sizeof(r.root_cause), "%s",
                rc.root_cause ? rc.root_cause : "");
            std::snprintf(r.root_cause_label, sizeof(r.root_cause_label), "%s",
                rc.root_cause_label ? rc.root_cause_label : "");
            std::snprintf(r.confidence, sizeof(r.confidence), "%s",
                rc.confidence ? rc.confidence : "");
            r.time_loss_ms = rc.time_loss_ms;

            // Coach message
            auto msg = impl_->coach.generate(
                r.segment_id, r.root_cause,
                std::max(std::abs(r.entry_delta_kmh),
                    std::max(std::abs(r.min_delta_kmh), std::abs(r.exit_delta_kmh))),
                2);  // Tier 2: straight analysis
            std::snprintf(r.coach_message, sizeof(r.coach_message), "%s",
                msg.text ? msg.text : "");
            r.coach_priority = msg.priority;
            r.coach_tier = msg.tier;

            impl_->results.push_back(r);

            // Reset for next corner
            impl_->corner_entry_speed = point.speed_kmh;
            impl_->corner_min_speed = point.speed_kmh;
            impl_->corner_max_lat_g = 0;
            impl_->in_corner = true;
            return true;
        }
    }

    return false;
}

int AnalysisPipeline::getResultCount() const {
    return static_cast<int>(impl_->results.size());
}

const PipelineResult* AnalysisPipeline::getResult(int index) const {
    if (index < 0 || index >= static_cast<int>(impl_->results.size())) return nullptr;
    return &impl_->results[index];
}

void AnalysisPipeline::reset() {
    impl_->corner_detector.reset();
    impl_->results.clear();
    impl_->in_corner = false;
    impl_->corner_min_speed = 999.0;
    impl_->corner_max_lat_g = 0;
}

CornerDetector* AnalysisPipeline::cornerDetector() { return &impl_->corner_detector; }
RootCauseEngine* AnalysisPipeline::rootCause() { return &impl_->root_cause; }
CoachTemplate* AnalysisPipeline::coachTemplate() { return &impl_->coach; }

} // namespace codriver
