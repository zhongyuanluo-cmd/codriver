#ifndef CODRIVER_ANALYSIS_PIPELINE_H
#define CODRIVER_ANALYSIS_PIPELINE_H

#include "codriver/types.h"
#include <vector>

namespace codriver {

// Forward declarations
class CornerDetector;
class RootCauseEngine;
class CoachTemplate;
class BrakeDetector;

// --- Pipeline Result (per-corner analysis output) ---
struct PipelineResult {
    char segment_id[32] = {0};

    // Corner data
    double entry_speed_kmh = 0;
    double min_speed_kmh = 0;
    double exit_speed_kmh = 0;
    double max_lat_g = 0;

    // Speed deltas vs reference
    double entry_delta_kmh = 0;
    double min_delta_kmh = 0;
    double exit_delta_kmh = 0;
    double lat_g_delta = 0;

    // Root cause
    char root_cause[32] = {0};
    char root_cause_label[32] = {0};
    char confidence[16] = {0};
    double time_loss_ms = 0;

    // Coach message
    char coach_message[256] = {0};
    int coach_priority = 0;
    int coach_tier = 0;

    // Associated brake event (if any)
    double brake_distance_m = 0;
    double brake_peak_decel_g = 0;
    double brake_speed_drop_kmh = 0;
};

// --- Analysis Pipeline ---
// Orchestrates: EKF → CornerDetector → RootCause → CoachTemplate
// Accepts FusedPoint stream, produces PipelineResult per corner.
class AnalysisPipeline {
public:
    AnalysisPipeline();
    ~AnalysisPipeline();

    // Process a FusedPoint. Returns true when a corner analysis completes
    // (i.e., a PipelineResult is available via getResult).
    bool processPoint(const FusedPoint& point);

    // Get results
    int getResultCount() const;
    const PipelineResult* getResult(int index) const;

    // Flush any pending corner result when session ends.
    // Returns true if a result was flushed (corner was in progress).
    bool finalize();

    // Reset for new session/lap
    void reset();

    // Direct access to sub-modules for C API
    CornerDetector* cornerDetector();
    RootCauseEngine* rootCause();
    CoachTemplate* coachTemplate();

private:
    class Impl;
    Impl* impl_;
};

} // namespace codriver

#endif
