#ifndef CODRIVER_CORNER_SPEED_COMPARE_H
#define CODRIVER_CORNER_SPEED_COMPARE_H

#include "codriver/types.h"

namespace codriver {

// --- 弯速对比结果 ---
struct CornerSpeedDelta {
    const char* segment_id;          // matching TrackSegment.segment_id

    // Entry speed
    double actual_entry_kmh;
    double reference_entry_kmh;      // NaN if no reference
    double entry_delta_kmh;          // actual - reference (>0 = faster)

    // Minimum speed (apex)
    double actual_min_kmh;
    double reference_min_kmh;
    double min_delta_kmh;

    // Exit speed
    double actual_exit_kmh;
    double reference_exit_kmh;
    double exit_delta_kmh;

    // Maximum lateral G
    double actual_lat_g;
    double reference_lat_g;
    double lat_g_delta;
};

// --- 弯速对比器 ---
// Compares actual corner speeds from a completed corner against reference data
// stored in TrackSegment.reference_* fields.
class CornerSpeedCompare {
public:
    CornerSpeedCompare();
    ~CornerSpeedCompare();

    // Compare one corner's actual speeds against reference.
    // Returns a CornerSpeedDelta with deltas. segment_id is copied.
    CornerSpeedDelta compare(
        const TrackSegment& segment,
        double actual_entry_kmh,
        double actual_min_kmh,
        double actual_exit_kmh,
        double actual_lat_g);

    // Bulk compare: process all segments with arrays of actual speeds.
    // Returns number of results populated.
    int compareAll(
        const TrackSegment* segments, int segment_count,
        const double* actual_entry, const double* actual_min,
        const double* actual_exit, const double* actual_lat_g,
        CornerSpeedDelta* results, int max_results);

private:
    class Impl;
    Impl* impl_;
};

} // namespace codriver

#endif
