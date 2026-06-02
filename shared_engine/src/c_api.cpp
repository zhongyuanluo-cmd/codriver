#include "codriver/c_api.h"
#include "codriver/kalman_filter.h"
#include "codriver/corner_detector.h"
#include "codriver/root_cause.h"
#include "codriver/coach_template.h"
#include "codriver/types.h"
#include <cstring>

extern "C" {

// ============================================================
// Kalman Filter
// ============================================================
void* c_kalman_create() { return new codriver::KalmanFilter(); }
void c_kalman_destroy(void* h) { delete static_cast<codriver::KalmanFilter*>(h); }
void c_kalman_predict(void* h, double dt) { static_cast<codriver::KalmanFilter*>(h)->predict(dt); }
void c_kalman_update_gps(void* h, double lat, double lon, double alt,
                          double spd, double hdg, double acc) {
    static_cast<codriver::KalmanFilter*>(h)->updateGPS(lat, lon, alt, spd, hdg, acc);
}
void c_kalman_update_imu(void* h, double ax, double ay, double az,
                          double gx, double gy, double gz) {
    static_cast<codriver::KalmanFilter*>(h)->updateIMU(ax, ay, az, gx, gy, gz);
}
int c_kalman_get_state(void* h, CFusedPoint* out) {
    if (!out) return -1;
    auto fp = static_cast<codriver::KalmanFilter*>(h)->getState();
    out->lat = fp.latitude; out->lon = fp.longitude;
    out->alt = fp.altitude_m; out->speed = fp.speed_kmh;
    out->heading = fp.heading_deg; out->long_g = fp.long_g;
    out->lat_g = fp.lat_g; out->vert_g = fp.vert_g;
    out->confidence = fp.confidence;
    return 0;
}

// ============================================================
// Corner Detector
// ============================================================
void* c_corner_detector_create() { return new codriver::CornerDetector(); }
void c_corner_detector_destroy(void* h) { delete static_cast<codriver::CornerDetector*>(h); }
int c_corner_detector_process_point(void* h, double d, double lat, double lon, double spd) {
    static_cast<codriver::CornerDetector*>(h)->processPoint(d, lat, lon, spd);
    return 0;  // TODO: return 1 when new corner detected (add to CornerDetector API)
}
int c_corner_detector_get_segment_count(void* h) {
    return static_cast<codriver::CornerDetector*>(h)->getSegmentCount();
}
int c_corner_detector_get_segment(void* h, int idx, void* out) {
    if (!out || idx < 0) return -1;
    auto segs = static_cast<codriver::CornerDetector*>(h)->getSegments();
    if (idx >= (int)segs.size()) return -1;
    std::memcpy(out, &segs[idx], sizeof(codriver::TrackSegment));
    return 0;
}

// ============================================================
// Root Cause Engine
// ============================================================
void* c_root_cause_create() { return new codriver::RootCauseEngine(); }
void c_root_cause_destroy(void* h) { delete static_cast<codriver::RootCauseEngine*>(h); }
int c_root_cause_analyze(void* h, double bd, double ed, double md, double xd,
                          double lg, double trail, double line, void* out) {
    if (!out) return -1;
    auto result = static_cast<codriver::RootCauseEngine*>(h)->analyze(bd, ed, md, xd, lg, trail, line);
    std::memcpy(out, &result, sizeof(codriver::RootCauseResult));
    return 0;
}

// ============================================================
// Coach Template
// ============================================================
void* c_coach_template_create() { return new codriver::CoachTemplate(); }
void c_coach_template_destroy(void* h) { delete static_cast<codriver::CoachTemplate*>(h); }
int c_coach_template_generate(void* h, const char* seg, const char* cause,
                               double delta, int tier, char* buf, int len) {
    if (!buf || len <= 0) return 0;
    auto msg = static_cast<codriver::CoachTemplate*>(h)->generate(seg, cause, delta, tier);
    int n = std::snprintf(buf, len, "%s", msg.text ? msg.text : "");
    return (n < len) ? n : len - 1;
}

} // extern "C"
