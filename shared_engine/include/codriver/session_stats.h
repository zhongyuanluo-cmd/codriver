#ifndef CODRIVER_SESSION_STATS_H
#define CODRIVER_SESSION_STATS_H

#include <cstdint>

namespace codriver {

// --- Session Statistics ---
struct SessionStats {
    int total_laps;
    int best_lap_number;
    int64_t best_lap_time_ms;
    int64_t total_time_ms;
    int64_t optimal_lap_time_ms;
    bool has_optimal;
    double avg_speed_kmh;
    double consistency_score;   // 0-100, higher = more consistent lap times
};

// --- Session Statistics Calculator ---
// Computes session-level statistics from lap data.
// Works with BestLapFinder output for lap times and distances.
class SessionStatsCalc {
public:
    SessionStatsCalc();
    ~SessionStatsCalc();

    // Record a lap with its time and distance
    void recordLap(int64_t lap_time_ms, double lap_distance_m);

    // Record a sector time for optimal lap calculation
    void recordSector(int sector_index, int64_t sector_time_ms);

    // Compute session statistics
    SessionStats compute() const;

    // Reset for new session
    void reset();

    // Accessors
    int getLapCount() const;

private:
    class Impl;
    Impl* impl_;
};

} // namespace codriver

#endif
