#ifndef CODRIVER_C_API_H
#define CODRIVER_C_API_H

#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

// --- Kalman Filter ---
void* c_kalman_create();
void c_kalman_destroy(void* handle);
void c_kalman_predict(void* handle, double dt);
void c_kalman_update_gps(void* handle, double lat, double lon, double alt,
                         double speed, double heading, double accuracy);
void c_kalman_update_imu(void* handle, double accel_x, double accel_y, double accel_z,
                          double gyro_x, double gyro_y, double gyro_z);
void c_kalman_get_state(void* handle, double* lat, double* lon, double* alt,
                         double* speed, double* heading, double* long_g,
                         double* lat_g, double* vert_g, double* confidence);

// --- Corner Detector ---
void* c_corner_detector_create();
void c_corner_detector_destroy(void* handle);
int c_corner_detector_process_point(void* handle, double distance_m, double lat,
                                     double lon, double speed_kmh, double curvature);
int c_corner_detector_get_segment_count(void* handle);

// --- Root Cause Engine ---
void* c_root_cause_create();
void c_root_cause_destroy(void* handle);
const char* c_root_cause_analyze(void* handle,
    double brake_point_delta_m, double entry_speed_delta,
    double min_speed_delta, double exit_speed_delta,
    double lat_g_ratio, double trail_brake_duration_ms,
    double line_deviation_m);

// --- Coach Template ---
void* c_coach_template_create();
void c_coach_template_destroy(void* handle);
const char* c_coach_template_generate(void* handle,
    const char* segment_id, const char* root_cause,
    double delta_value, int tier);

#ifdef __cplusplus
}
#endif

#endif // CODRIVER_C_API_H
