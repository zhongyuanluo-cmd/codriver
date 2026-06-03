#include "codriver/analysis_pipeline.h"
#include "codriver/corner_detector.h"
#include "codriver/root_cause.h"
#include "codriver/coach_template.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <vector>

namespace codriver {

// Pipeline state machine for corner tracking
enum class PipeState {
    STRAIGHT,    // on straight, waiting for corner
    IN_CORNER,   // inside a corner, accumulating data
    CORNER_DONE  // corner just finished, result available
};

class AnalysisPipeline::Impl {
public:
    CornerDetector corner_detector;
    RootCauseEngine root_cause;
    CoachTemplate coach;

    std::vector<PipelineResult> results;

    // State machine
    PipeState state = PipeState::STRAIGHT;

    // Per-corner data accumulation
    double corner_entry_speed = 0;
    double corner_min_speed = 999.0;
    double corner_exit_speed = 0;
    double corner_max_lat_g = 0;
    double corner_start_dist = 0;

    // Pending result (set on CORNER_DONE, consumed by caller)
    PipelineResult pending_result;
    bool has_pending = false;
};

AnalysisPipeline::AnalysisPipeline() : impl_(new Impl()) {}
AnalysisPipeline::~AnalysisPipeline() { delete impl_; }

bool AnalysisPipeline::processPoint(const FusedPoint& point) {
    // Feed to corner detector
    int prev_count = impl_->corner_detector.getSegmentCount();
    impl_->corner_detector.processPoint(
        point.track_distance_m, point.latitude, point.longitude, point.speed_kmh);
    int new_count = impl_->corner_detector.getSegmentCount();

    switch (impl_->state) {
    case PipeState::STRAIGHT:
        if (new_count > prev_count) {
            // Entering first corner
            impl_->state = PipeState::IN_CORNER;
            impl_->corner_entry_speed = point.speed_kmh;
            impl_->corner_min_speed = point.speed_kmh;
            impl_->corner_max_lat_g = std::abs(point.lat_g);
            impl_->corner_start_dist = point.track_distance_m;
        }
        break;

    case PipeState::IN_CORNER:
        // Track min speed and max lat_g
        if (point.speed_kmh < impl_->corner_min_speed) {
            impl_->corner_min_speed = point.speed_kmh;
        }
        if (std::abs(point.lat_g) > impl_->corner_max_lat_g) {
            impl_->corner_max_lat_g = std::abs(point.lat_g);
        }
        impl_->corner_exit_speed = point.speed_kmh;

        // Corner completion: new segment detected (next corner) or
        // lat_g drops below threshold after being in corner
        if (new_count > prev_count) {
            // New corner starting → previous corner is complete
            impl_->state = PipeState::CORNER_DONE;
        }
        // Note: In production, also use lat_g threshold fallback:
        // else if (std::abs(point.lat_g) < 0.15 && impl_->corner_max_lat_g > 0.3) {
        //     impl_->state = PipeState::CORNER_DONE;
        // }
        break;

    case PipeState::CORNER_DONE:
        // Should not normally reach here — result consumed before next point
        break;
    }

    // Produce result when corner done
    if (impl_->state == PipeState::CORNER_DONE) {
        auto segs = impl_->corner_detector.getSegments();
        int seg_idx = new_count - 1;  // last completed segment
        if (seg_idx >= 0 && seg_idx < static_cast<int>(segs.size())) {
            const auto& seg = segs[seg_idx];

            PipelineResult r{};
            std::snprintf(r.segment_id, sizeof(r.segment_id), "%s",
                seg.segment_id ? seg.segment_id : "");

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

            // Root cause (P1-2: TODO — trail_brake and line_deviation are placeholders;
            // will be replaced with real BrakeDetector data in Phase 2.6)
            auto rc = impl_->root_cause.analyze(
                0.0,
                r.entry_delta_kmh,
                r.min_delta_kmh,
                r.exit_delta_kmh,
                seg.reference_lateral_g > 0 ? r.max_lat_g / seg.reference_lateral_g : 1.0,
                0.5,   // TODO: trail_brake — integrate BrakeDetector
                0.3);  // TODO: line_deviation — implement line scoring

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
                2);
            std::snprintf(r.coach_message, sizeof(r.coach_message), "%s",
                msg.text ? msg.text : "");
            r.coach_priority = msg.priority;
            r.coach_tier = msg.tier;

            impl_->results.push_back(r);
            impl_->has_pending = true;
        }

        // Transition: CORNER_DONE → IN_CORNER (new corner already detected)
        // or → STRAIGHT (if this was the last corner)
        impl_->corner_entry_speed = point.speed_kmh;
        impl_->corner_min_speed = point.speed_kmh;
        impl_->corner_max_lat_g = std::abs(point.lat_g);
        impl_->state = (new_count > 0) ? PipeState::IN_CORNER : PipeState::STRAIGHT;
        return true;
    }

    return false;
}

bool AnalysisPipeline::finalize() {
    if (impl_->state != PipeState::IN_CORNER) return false;

    // Flush the pending corner — same result production logic as CORNER_DONE
    auto segs = impl_->corner_detector.getSegments();
    int seg_idx = static_cast<int>(segs.size()) - 1;
    if (seg_idx >= 0 && seg_idx < static_cast<int>(segs.size())) {
        const auto& seg = segs[seg_idx];

        PipelineResult r{};
        std::snprintf(r.segment_id, sizeof(r.segment_id), "%s",
            seg.segment_id ? seg.segment_id : "");

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

        // Root cause
        auto rc = impl_->root_cause.analyze(
            0.0,
            r.entry_delta_kmh,
            r.min_delta_kmh,
            r.exit_delta_kmh,
            seg.reference_lateral_g > 0 ? r.max_lat_g / seg.reference_lateral_g : 1.0,
            0.5,   // TODO: trail_brake
            0.3);  // TODO: line_deviation

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
            2);
        std::snprintf(r.coach_message, sizeof(r.coach_message), "%s",
            msg.text ? msg.text : "");
        r.coach_priority = msg.priority;
        r.coach_tier = msg.tier;

        impl_->results.push_back(r);
        impl_->has_pending = true;
    }

    impl_->state = PipeState::STRAIGHT;
    return true;
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
    impl_->state = PipeState::STRAIGHT;
    impl_->corner_min_speed = 999.0;
    impl_->corner_max_lat_g = 0;
    impl_->has_pending = false;
}

CornerDetector* AnalysisPipeline::cornerDetector() { return &impl_->corner_detector; }
RootCauseEngine* AnalysisPipeline::rootCause() { return &impl_->root_cause; }
CoachTemplate* AnalysisPipeline::coachTemplate() { return &impl_->coach; }

} // namespace codriver
