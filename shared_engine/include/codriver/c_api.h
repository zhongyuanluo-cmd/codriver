#ifndef CODRIVER_C_API_H
#define CODRIVER_C_API_H

#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================
// Ownership & Lifecycle: All handles returned by c_*_create()
// MUST be freed by the matching c_*_destroy(). Handles are
// opaque; do not free() or delete them directly.
// ============================================================

// --- Kalman Filter ---
void* c_kalman_create();
void c_kalman_destroy(void* handle);
void c_kalman_predict(void* handle, double dt);
void c_kalman_update_gps(void* handle, double lat, double lon, double alt,
                         double speed, double heading, double accuracy);
void c_kalman_update_imu(void* handle, double accel_x, double accel_y, double accel_z,
                          double gyro_x, double gyro_y, double gyro_z);

// Prefer CFusedPoint struct over individual output params
typedef struct { double lat, lon, alt, speed, heading, long_g, lat_g, vert_g, confidence; } CFusedPoint;
int c_kalman_get_state(void* handle, CFusedPoint* out);

// --- Corner Detector ---
void* c_corner_detector_create();
void c_corner_detector_destroy(void* handle);
// Returns 1 if a new corner was detected, 0 otherwise (P1-5)
int c_corner_detector_process_point(void* handle, double distance_m, double lat,
                                     double lon, double speed_kmh);
int c_corner_detector_get_segment_count(void* handle);

// P0-2: C-side struct with char arrays, no dangling pointers
typedef struct { char seg_id[8], type[16], dir[8];
    double entry_d, apex_d, exit_d, entry_la, entry_lo, apex_la, apex_lo, exit_la, exit_lo, radius, angle; int diff; } CCornerInfo;
int c_corner_detector_get_segment(void* handle, int index, CCornerInfo* out);

// --- Root Cause Engine ---
void* c_root_cause_create();
void c_root_cause_destroy(void* handle);
// P0-3: C-side struct with char arrays
typedef struct { char cause[32], label[32], conf[16], sugg[128]; double loss; } CRootCause;
int c_root_cause_analyze(void* handle,
    double brake_delta, double entry_delta, double min_delta, double exit_delta,
    double lat_g_ratio, double trail_ms, double line_dev, CRootCause* out);

// --- Coach Template ---
void* c_coach_template_create();
void c_coach_template_destroy(void* handle);
int c_coach_template_generate(void* handle, const char* segment_id, const char* root_cause,
                               double delta_value, int tier, char* out_message, int max_len);

// --- Lap Timer (P1-4) ---
void* c_lap_timer_create();
void c_lap_timer_destroy(void* handle);
void c_lap_timer_set_line(void* handle, double lat1, double lon1, double lat2, double lon2);
// Returns lap_time_ms if start/finish crossed, 0 otherwise. dist_out=per-lap distance, direction=+1/-1/0
int64_t c_lap_timer_process(void* handle, double lat, double lon, int64_t timestamp_ms, double* dist_out, int* direction);
int c_lap_timer_count(void* handle);
double c_lap_timer_total_dist(void* handle);

// --- Coord Transform (Phase 2) ---
void* c_coord_transform_create();
void c_coord_transform_destroy(void* handle);
// Calibrate phone orientation: pass averaged accelerometer reading when phone is stationary.
// Returns 1 on success, 0 if gravity out of plausible range.
int c_coord_transform_calibrate(void* handle, double ax, double ay, double az);
// Transform phone-frame acceleration to car-frame G-force.
// Returns 0 on success, -1 if not calibrated or null pointers.
int c_coord_transform_transform(void* handle, double ax, double ay, double az,
                                 double* car_long_g, double* car_lat_g,
                                 double* car_vert_g);
int c_coord_transform_is_calibrated(void* handle);
// Detect orientation drift (GPS vs IMU heading mismatch). Static, no handle needed.
// Returns 1 if drift > 15°, 0 otherwise. Pass nullptr for handle.
int c_coord_transform_detect_drift(void* handle, double gps_heading, double imu_heading);

// --- Brake Detector (Phase 2.2) ---
// C-side BrakeEvent struct (char arrays, no dangling pointers)
typedef struct { double brake_lat, brake_lon, brake_dist, brake_spd, peak_g, peak_dist;
    double rel_lat, rel_lon, rel_dist, rel_spd;
    double dur_ms, trail_ms, release_ms, speed_drop; int64_t brake_ts, release_ts;
    char seg_id[32]; } CBrakeEvent;
void* c_brake_detector_create();
void c_brake_detector_destroy(void* handle);
// Returns 1 if a braking event was finalized, 0 otherwise
int c_brake_detector_process_point(void* handle, double lat, double lon, double dist,
                                    double speed, double long_g, int64_t ts);
int c_brake_detector_get_event_count(void* handle);
int c_brake_detector_get_event(void* handle, int index, CBrakeEvent* out);
void c_brake_detector_reset(void* handle);

#ifdef __cplusplus
}
#endif

#endif // CODRIVER_C_API_H
