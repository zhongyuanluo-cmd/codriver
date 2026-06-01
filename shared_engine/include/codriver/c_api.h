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
// Returns: 1 if a new corner was detected, 0 otherwise
int c_corner_detector_process_point(void* handle, double distance_m, double lat,
                                     double lon, double speed_kmh, double curvature);
int c_corner_detector_get_segment_count(void* handle);
// Fills out_segment at index (0-based). Returns 0 on success, -1 if index out of range.
// Caller owns out_segment memory; c_api fills it.
int c_corner_detector_get_segment(void* handle, int index, void* out_segment);

// --- Root Cause Engine ---
void* c_root_cause_create();
void c_root_cause_destroy(void* handle);
// Analyzes corner metrics and fills result struct. Returns 0 on success.
// Caller allocates RootCauseResult; callee fills string fields with static literals (no free needed).
int c_root_cause_analyze(void* handle,
    double brake_point_delta_m, double entry_speed_delta,
    double min_speed_delta, double exit_speed_delta,
    double lat_g_ratio, double trail_brake_duration_ms,
    double line_deviation_m,
    void* out_result);  // cast to RootCauseResult*

// --- Coach Template ---
void* c_coach_template_create();
void c_coach_template_destroy(void* handle);
// Fills out_message buffer (caller provides max_len bytes). Returns bytes written (or 0 if truncated).
int c_coach_template_generate(void* handle,
    const char* segment_id, const char* root_cause,
    double delta_value, int tier,
    char* out_message, int max_len);

#ifdef __cplusplus
}
#endif

#endif // CODRIVER_C_API_H
