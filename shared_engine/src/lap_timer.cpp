#include "codriver/lap_timer.h"
#include <cmath>

namespace codriver {

struct LapTimer::Impl {
    double slat1=0, slon1=0, slat2=0, slon2=0;
    double prev_lat=0, prev_lon=0;
    int64_t prev_ts=0, lap_start_ts=0;
    double total_dist=0;    // P1-2: total cumulative, never reset
    double lap_dist=0;      // P1-2: per-lap, reset at crossing
    int lap_count=0;
    bool has_prev=false, line_set=false;
    int cross_dir=0;        // P1-1: accumulated crossing direction
};

LapTimer::LapTimer() : impl_(new Impl()) {}
LapTimer::~LapTimer() { delete impl_; }

void LapTimer::setStartLine(double lat1, double lon1, double lat2, double lon2) {
    impl_->slat1 = lat1; impl_->slon1 = lon1;
    impl_->slat2 = lat2; impl_->slon2 = lon2;
    impl_->line_set = true;
}

int64_t LapTimer::processPoint(double lat, double lon, int64_t ts, double* dist_out, int* dir_out) {
    if (!impl_->line_set) return 0;
    if (dir_out) *dir_out = 0;

    if (!impl_->has_prev) {
        impl_->prev_lat = lat; impl_->prev_lon = lon;
        impl_->prev_ts = ts; impl_->lap_start_ts = ts;
        impl_->lap_dist = 0; impl_->total_dist = 0;
        if (dist_out) *dist_out = 0;
        impl_->has_prev = true;
        return 0;
    }

    constexpr double kM = 111320.0; constexpr double kPI = 3.14159265358979;
    double cos_lat = std::cos((impl_->prev_lat + lat) * 0.5 * kPI / 180.0);
    double dx = (lon - impl_->prev_lon) * kM * cos_lat;
    double dy = (lat - impl_->prev_lat) * kM;
    double seg_dist = std::sqrt(dx*dx + dy*dy);
    impl_->lap_dist += seg_dist;
    impl_->total_dist += seg_dist;
    if (dist_out) *dist_out = impl_->lap_dist;

    // Orientation test for line crossing
    auto orient = [](double ax, double ay, double bx, double by, double cx, double cy) {
        return (bx - ax) * (cy - ay) - (by - ay) * (cx - ax);
    };
    double o1 = orient(impl_->slat1,impl_->slon1,impl_->slat2,impl_->slon2,impl_->prev_lat,impl_->prev_lon);
    double o2 = orient(impl_->slat1,impl_->slon1,impl_->slat2,impl_->slon2,lat,lon);
    double o3 = orient(impl_->prev_lat,impl_->prev_lon,lat,lon,impl_->slat1,impl_->slon1);
    double o4 = orient(impl_->prev_lat,impl_->prev_lon,lat,lon,impl_->slat2,impl_->slon2);

    int64_t lap_time = 0;
    if (o1 * o2 < 0 && o3 * o4 < 0) {
        // P1-1: direction check — forward crossing only
        impl_->cross_dir = (o3 > 0) ? 1 : -1;
        if (impl_->cross_dir > 0) {
            impl_->lap_count++;
            lap_time = ts - impl_->lap_start_ts;
            impl_->lap_start_ts = ts;
            impl_->lap_dist = 0;
            if (dist_out) *dist_out = 0;
            if (dir_out) *dir_out = 1;
        } else {
            if (dir_out) *dir_out = -1;
        }
    }

    impl_->prev_lat = lat; impl_->prev_lon = lon; impl_->prev_ts = ts;
    return lap_time;
}

int LapTimer::lapCount() const { return impl_->lap_count; }
double LapTimer::totalDistance() const { return impl_->total_dist; }
void LapTimer::reset() { *impl_ = Impl{}; }

} // namespace codriver
