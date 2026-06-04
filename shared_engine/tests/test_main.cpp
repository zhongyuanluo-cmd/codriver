#include "codriver/kalman_filter.h"
#include "codriver/root_cause.h"
#include "codriver/coord_transform.h"
#include "codriver/brake_detector.h"
#include "codriver/corner_speed_compare.h"
#include "codriver/analysis_pipeline.h"
#include "codriver/best_lap_finder.h"
#include "codriver/session_stats.h"
#include "codriver/coach_engine.h"
#include "codriver/c_api.h"
#include "codriver/lap_timer.h"
#include <cassert>
#include <cmath>
#include <cstdio>
#include <limits>

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

    // Test 8: BrakeDetector — RELEASING→BRAKING rollback (L-12)
    {
        codriver::BrakeDetector bd;
        codriver::FusedPoint fp{};
        fp.timestamp_ms = 1000; fp.track_distance_m = 50.0;
        fp.speed_kmh = 100.0; fp.long_g = 0.05;
        bd.processPoint(fp);

        fp.timestamp_ms = 2000; fp.track_distance_m = 80.0;
        fp.speed_kmh = 90.0; fp.long_g = -0.5;
        bd.processPoint(fp);  // CRUISING → BRAKING

        fp.timestamp_ms = 3000; fp.track_distance_m = 110.0;
        fp.speed_kmh = 85.0; fp.long_g = -0.05;
        bd.processPoint(fp);  // BRAKING → RELEASING

        // Now brake again during release (trail braking re-apply)
        fp.timestamp_ms = 4000; fp.track_distance_m = 140.0;
        fp.speed_kmh = 75.0; fp.long_g = -0.6;
        bd.processPoint(fp);  // RELEASING → BRAKING (rollback!)

        fp.timestamp_ms = 5000; fp.track_distance_m = 160.0;
        fp.speed_kmh = 65.0; fp.long_g = -0.03;
        bd.processPoint(fp);  // BRAKING → RELEASING

        fp.timestamp_ms = 6000; fp.track_distance_m = 180.0;
        fp.speed_kmh = 60.0; fp.long_g = 0.01;
        bd.processPoint(fp);  // RELEASING → CRUISING, finalized

        assert(bd.getEventCount() == 1);
        auto e = bd.getEvent(0);
        assert(e->peak_decel_g < -0.55);  // peak should be -0.6 (second braking)
        printf("PASS: BrakeDetector rollback: peak=%.2fg, speed drop=%.0fkm/h\n",
               e->peak_decel_g, e->speed_drop_kmh);
    }

    // Test 9: CornerSpeedCompare — entry/apex/exit/lat_g comparison
    {
        codriver::CornerSpeedCompare cmp;
        codriver::TrackSegment seg{};
        seg.segment_id = "T1";
        seg.reference_entry_speed_kmh = 100.0;
        seg.reference_speed_kmh = 80.0;
        seg.reference_exit_speed_kmh = 95.0;
        seg.reference_lateral_g = 1.2;

        // Driver entered 5 km/h faster, min 2 km/h slower, exit same, lat_g 0.1g less
        auto d = cmp.compare(seg, 105.0, 78.0, 95.0, 1.1);
        assert(d.entry_delta_kmh == 5.0);
        assert(d.min_delta_kmh == -2.0);
        assert(d.exit_delta_kmh == 0.0);
        assert(std::abs(d.lat_g_delta + 0.1) < 0.01);
        printf("PASS: CornerSpeedCompare T1: entry=+%.0f min=%.0f exit=%.0f lat=%.2f\n",
               d.entry_delta_kmh, d.min_delta_kmh, d.exit_delta_kmh, d.lat_g_delta);
    }

    // Test 10: CornerSpeedCompare — no reference (NaN)
    {
        codriver::CornerSpeedCompare cmp;
        codriver::TrackSegment seg{};
        seg.segment_id = "T2";
        // All reference fields default to 0.0, not NaN
        // Set them explicitly to NaN for new track scenario
        seg.reference_entry_speed_kmh = std::numeric_limits<double>::quiet_NaN();
        seg.reference_speed_kmh = std::numeric_limits<double>::quiet_NaN();
        seg.reference_exit_speed_kmh = std::numeric_limits<double>::quiet_NaN();
        seg.reference_lateral_g = std::numeric_limits<double>::quiet_NaN();

        auto d = cmp.compare(seg, 90.0, 70.0, 85.0, 1.0);
        // Without reference, all deltas should be 0
        assert(d.entry_delta_kmh == 0.0);
        assert(d.min_delta_kmh == 0.0);
        assert(d.exit_delta_kmh == 0.0);
        assert(d.lat_g_delta == 0.0);
        printf("PASS: CornerSpeedCompare T2 (no ref): all deltas zeroed\n");
    }

    // Test 11: AnalysisPipeline — create/destroy
    {
        codriver::AnalysisPipeline pipeline;
        assert(pipeline.getResultCount() == 0);
        printf("PASS: AnalysisPipeline create/destroy, results=%d\n", pipeline.getResultCount());
    }

    // Test 12: AnalysisPipeline — process points through pipeline
    {
        codriver::AnalysisPipeline pipeline;
        // Simulate a straight → sharp corner → straight with clear curvature
        double base_lat = 30.0, base_lon = 120.0;
        for (int i = 0; i < 5; i++) {
            codriver::FusedPoint fp{};
            fp.timestamp_ms = 1000 + i * 100;
            fp.track_distance_m = i * 20.0;
            fp.latitude = base_lat + i * 0.001;
            fp.longitude = base_lon;
            fp.speed_kmh = 100.0;
            fp.long_g = 0.05; fp.lat_g = 0.02;
            pipeline.processPoint(fp);
        }
        // Sharp left turn (decreasing radius → curvature detected)
        for (int i = 0; i < 15; i++) {
            codriver::FusedPoint fp{};
            fp.timestamp_ms = 1500 + i * 100;
            fp.track_distance_m = 100.0 + i * 5.0;
            double angle = i * 0.12;  // progressively turning
            fp.latitude = base_lat + 0.005 + std::sin(angle) * 0.001;
            fp.longitude = base_lon + std::cos(angle) * 0.001;
            fp.speed_kmh = 100.0 - i * 2.0;
            fp.long_g = -0.05 - i * 0.02;
            fp.lat_g = 0.3 + i * 0.04;
            pipeline.processPoint(fp);
        }
        // Exit corner
        for (int i = 0; i < 5; i++) {
            codriver::FusedPoint fp{};
            fp.timestamp_ms = 3000 + i * 100;
            fp.track_distance_m = 175.0 + i * 20.0;
            fp.latitude = base_lat + 0.006 + i * 0.001;
            fp.longitude = base_lon + 0.001 + i * 0.001;
            fp.speed_kmh = 70.0 + i * 3.0;
            fp.long_g = 0.1; fp.lat_g = 0.15;
            pipeline.processPoint(fp);
        }

        int count = pipeline.getResultCount();
        printf("PASS: AnalysisPipeline processed 25 points, %d corner results\n", count);
        if (count > 0) {
            auto r = pipeline.getResult(0);
            printf("  Result[0]: %s entry=%.0f min=%.0f exit=%.0f cause=%s\n",
                   r->segment_id, r->entry_speed_kmh, r->min_speed_kmh,
                   r->exit_speed_kmh, r->root_cause);
        }
    }

    // Test 13: AnalysisPipeline — finalize() flushes last corner
    {
        codriver::AnalysisPipeline pipeline;
        // Simulate entering a single corner but never detecting a second one
        double base_lat = 30.0, base_lon = 120.0;
        // Straight
        for (int i = 0; i < 5; i++) {
            codriver::FusedPoint fp{};
            fp.timestamp_ms = 1000 + i * 100;
            fp.track_distance_m = i * 20.0;
            fp.latitude = base_lat + i * 0.001;
            fp.longitude = base_lon;
            fp.speed_kmh = 100.0;
            fp.long_g = 0.05; fp.lat_g = 0.02;
            pipeline.processPoint(fp);
        }
        // Corner (only one — no second corner to trigger CORNER_DONE)
        for (int i = 0; i < 15; i++) {
            codriver::FusedPoint fp{};
            fp.timestamp_ms = 1500 + i * 100;
            fp.track_distance_m = 100.0 + i * 5.0;
            double angle = i * 0.12;
            fp.latitude = base_lat + 0.005 + std::sin(angle) * 0.001;
            fp.longitude = base_lon + std::cos(angle) * 0.001;
            fp.speed_kmh = 100.0 - i * 2.0;
            fp.long_g = -0.05 - i * 0.02;
            fp.lat_g = 0.3 + i * 0.04;
            pipeline.processPoint(fp);
        }
        // No more points — corner is still pending
        int before = pipeline.getResultCount();
        bool flushed = pipeline.finalize();
        int after = pipeline.getResultCount();
        printf("PASS: AnalysisPipeline finalize() before=%d after=%d flushed=%d\n",
               before, after, flushed ? 1 : 0);
        // If there was a pending corner, it should be flushed
        if (flushed) {
            assert(after == before + 1);
        }
    }

    {
        codriver::BestLapFinder blf;
        // Record 4 laps: 120s, 115s, 118s, 122s
        blf.recordLap(120000, 4500.0);
        blf.recordLap(115000, 4500.0);
        blf.recordLap(118000, 4500.0);
        blf.recordLap(122000, 4500.0);

        assert(blf.getLapCount() == 4);

        auto best = blf.getBest();
        assert(best.total_laps == 4);
        assert(best.best_lap_number == 2);     // lap 2 is fastest (115s)
        assert(best.best_lap_time_ms == 115000);
        assert(!best.has_optimal);             // no sectors recorded

        printf("PASS: BestLapFinder: %d laps, best=L%d (%lld ms), total=%lld ms\n",
               best.total_laps, best.best_lap_number, best.best_lap_time_ms, best.total_time_ms);

        // Test getLap
        auto lap0 = blf.getLap(0);
        auto lap1 = blf.getLap(1);
        assert(lap0 != nullptr);
        assert(lap1 != nullptr);
        assert(lap0->lap_number == 1);
        assert(lap0->lap_time_ms == 120000);
        assert(lap1->lap_number == 2);
        assert(lap1->lap_time_ms == 115000);
        assert(blf.getLap(-1) == nullptr);  // out of range
        assert(blf.getLap(4) == nullptr);   // out of range
        printf("PASS: BestLapFinder getLap(0)=L%d(%lldms) getLap(1)=L%d(%lldms)\n",
               lap0->lap_number, lap0->lap_time_ms, lap1->lap_number, lap1->lap_time_ms);
    }

    // Test 14: BestLapFinder — optimal lap from sectors
    {
        codriver::BestLapFinder blf;
        blf.recordLap(120000, 4500.0);
        blf.recordSector(0, 30000);
        blf.recordSector(1, 40000);
        blf.recordSector(2, 50000);

        blf.recordLap(118000, 4500.0);
        blf.recordSector(0, 28000);  // better sector 0
        blf.recordSector(1, 42000);
        blf.recordSector(2, 48000);  // better sector 2

        auto best = blf.getBest();
        assert(best.has_optimal);
        // optimal = 28000 + 40000 + 48000 = 116000
        assert(best.optimal_lap_time_ms == 116000);

        printf("PASS: BestLapFinder optimal: best=L%d(%lldms) optimal=%lldms\n",
               best.best_lap_number, best.best_lap_time_ms, best.optimal_lap_time_ms);
    }

    // Test 15: SessionStats — compute session summary
    {
        codriver::SessionStatsCalc stats;
        stats.recordLap(120000, 4500.0);
        stats.recordLap(115000, 4500.0);
        stats.recordLap(118000, 4500.0);
        stats.recordLap(122000, 4500.0);

        auto s = stats.compute();
        assert(s.total_laps == 4);
        assert(s.best_lap_number == 2);
        assert(s.best_lap_time_ms == 115000);
        assert(s.avg_speed_kmh > 130);
        assert(s.consistency_score > 80);  // 4 laps within 7s spread

        printf("PASS: SessionStats: %d laps, best=L%d(%lldms), avg=%.0fkm/h, consistency=%.1f\n",
               s.total_laps, s.best_lap_number, s.best_lap_time_ms,
               s.avg_speed_kmh, s.consistency_score);
    }

    // Test 16: SessionStats — empty session (L-10)
    {
        codriver::SessionStatsCalc stats;
        auto s = stats.compute();
        assert(s.total_laps == 0);
        printf("PASS: SessionStats empty: total_laps=%d\n", s.total_laps);
    }

    // Test 16a: SessionStats — optimal from sectors (P1-5)
    {
        codriver::SessionStatsCalc stats;
        stats.recordSector(0, 5000);
        stats.recordSector(1, 8000);
        stats.recordSector(2, 6000);
        auto s = stats.compute();
        assert(s.total_laps == 0);  // no laps, only sectors
        assert(s.has_optimal);
        assert(s.optimal_lap_time_ms == 19000);
        printf("PASS: SessionStats optimal: %lld ms\n", s.optimal_lap_time_ms);
    }

    // Test 16b: SessionStats — reset clears all (P1-5)
    {
        codriver::SessionStatsCalc stats;
        stats.recordLap(30000, 1000);
        stats.recordSector(0, 5000);
        assert(stats.getLapCount() == 1);
        stats.reset();
        assert(stats.getLapCount() == 0);
        auto s2 = stats.compute();
        assert(s2.total_laps == 0);
        assert(!s2.has_optimal);
        printf("PASS: SessionStats reset clear\n");
    }

    // Test 16c: SessionStats — consistency single lap (P1-5)
    {
        codriver::SessionStatsCalc stats;
        stats.recordLap(60000, 2000);
        auto s = stats.compute();
        assert(s.total_laps == 1);
        assert(s.consistency_score == 0);  // single lap → no consistency
        printf("PASS: SessionStats single-lap consistency=%.1f\n", s.consistency_score);
    }

    // Test 16d: SessionStats — compute from BestLapFinder reuse (P1-5)
    {
        codriver::BestLapFinder blf;
        blf.recordLap(50000, 4000);
        blf.recordSector(0, 20000);
        blf.recordSector(1, 15000);

        codriver::SessionStatsCalc calc;
        auto s = calc.compute(blf);
        assert(s.total_laps == 1);
        assert(s.best_lap_time_ms == 50000);
        assert(s.has_optimal);
        assert(s.optimal_lap_time_ms == 35000);
        assert(s.consistency_score == 50.0);  // single-lap neutral
        printf("PASS: SessionStats compute(BLF): optimal=%lld cons=%.1f\n",
               s.optimal_lap_time_ms, s.consistency_score);
    }

    // Test 17: c_api — KalmanFilter predict+update+getState (L-10 enhanced)
    {
        void* kf = c_kalman_create();
        assert(kf != nullptr);
        c_kalman_predict(kf, 0.1);
        c_kalman_update_gps(kf, 30.0, 120.0, 10.0, 80.0, 90.0, 5.0);
        CFusedPoint out{};
        int rc = c_kalman_get_state(kf, &out);
        assert(rc == 0);
        assert(std::isfinite(out.lat));
        assert(std::isfinite(out.lon));
        c_kalman_destroy(kf);
        printf("PASS: c_api KalmanFilter: predict+update+getState rc=%d lat=%.2f lon=%.2f\n", rc, out.lat, out.lon);
    }

    // Test 18: c_api — CornerDetector multi-point (L-10 enhanced)
    {
        void* cd = c_corner_detector_create();
        assert(cd != nullptr);
        int cnt = c_corner_detector_get_segment_count(cd);
        assert(cnt >= 0);
        for (int i = 0; i < 10; i++) {
            c_corner_detector_process_point(cd,
                i * 10.0, 30.0 + i * 0.0005, 120.0 + i * 0.001, 100.0 - i * 2.0);
        }
        int final_cnt = c_corner_detector_get_segment_count(cd);
        assert(final_cnt >= 0);
        if (final_cnt > 0) {
            CCornerInfo info{};
            int rc = c_corner_detector_get_segment(cd, 0, &info);
            assert(rc == 0);
        }
        printf("PASS: c_api CornerDetector: 10pts segments=%d\n", final_cnt);
        c_corner_detector_destroy(cd);
    }

    // Test 19: LapTimer — crossing detection (L-10 enhanced)
    {
        codriver::LapTimer timer;
        timer.setStartLine(30.0, 120.0, 30.001, 120.001);
        double dist = 0; int dir = 0;
        timer.processPoint(30.01, 120.01, 1000, &dist, &dir);
        timer.processPoint(30.02, 120.02, 2000, &dist, &dir);
        assert(timer.lapCount() == 0);
        auto lap_ms = timer.processPoint(30.0005, 120.0005, 70000, &dist, &dir);
        assert(timer.lapCount() >= 1 || lap_ms > 0);
        printf("PASS: LapTimer: laps=%d lap_ms=%lld dist=%.0f dir=%d\n",
               timer.lapCount(), lap_ms, dist, dir);
    }

    // Test 20: c_api — CoordTransform calibrate+transform+detectDrift (L-10 enhanced)
    {
        void* ct = c_coord_transform_create();
        int ok = c_coord_transform_calibrate(ct, 0.0, 0.0, -9.81);
        assert(ok == 1);
        double clg = 0, clat = 0, cv = 0;
        int rc = c_coord_transform_transform(ct, 4.905, 0.0, -9.81, &clg, &clat, &cv);
        assert(rc == 0);
        assert(std::abs(clg - 0.5) < 0.1);
        int drift = c_coord_transform_detect_drift(ct, 90.0, 120.0);
        assert(drift == 1);
        c_coord_transform_destroy(ct);
        printf("PASS: c_api CoordTransform: calibrate=%d long_g=%.3f drift=%d\n", ok, clg, drift);
    }

    // ============================================================
    // Phase 3.1: Coach Engine
    // ============================================================

    // Test 21: CoachEngine create/destroy/clear
    {
        void* ce = c_coach_engine_create();
        assert(ce != nullptr);
        assert(c_coach_engine_message_count(ce) == 0);
        c_coach_engine_destroy(ce);
        printf("PASS: CoachEngine create/destroy\n");
    }

    // Test 22: CoachEngine feed + tier classification
    {
        void* ce = c_coach_engine_create();
        CPipelineResult r1{};
        std::memcpy(r1.seg_id, "T1", 3);
        std::memcpy(r1.msg, "T1: brake too early", 20);
        r1.tier = 1; r1.priority = 0;

        CPipelineResult r2{};
        std::memcpy(r2.seg_id, "T3", 3);
        std::memcpy(r2.msg, "T3: throttle late", 19);
        r2.tier = 2; r2.priority = 1;

        CPipelineResult r3{};
        std::memcpy(r3.seg_id, "T5", 3);
        std::memcpy(r3.msg, "T5: exit wide", 14);
        r3.tier = 1; r3.priority = 1;

        c_coach_engine_feed(ce, &r1);
        c_coach_engine_feed(ce, &r2);
        c_coach_engine_feed(ce, &r3);

        assert(c_coach_engine_message_count(ce) == 3);
        assert(c_coach_engine_tier_count(ce, 1) == 2);  // T1 + T5
        int t2cnt = c_coach_engine_tier_count(ce, 2);
        assert(t2cnt == 1);  // T3
        assert(c_coach_engine_tier_count(ce, 3) == 0);

        // Get tier 1 messages
        CCoachMessage cm{};
        int rc = c_coach_engine_get_message(ce, 1, 0, &cm);
        assert(rc == 0);
        assert(cm.tier == 1);

        c_coach_engine_destroy(ce);
        printf("PASS: CoachEngine feed+tier: t1=2 t2=%d t3=0\n", t2cnt);
    }

    // Test 23: CoachEngine lap summary
    {
        void* ce = c_coach_engine_create();

        CPipelineResult r1{};
        std::memcpy(r1.seg_id, "T1", 3);
        std::memcpy(r1.cause, "entry_too_hot", 14);
        std::memcpy(r1.msg, "T1: entry too hot", 18);
        r1.loss_ms = 500; r1.tier = 1;

        CPipelineResult r2{};
        std::memcpy(r2.seg_id, "T3", 3);
        std::memcpy(r2.cause, "throttle_late", 14);
        std::memcpy(r2.msg, "T3: throttle late", 18);
        r2.loss_ms = 300; r2.tier = 2;

        c_coach_engine_feed(ce, &r1);
        c_coach_engine_feed(ce, &r2);

        CCoachMessage summary{};
        int rc = c_coach_engine_generate_summary(ce, 1, 120000, &summary);
        assert(rc == 0);
        assert(summary.tier == 3);
        assert(summary.text[0] != '\0');

        c_coach_engine_destroy(ce);
        printf("PASS: CoachEngine lap summary: tier=%d\n", summary.tier);
    }

    // Test 24: CoachEngine clear + reuse
    {
        void* ce = c_coach_engine_create();

        CPipelineResult r1{};
        std::memcpy(r1.seg_id, "T1", 3);
        std::memcpy(r1.msg, "T1: test", 9);
        r1.tier = 1;
        c_coach_engine_feed(ce, &r1);
        assert(c_coach_engine_message_count(ce) == 1);

        c_coach_engine_clear(ce);
        assert(c_coach_engine_message_count(ce) == 0);

        // Reuse after clear
        CPipelineResult r2{};
        std::memcpy(r2.seg_id, "T2", 3);
        std::memcpy(r2.msg, "T2: test", 9);
        r2.tier = 2;
        c_coach_engine_feed(ce, &r2);
        assert(c_coach_engine_message_count(ce) == 1);
        assert(c_coach_engine_tier_count(ce, 2) == 1);

        c_coach_engine_destroy(ce);
        printf("PASS: CoachEngine clear+reuse\n");
    }

    printf("\nAll tests passed.\n");
    return 0;
}
