#ifndef CODRIVER_BEST_LAP_FINDER_H
#define CODRIVER_BEST_LAP_FINDER_H

#include <cstdint>
#include <vector>

namespace codriver {

// --- Lap Record ---
struct LapRecord {
    int lap_number;
    int64_t lap_time_ms;
    double lap_distance_m;
    double avg_speed_kmh;
};

// --- Best Lap Result ---
struct BestLapResult {
    int best_lap_number;
    int64_t best_lap_time_ms;
    int total_laps;
    int64_t total_time_ms;

    // Theoretical optimal: sum of best sector times across all laps
    int64_t optimal_lap_time_ms;
    bool has_optimal;  // true if enough data for optimal calculation
};

// --- Best Lap Finder ---
// Tracks lap times and identifies the fastest lap.
// Can also compute the theoretical optimal lap from best sector times.
class BestLapFinder {
public:
    BestLapFinder();
    ~BestLapFinder();

    // Record a completed lap. Returns true.
    bool recordLap(int64_t lap_time_ms, double lap_distance_m);

    // Get best lap result
    BestLapResult getBest() const;

    // Get all lap records
    int getLapCount() const;
    const LapRecord* getLap(int index) const;

    // Record sector time for optimal lap calculation.
    // sector_index: 0-based sector in current lap.
    // If sector time is better than previous best for this sector, update.
    void recordSector(int sector_index, int64_t sector_time_ms);

    // Reset for new session
    void reset();

private:
    class Impl;
    Impl* impl_;
};

} // namespace codriver

#endif
