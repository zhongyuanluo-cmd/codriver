#define _USE_MATH_DEFINES
#include "codriver/kalman_filter.h"
#include "codriver/root_cause.h"
#include <cassert>
#include <cmath>
#include <cstdio>

static bool near(double a, double b, double tol = 0.01) { return std::abs(a - b) < tol; }

int main() {
    printf("CoDriver Engine Tests\n=====================\n");

    // Test 1: create/destroy
    { codriver::KalmanFilter kf; printf("PASS: create/destroy\n"); }

    // Test 2: Stationary — GPS at origin, IMU zero
    {
        codriver::KalmanFilter kf;
        kf.updateGPS(0.0, 0.0, 0.0, 0.0, 0.0, 1.0);
        for (int i = 0; i < 100; i++) {
            kf.updateIMU(0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
            kf.predict(0.01);
        }
        kf.updateGPS(0.0, 0.0, 0.0, 0.0, 0.0, 1.0);
        auto s = kf.getState();
        assert(near(s.latitude, 0.0, 1e-5));
        assert(near(s.speed_kmh, 0.0, 0.1));
        printf("PASS: Stationary — lat=%.6f speed=%.2f\n", s.latitude, s.speed_kmh);
    }

    // Test 3: Straight-line acceleration east
    {
        codriver::KalmanFilter kf;
        kf.updateGPS(0.0, 0.0, 0.0, 0.0, M_PI_2, 1.0);
        for (int i = 0; i < 50; i++) {
            kf.updateIMU(10.0, 0.0, 0.0, 0.0, 0.0, 0.0);
            kf.predict(0.01);
        }
        kf.updateGPS(0.0, 0.001, 0.0, 10.0, M_PI_2, 1.0);
        auto s = kf.getState();
        assert(near(s.speed_kmh, 36.0, 5.0));
        printf("PASS: Straight — speed=%.1f\n", s.speed_kmh);
    }

    // Test 4: Q(7,7) gyro bias stability
    {
        codriver::KalmanFilter kf;
        kf.updateGPS(0.0, 0.0, 0.0, 0.0, 0.0, 1.0);
        for (int i = 0; i < 100; i++) {
            kf.updateIMU(0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
            kf.predict(0.01);
        }
        auto s = kf.getState();
        assert(s.confidence > 0.5);
        printf("PASS: Q stability — confidence=%.3f\n", s.confidence);
    }

    // Test 5: Root Cause
    {
        codriver::RootCauseEngine engine;
        auto r = engine.analyze(8.0, -3.0, -2.0, 0.0, 0.8, 0.5, 0.5);
        assert(r.root_cause != nullptr);
        printf("PASS: Root cause: %s\n", r.root_cause_label);
    }

    printf("\nAll tests passed.\n");
    return 0;
}