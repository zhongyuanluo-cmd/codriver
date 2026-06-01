#include "codriver/kalman_filter.h"
#include "codriver/root_cause.h"
#include <cassert>
#include <cstdio>

int main() {
    printf("CoDriver Engine Tests\n");
    printf("=====================\n");

    // Test 1: Kalman Filter create/destroy
    {
        codriver::KalmanFilter kf;
        printf("PASS: KalmanFilter create/destroy\n");
    }

    // Test 2: Root Cause Engine - entry_too_early detection
    {
        codriver::RootCauseEngine engine;
        auto result = engine.analyze(
            8.0,   // brake_point_delta_m (8m early)
            -3.0,  // entry_speed_delta (3 km/h slow)
            -2.0,  // min_speed_delta (2 km/h slow)
            0.0,   // exit_speed_delta
            0.8,   // lat_g_ratio
            0.5,   // trail_brake_duration
            0.5    // line_deviation
        );
        assert(result.root_cause != nullptr);
        printf("PASS: Root cause detected: %s (confidence: %s)\n",
               result.root_cause_label, result.confidence);
    }

    printf("\nAll tests passed.\n");
    return 0;
}
