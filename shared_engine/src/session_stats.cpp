#include "codriver/session_stats.h"
#include "codriver/best_lap_finder.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

namespace codriver {

class SessionStatsCalc::Impl {
public:
    struct LapData {
        int64_t time_ms;
        double distance_m;
    };
    std::vector<LapData> laps;
    std::vector<int64_t> best_sector_times;
};

SessionStatsCalc::SessionStatsCalc() : impl_(new Impl()) {}
SessionStatsCalc::~SessionStatsCalc() { delete impl_; }

void SessionStatsCalc::recordLap(int64_t lap_time_ms, double lap_distance_m) {
    impl_->laps.push_back({lap_time_ms, lap_distance_m});
}

void SessionStatsCalc::recordSector(int sector_index, int64_t sector_time_ms) {
    if (sector_index < 0 || sector_index >= 64) return;
    if (sector_index >= static_cast<int>(impl_->best_sector_times.size())) {
        impl_->best_sector_times.resize(sector_index + 1,
            std::numeric_limits<int64_t>::max());
    }
    if (sector_time_ms < impl_->best_sector_times[sector_index]) {
        impl_->best_sector_times[sector_index] = sector_time_ms;
    }
}

SessionStats SessionStatsCalc::compute() const {
    SessionStats s{};
    s.total_laps = static_cast<int>(impl_->laps.size());

    // Optimal lap from best sectors (computed even without laps)
    if (!impl_->best_sector_times.empty()) {
        int64_t optimal = 0;
        bool all_valid = true;
        for (auto t : impl_->best_sector_times) {
            if (t == std::numeric_limits<int64_t>::max()) {
                all_valid = false;
                break;
            }
            optimal += t;
        }
        if (all_valid) {
            s.optimal_lap_time_ms = optimal;
            s.has_optimal = true;
        }
    }

    if (impl_->laps.empty()) return s;

    // Best lap
    int64_t best_time = std::numeric_limits<int64_t>::max();
    int best_lap = 0;
    int64_t total_time = 0;
    double total_dist = 0;

    for (size_t i = 0; i < impl_->laps.size(); i++) {
        total_time += impl_->laps[i].time_ms;
        total_dist += impl_->laps[i].distance_m;
        if (impl_->laps[i].time_ms < best_time) {
            best_time = impl_->laps[i].time_ms;
            best_lap = static_cast<int>(i) + 1;
        }
    }

    s.best_lap_number = best_lap;
    s.best_lap_time_ms = best_time;
    s.total_time_ms = total_time;

    // Average speed (weighted by distance)
    s.avg_speed_kmh = total_dist > 0
        ? (total_dist / 1000.0) / (total_time / 3600000.0)
        : 0.0;

    // Optimal lap from best sectors
    if (!impl_->best_sector_times.empty()) {
        int64_t optimal = 0;
        bool all_valid = true;
        for (auto t : impl_->best_sector_times) {
            if (t == std::numeric_limits<int64_t>::max()) {
                all_valid = false;
                break;
            }
            optimal += t;
        }
        if (all_valid) {
            s.optimal_lap_time_ms = optimal;
            s.has_optimal = true;
        }
    }

    // Consistency score: based on standard deviation of lap times
    // Lower std dev = higher consistency. Scored 0-100.
    if (impl_->laps.size() >= 2) {
        double mean = static_cast<double>(total_time) / impl_->laps.size();
        double sum_sq = 0;
        for (const auto& lap : impl_->laps) {
            double diff = static_cast<double>(lap.time_ms) - mean;
            sum_sq += diff * diff;
        }
        double std_dev = std::sqrt(sum_sq / impl_->laps.size());
        // Normalize: CV = std_dev/mean, score = max(0, 100 - CV*200)
        double cv = mean > 0 ? std_dev / mean : 1.0;
        s.consistency_score = std::max(0.0, 100.0 - cv * 200.0);
    } else {
        s.consistency_score = 0;
    }

    return s;
}

SessionStats SessionStatsCalc::compute(const BestLapFinder& blf) const {
    SessionStats s{};
    BestLapResult best = blf.getBest();
    if (best.total_laps == 0) return s;

    s.total_laps = best.total_laps;
    s.best_lap_time_ms = best.best_lap_time_ms;
    s.best_lap_number = best.best_lap_number;
    s.total_time_ms = best.total_time_ms;

    // Average speed: estimate from best lap
    if (best.best_lap_time_ms > 0) {
        // Use a default lap length estimate (4 km) for speed calc
        double est_dist = 4000.0;
        s.avg_speed_kmh = (est_dist / 1000.0) / ((double)best.best_lap_time_ms / 3600000.0);
    }

    // Optimal lap from best sector times
    s.optimal_lap_time_ms = best.optimal_lap_time_ms;
    s.has_optimal = best.has_optimal;

    // Consistency: neutral for single lap, computed for multi
    if (best.total_laps > 1) {
        // iterate laps to compute consistency
        double mean = (double)best.total_time_ms / best.total_laps;
        double sum_sq = 0.0;
        for (int i = 0; i < best.total_laps; i++) {
            const LapRecord* rec = blf.getLap(i);
            if (rec) {
                double diff = (double)rec->lap_time_ms - mean;
                sum_sq += diff * diff;
            }
        }
        double std_dev = std::sqrt(sum_sq / best.total_laps);
        double cv = mean > 0 ? std_dev / mean : 1.0;
        s.consistency_score = std::max(0.0, 100.0 - cv * 200.0);
    } else {
        s.consistency_score = 50.0;  // single lap neutral
    }
    return s;
}

void SessionStatsCalc::reset() {
    impl_->laps.clear();
    impl_->best_sector_times.clear();
}

int SessionStatsCalc::getLapCount() const {
    return static_cast<int>(impl_->laps.size());
}

} // namespace codriver
