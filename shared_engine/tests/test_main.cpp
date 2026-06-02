#include "codriver/kalman_filter.h"
#include "codriver/root_cause.h"
#include "codriver/coord_transform.h"
#include "codriver/brake_detector.h"
#include <cassert>
#include <cmath>
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

    // Test 3: CoordTransform — calibrate + transform (phone flat, screen up)
    {
        codriver::CoordTransform ct;
        // Phone flat: gravity along -z in phone frame (~0, 0, -9.81)
        bool ok = ct.calibrate(0.0, 0.0, -9.81);
        assert(ok);
        assert(ct.isCalibrated());

        double clg = 0, clat = 0, cv = 0;
        // Simulate 0.5g forward acceleration (phone +X axis = car forward)
        int rc = ct.transform(0.5 * 9.81, 0.0, -9.81, &clg, &clat, &cv);
        assert(rc == 0);
        // Forward accel should map to car longitudinal (~0.5g)
        assert(std::abs(clg - 0.5) < 0.1);
        assert(std::abs(clat) < 0.1);
        printf("PASS: CoordTransform flat-phone: long_g=%.3f lat_g=%.3f vert_g=%.3f\n", clg, clat, cv);
    }

    // Test 4: CoordTransform — uncalibrated returns -1 + zeros
    {
        codriver::CoordTransform ct;
        assert(!ct.isCalibrated());
        double clg = 99, clat = 99, cv = 99;
        int rc = ct.transform(1.0, 2.0, 3.0, &clg, &clat, &cv);
        assert(rc == -1);
        assert(clg == 0.0);
        assert(clat == 0.0);
        assert(cv == 0.0);
        printf("PASS: CoordTransform uncalibrated: returned %d, outputs zeroed\n", rc);
    }

    // Test 5: CoordTransform — detectDrift (static)
    {
        // 20° difference → drift detected
        assert(codriver::CoordTransform::detectDrift(0.0, 20.0));
        // 5° difference → no drift
        assert(!codriver::CoordTransform::detectDrift(0.0, 5.0));
        // 350° vs 10° → 20° difference (wrap-around)
        assert(codriver::CoordTransform::detectDrift(350.0, 10.0));
        printf("PASS: CoordTransform detectDrift: 20°=drift, 5°=ok, 350°vs10°=drift\n");
    }

    // Test 6: BrakeDetector — create/destroy
    {
        codriver::BrakeDetector bd;
        assert(bd.getEventCount() == 0);
        printf("PASS: BrakeDetector create/destroy, events=%d\n", bd.getEventCount());
    }

    // Test 7: BrakeDetector — single cruising point
    {
        codriver::BrakeDetector bd;
        codriver::FusedPoint fp{};
        fp.timestamp_ms = 1000;
        fp.track_distance_m = 100.0;
        fp.speed_kmh = 120.0;
        fp.long_g = 0.05;
        bd.processPoint(fp);
        assert(bd.getEventCount() == 0);

        // Start braking
        fp.timestamp_ms = 2000;
        fp.track_distance_m = 130.0;
        fp.speed_kmh = 115.0;
        fp.long_g = -0.4;
        bd.processPoint(fp);
        assert(bd.getEventCount() == 0);

        // Peak braking
        fp.timestamp_ms = 3000;
        fp.track_distance_m = 160.0;
        fp.speed_kmh = 95.0;
        fp.long_g = -0.85;
        bd.processPoint(fp);
        assert(bd.getEventCount() == 0);
        printf("PASS: BrakeDetector peak braking, events=%d\n", bd.getEventCount());

        // Releasing → Finalizing (transition BRAKING→RELEASING→CRUISING)
        fp.timestamp_ms = 4000;
        fp.track_distance_m = 190.0;
        fp.speed_kmh = 75.0;
        fp.long_g = -0.05;
        bd.processPoint(fp);  // BRAKING → RELEASING (lg > -0.10)

        fp.timestamp_ms = 5000;
        fp.track_distance_m = 220.0;
        fp.speed_kmh = 70.0;
        fp.long_g = 0.02;
        bd.processPoint(fp);  // RELEASING → CRUISING (lg >= -0.05), event finalized!
        assert(bd.getEventCount() == 1);

        auto e = bd.getEvent(0);
        assert(e != nullptr);
        assert(e->brake_speed_kmh > 110.0);
        assert(e->release_speed_kmh > 65.0);
        assert(e->peak_decel_g < -0.8);
        assert(e->speed_drop_kmh > 40.0);

        printf("PASS: BrakeDetector finalized: %d events, brake at %.0fm, speed drop %.0f km/h, peak %.2fg\n",
               bd.getEventCount(), e->brake_distance_m, e->speed_drop_kmh, e->peak_decel_g);

        bd.reset();
        assert(bd.getEventCount() == 0);
        printf("PASS: BrakeDetector reset: %d events\n", bd.getEventCount());
    }

    printf("\nAll tests passed.\n");
    return 0;
}
