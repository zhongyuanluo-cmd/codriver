#include "codriver/best_lap_finder.h"
#include <algorithm>
#include <limits>
#include <vector>

namespace codriver {

class BestLapFinder::Impl {
public:
    std::vector<LapRecord> laps;
    int next_lap_number = 1;

    // Sector tracking for optimal lap calculation
    std::vector<int64_t> best_sector_times;  // index → best time
    bool has_sector_data = false;
};

BestLapFinder::BestLapFinder() : impl_(new Impl()) {}
BestLapFinder::~BestLapFinder() { delete impl_; }

bool BestLapFinder::recordLap(int64_t lap_time_ms, double lap_distance_m) {
    LapRecord rec{};
    rec.lap_number = impl_->next_lap_number++;
    rec.lap_time_ms = lap_time_ms;
    rec.lap_distance_m = lap_distance_m;
    rec.avg_speed_kmh = lap_distance_m > 0
        ? (lap_distance_m / 1000.0) / (lap_time_ms / 3600000.0)
        : 0.0;
    impl_->laps.push_back(rec);
    return true;
}

BestLapResult BestLapFinder::getBest() const {
    BestLapResult result{};

    result.total_laps = static_cast<int>(impl_->laps.size());
    if (impl_->laps.empty()) return result;

    // Find fastest lap
    int64_t best_time = std::numeric_limits<int64_t>::max();
    int best_lap = 0;
    int64_t total_time = 0;

    for (const auto& lap : impl_->laps) {
        total_time += lap.lap_time_ms;
        if (lap.lap_time_ms < best_time) {
            best_time = lap.lap_time_ms;
            best_lap = lap.lap_number;
        }
    }

    result.best_lap_number = best_lap;
    result.best_lap_time_ms = best_time;
    result.total_time_ms = total_time;

    // Theoretical optimal: sum of best sector times
    if (impl_->has_sector_data && !impl_->best_sector_times.empty()) {
        int64_t optimal = 0;
        for (auto t : impl_->best_sector_times) {
            optimal += t;
        }
        result.optimal_lap_time_ms = optimal;
        result.has_optimal = true;
    }

    return result;
}

int BestLapFinder::getLapCount() const {
    return static_cast<int>(impl_->laps.size());
}

const LapRecord* BestLapFinder::getLap(int index) const {
    if (index < 0 || index >= static_cast<int>(impl_->laps.size())) return nullptr;
    return &impl_->laps[index];
}

void BestLapFinder::recordSector(int sector_index, int64_t sector_time_ms) {
    // P1-6: sector bounds check
    if (sector_index < 0 || sector_index >= 64) return;
    impl_->has_sector_data = true;
    if (sector_index >= static_cast<int>(impl_->best_sector_times.size())) {
        impl_->best_sector_times.resize(sector_index + 1,
            std::numeric_limits<int64_t>::max());
    }
    if (sector_time_ms < impl_->best_sector_times[sector_index]) {
        impl_->best_sector_times[sector_index] = sector_time_ms;
    }
}

void BestLapFinder::reset() {
    impl_->laps.clear();
    impl_->next_lap_number = 1;
    impl_->best_sector_times.clear();
    impl_->has_sector_data = false;
}

} // namespace codriver
