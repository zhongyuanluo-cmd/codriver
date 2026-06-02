#ifndef CODRIVER_BRAKE_DETECTOR_H
#define CODRIVER_BRAKE_DETECTOR_H

#include "codriver/types.h"
#include <vector>

namespace codriver {

// --- 制动事件 ---
struct BrakeEvent {
    // Braking point (where deceleration starts)
    double brake_lat, brake_lon;
    double brake_distance_m;
    double brake_speed_kmh;
    int64_t brake_timestamp_ms;

    // Peak braking
    double peak_decel_g;          // maximum negative long_g (e.g., -0.8)
    double peak_decel_distance_m; // track distance at peak decel

    // Release point (where braking ends / corner entry)
    double release_lat, release_lon;
    double release_distance_m;
    double release_speed_kmh;
    int64_t release_timestamp_ms;

    // Metrics
    double braking_duration_ms;       // brake_start → release
    double trail_brake_duration_ms;   // 80%→20% of peak decel release phase
    double brake_release_duration_ms; // 20%→0 release phase
    double speed_drop_kmh;            // brake_speed - release_speed

    // Associated corner (filled by caller / corner_detector integration)
    const char* segment_id;
};

// --- 制动检测器 ---
// Detects braking events from FusedPoint stream using longitudinal G threshold.
// State machine: CRUISING → BRAKING → RELEASING → CRUISING
class BrakeDetector {
public:
    BrakeDetector();
    ~BrakeDetector();

    // Process a fused point. Returns true when a braking event completes
    // (transition RELEASING → CRUISING), i.e., the BrakeEvent is finalized.
    bool processPoint(const FusedPoint& point);

    // Access detected events
    int getEventCount() const;
    const BrakeEvent* getEvent(int index) const;

    // Reset for new lap/session
    void reset();

private:
    class Impl;
    Impl* impl_;
};

} // namespace codriver

#endif
