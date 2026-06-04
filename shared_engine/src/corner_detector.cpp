#include "codriver/corner_detector.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>

namespace codriver {

// --- Haversine distance between two WGS84 points (meters) ---
// Accurate at all latitudes; replaces naive lat/lon Euclidean distance.
static double haversineMeters(double lat1, double lon1, double lat2, double lon2) {
    constexpr double kEarthRadiusM = 6371000.0;
    constexpr double kDeg2Rad = 3.14159265358979323846 / 180.0;
    double dlat = (lat2 - lat1) * kDeg2Rad;
    double dlon = (lon2 - lon1) * kDeg2Rad;
    double a = std::sin(dlat * 0.5) * std::sin(dlat * 0.5) +
               std::cos(lat1 * kDeg2Rad) * std::cos(lat2 * kDeg2Rad) *
               std::sin(dlon * 0.5) * std::sin(dlon * 0.5);
    double c = 2.0 * std::atan2(std::sqrt(a), std::sqrt(1.0 - a));
    return kEarthRadiusM * c;
}

enum class CornerState { STRAIGHT, ENTERING, IN_CORNER, EXITING };

struct CornerData {
    std::vector<double> lats, lons, distances, speeds, curvatures;
    double max_curv = 0; int max_idx = 0;
    double cross_sum = 0;  // L-11: accumulated raw cross product for turn direction
    void reset() { lats.clear(); lons.clear(); distances.clear();
                   speeds.clear(); curvatures.clear(); max_curv=0; max_idx=0; cross_sum=0; }
};

struct CornerDetector::Impl {
    std::vector<TrackSegment> segments;
    CornerState state = CornerState::STRAIGHT;
    CornerData current;
    int seg_count = 0;
    std::vector<std::string> id_store;  // New-P0-1: owns segment_id strings

    // Menger Curvature window: 3 points P_{k-1}, P_k, P_{k+1}
    struct { double d[3], lat[3], lon[3]; int idx=0; } win;

