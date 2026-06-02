#ifndef CODRIVER_LAP_TIMER_H
#define CODRIVER_LAP_TIMER_H

#include <cstdint>

namespace codriver {

class LapTimer {
public:
    LapTimer();
    ~LapTimer();

    // Set start/finish line (two GPS points)
    void setStartLine(double lat1, double lon1, double lat2, double lon2);

    // Process a GPS point. Returns lap_time_ms if line crossed, 0 otherwise.
    // track_distance_m is updated to cumulative distance from start.
    int64_t processPoint(double lat, double lon, int64_t timestamp_ms,
                         double* track_distance_m);

    // Current lap count
    int lapCount() const;

    // Total track distance accumulated (meters)
    double totalDistance() const;

    void reset();

private:
    class Impl;
    Impl* impl_;
};

} // namespace codriver

#endif
