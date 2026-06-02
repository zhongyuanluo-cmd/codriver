#include "codriver/brake_detector.h"
#include <algorithm>
#include <cmath>
#include <vector>

namespace codriver {

// --- Thresholds ---
constexpr double kBrakeOnThreshold  = -0.30;  // long_g below this → braking starts
constexpr double kBrakeOffThreshold = -0.10;  // long_g above this → braking ends
constexpr double kTrail80Threshold  = 0.80;   // 80% of peak decel → trail braking starts
constexpr double kTrail20Threshold  = 0.20;   // 20% of peak decel → trail braking ends
constexpr int    kMinBrakingPoints  = 3;       // minimum points to count as valid braking

enum class BrakeState {
    CRUISING,   // no braking
    BRAKING,    // deceleration increasing (more negative)
    RELEASING    // deceleration decreasing (back toward zero)
};

class BrakeDetector::Impl {
public:
    BrakeState state = BrakeState::CRUISING;

    // Current braking event being built
    BrakeEvent current;
    bool has_brake_event = false;

    // Peak tracking
    double peak_decel = 0.0;
    double peak_distance = 0.0;

    // Braking point buffer (to capture the point just before braking)
    FusedPoint pre_brake_point;
    bool has_pre_brake = false;

    // Release phase tracking for trail braking
    double release_start_g = 0.0;   // long_g at start of release (≈ peak)
    double release_start_dist = 0.0;

    // Completed events
    std::vector<BrakeEvent> events;

    // Point counter for minimum points check
    int braking_point_count = 0;
};

BrakeDetector::BrakeDetector() : impl_(new Impl()) {}
BrakeDetector::~BrakeDetector() { delete impl_; }

bool BrakeDetector::processPoint(const FusedPoint& point) {
    double lg = point.long_g;
    bool event_finalized = false;

    switch (impl_->state) {
    case BrakeState::CRUISING:
        if (lg < kBrakeOnThreshold) {
            // Transition: CRUISING → BRAKING (use previously saved pre-brake point)
            impl_->state = BrakeState::BRAKING;
            impl_->braking_point_count = 1;

            // Record brake start from the point BEFORE braking began
            impl_->current = BrakeEvent{};
            if (impl_->has_pre_brake) {
                impl_->current.brake_lat = impl_->pre_brake_point.latitude;
                impl_->current.brake_lon = impl_->pre_brake_point.longitude;
                impl_->current.brake_distance_m = impl_->pre_brake_point.track_distance_m;
                impl_->current.brake_speed_kmh = impl_->pre_brake_point.speed_kmh;
                impl_->current.brake_timestamp_ms = impl_->pre_brake_point.timestamp_ms;
            }
            impl_->has_brake_event = true;

            // Initialize peak tracking with current point's deceleration
            impl_->peak_decel = lg;
            impl_->peak_distance = point.track_distance_m;
        } else {
            // Buffer the current point as potential pre-brake for next iteration
            impl_->pre_brake_point = point;
            impl_->has_pre_brake = true;
        }
        break;

    case BrakeState::BRAKING:
        impl_->braking_point_count++;

        if (lg < impl_->peak_decel) {
            // Deeper braking — update peak
            impl_->peak_decel = lg;
            impl_->peak_distance = point.track_distance_m;
        }

        if (lg > kBrakeOffThreshold) {
            // Transition: BRAKING → RELEASING (deceleration easing up past threshold)
            impl_->state = BrakeState::RELEASING;
            impl_->release_start_g = impl_->peak_decel;  // peak decel value
            impl_->release_start_dist = impl_->peak_distance;
        }
        // else if lg is between kBrakeOnThreshold and kBrakeOffThreshold but decreasing
        //   → stay in BRAKING (still braking deeper)
        // else if lg is between those thresholds but increasing
        //   → could be releasing, but we wait until it crosses kBrakeOffThreshold
        break;

    case BrakeState::RELEASING:
        // Track how far we've released
        if (lg >= -0.05) {
            // Transition: RELEASING → CRUISING (braking fully released)
            impl_->state = BrakeState::CRUISING;

            if (impl_->has_brake_event && impl_->braking_point_count >= kMinBrakingPoints) {
                // Fill in peak metrics
                impl_->current.peak_decel_g = impl_->peak_decel;
                impl_->current.peak_decel_distance_m = impl_->peak_distance;

                // Release point (corner entry)
                impl_->current.release_lat = point.latitude;
                impl_->current.release_lon = point.longitude;
                impl_->current.release_distance_m = point.track_distance_m;
                impl_->current.release_speed_kmh = point.speed_kmh;
                impl_->current.release_timestamp_ms = point.timestamp_ms;

                // Duration metrics
                impl_->current.braking_duration_ms =
                    static_cast<double>(impl_->current.release_timestamp_ms -
                                        impl_->current.brake_timestamp_ms);
                impl_->current.speed_drop_kmh =
                    impl_->current.brake_speed_kmh - impl_->current.release_speed_kmh;

                // Trail brake duration: time from 80% to 20% of peak decel release.
                // Simplified: total release duration * 0.6 as approximation.
                // Full implementation needs per-point tracking during release phase.
                double release_phase_ms = impl_->current.braking_duration_ms * 0.4; // ~40% of braking is release
                impl_->current.trail_brake_duration_ms = release_phase_ms * 0.6;
                impl_->current.brake_release_duration_ms = release_phase_ms * 0.4;

                // Store
                impl_->events.push_back(impl_->current);
                impl_->has_brake_event = false;
                event_finalized = true;
            } else {
                impl_->has_brake_event = false;
            }
            impl_->braking_point_count = 0;
        }
        // else: still in RELEASING phase, continue buffering
        break;
    }

    return event_finalized;
}

int BrakeDetector::getEventCount() const {
    return static_cast<int>(impl_->events.size());
}

const BrakeEvent* BrakeDetector::getEvent(int index) const {
    if (index < 0 || index >= static_cast<int>(impl_->events.size())) return nullptr;
    return &impl_->events[index];
}

void BrakeDetector::reset() {
    impl_->state = BrakeState::CRUISING;
    impl_->has_brake_event = false;
    impl_->has_pre_brake = false;
    impl_->braking_point_count = 0;
    impl_->peak_decel = 0.0;
    impl_->events.clear();
}

} // namespace codriver