    static constexpr double kCurvEntryThresh = 0.02;  // curvature to trigger corner entry
    static constexpr double kCurvExitThresh  = 0.005; // curvature to exit corner
    static constexpr int    kMinCornerPts    = 10;    // min points in a corner
    static constexpr double kMinSpeedKmh    = 5.0;   // ignore corners below this speed
};

CornerDetector::CornerDetector() : impl_(new Impl()) {}
CornerDetector::~CornerDetector() { delete impl_; }

void CornerDetector::processPoint(double dist, double lat, double lon, double speed) {
    // Update 3-point window
    auto& w = impl_->win;
    w.d[0] = w.d[1]; w.d[1] = w.d[2]; w.d[2] = dist;
    w.lat[0] = w.lat[1]; w.lat[1] = w.lat[2]; w.lat[2] = lat;
    w.lon[0] = w.lon[1]; w.lon[1] = w.lon[2]; w.lon[2] = lon;
    if (++w.idx < 3) return;

    // L-12: Menger Curvature (P0-1: Haversine distance for spatial accuracy)
    double d1 = haversineMeters(w.lat[0], w.lon[0], w.lat[1], w.lon[1]);
    double d2 = haversineMeters(w.lat[1], w.lon[1], w.lat[2], w.lon[2]);
    double d3 = haversineMeters(w.lat[0], w.lon[0], w.lat[2], w.lon[2]);
    // Cross product still uses lat/lon diff in degrees for sign only (direction)
    double dx1 = w.lat[1]-w.lat[0], dy1 = w.lon[1]-w.lon[0];
    double dx2 = w.lat[2]-w.lat[1], dy2 = w.lon[2]-w.lon[1];
    double cross = dx1*dy2 - dy1*dx2;  // L-11: raw cross (not abs) for direction
    double curv = (d1*d2*d3 > 1e-9) ? (2.0*std::abs(cross)/(d1*d2*d3)) : 0.0;

    auto& s = impl_->state;
    auto& cd = impl_->current;

    // State machine for corner detection
    if (s == CornerState::STRAIGHT || s == CornerState::EXITING) {
        if (curv > Impl::kCurvEntryThresh && speed > Impl::kMinSpeedKmh) {
            s = CornerState::ENTERING;
            cd.reset();
            cd.lats.push_back(lat); cd.lons.push_back(lon);
            cd.distances.push_back(dist); cd.speeds.push_back(speed);
            cd.curvatures.push_back(curv);
            cd.cross_sum = cross;  // L-11: start accumulating
        }
    } else if (s == CornerState::ENTERING || s == CornerState::IN_CORNER) {
        cd.lats.push_back(lat); cd.lons.push_back(lon);
        cd.distances.push_back(dist); cd.speeds.push_back(speed);
        cd.curvatures.push_back(curv);
        cd.cross_sum += cross;  // L-11: accumulate raw cross

        if (curv > cd.max_curv) { cd.max_curv = curv; cd.max_idx = (int)cd.curvatures.size()-1; }
        if (s == CornerState::ENTERING && cd.curvatures.size() > 3) s = CornerState::IN_CORNER;

        // Exit condition: curvature drops below threshold for 3 consecutive points
        int n = (int)cd.curvatures.size();
        if (n >= 3 && curv < Impl::kCurvExitThresh &&
            cd.curvatures[n-2] < Impl::kCurvExitThresh &&
            cd.curvatures[n-3] < Impl::kCurvExitThresh &&
            cd.curvatures.size() >= (size_t)Impl::kMinCornerPts) {
            s = CornerState::EXITING;
            // Create TrackSegment
            TrackSegment seg{};
            char buf[8]; std::snprintf(buf,sizeof(buf),"T%d",++impl_->seg_count);
            impl_->id_store.push_back(buf);      // New-P0-1: persist in Impl
            seg.segment_id = impl_->id_store.back().c_str();
            seg.segment_type = "corner";

            // L-11: direction from accumulated cross sum (positive=right, negative=left)
            seg.turn_direction = (cd.cross_sum > 0) ? "right" : "left";

            // Entry / Apex / Exit
            seg.entry_distance_m = cd.distances.front();
            seg.apex_distance_m = cd.distances[cd.max_idx];
            seg.exit_distance_m = cd.distances.back();
            seg.entry_lat = cd.lats.front(); seg.entry_lon = cd.lons.front();
            seg.apex_lat = cd.lats[cd.max_idx]; seg.apex_lon = cd.lons[cd.max_idx];
            seg.exit_lat = cd.lats.back(); seg.exit_lon = cd.lons.back();

            // Estimate radius from curvature: R ≈ 1/κ_max
            seg.radius_m = (cd.max_curv > 0.001) ? (1.0/cd.max_curv) : 100.0;
            // Estimate angle from entry→exit Haversine distance and radius
            double entry_exit_dist = haversineMeters(
                seg.entry_lat, seg.entry_lon, seg.exit_lat, seg.exit_lon);
            // P1-1: accurate angle via 2*arcsin(chord/(2*R))
            constexpr double kPI = 3.14159265358979323846;
            double half_chord = entry_exit_dist * 0.5;
            seg.angle_deg = (seg.radius_m > 1.0)
                ? (2.0 * std::asin(std::min(1.0, half_chord / seg.radius_m)) * 180.0 / kPI)
                : 90.0;
            if (seg.angle_deg > 180) seg.angle_deg = 180;

            // Reference data: NaN (auto-detected, no reference yet)
            seg.reference_speed_kmh = std::numeric_limits<double>::quiet_NaN();
            seg.reference_brake_point_m = std::numeric_limits<double>::quiet_NaN();
            seg.reference_entry_speed_kmh = std::numeric_limits<double>::quiet_NaN();
            seg.reference_exit_speed_kmh = std::numeric_limits<double>::quiet_NaN();
            seg.reference_lateral_g = std::numeric_limits<double>::quiet_NaN();

            // Difficulty heuristic: tighter radius + larger angle = harder
            double tight = (seg.radius_m < 30)?4:(seg.radius_m<60?3:(seg.radius_m<100?2:1));
            double ang = (seg.angle_deg>120?4:(seg.angle_deg>80?3:(seg.angle_deg>40?2:1)));
            int diff = std::max(1, std::min(5, (int)((tight+ang)/2)));
            seg.difficulty = diff;

            impl_->segments.push_back(seg);
            s = CornerState::STRAIGHT;
        }
    }
}

std::vector<TrackSegment> CornerDetector::getSegments() const { return impl_->segments; }
int CornerDetector::getSegmentCount() const { return static_cast<int>(impl_->segments.size()); }
void CornerDetector::reset() { impl_->segments.clear(); impl_->id_store.clear(); impl_->seg_count=0; impl_->state=CornerState::STRAIGHT; }

} // namespace codriver
