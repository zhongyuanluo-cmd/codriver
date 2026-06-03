import 'dart:ffi';
import 'dart:io';
import 'package:ffi/ffi.dart';

// ============================================================
// C-side Structs (match c_api.h definitions)
// ============================================================

final class CFusedPoint extends Struct {
  @Double() external double lat;
  @Double() external double lon;
  @Double() external double alt;
  @Double() external double speed;
  @Double() external double heading;
  @Double() external double long_g;
  @Double() external double lat_g;
  @Double() external double vert_g;
  @Double() external double confidence;
}

final class CCornerInfo extends Struct {
  @Array(8)  external Array<Uint8> segId;
  @Array(16) external Array<Uint8> type;
  @Array(8)  external Array<Uint8> dir;
  @Double() external double entryD;
  @Double() external double apexD;
  @Double() external double exitD;
  @Double() external double entryLa;
  @Double() external double entryLo;
  @Double() external double apexLa;
  @Double() external double apexLo;
  @Double() external double exitLa;
  @Double() external double exitLo;
  @Double() external double radius;
  @Double() external double angle;
  @Int32()  external int diff;
}

final class CRootCause extends Struct {
  @Array(32)  external Array<Uint8> cause;
  @Array(32)  external Array<Uint8> label;
  @Array(16)  external Array<Uint8> conf;
  @Array(128) external Array<Uint8> sugg;
  @Double()   external double loss;
}

final class CBrakeEvent extends Struct {
  @Double() external double brakeLat;
  @Double() external double brakeLon;
  @Double() external double brakeDist;
  @Double() external double brakeSpd;
  @Double() external double peakG;
  @Double() external double peakDist;
  @Double() external double relLat;
  @Double() external double relLon;
  @Double() external double relDist;
  @Double() external double relSpd;
  @Double() external double durMs;
  @Double() external double trailMs;
  @Double() external double releaseMs;
  @Double() external double speedDrop;
  @Int64()  external int brakeTs;
  @Int64()  external int releaseTs;
  @Array(32) external Array<Uint8> segId;
}

final class CCornerSpeedDelta extends Struct {
  @Double() external double entryKmh;
  @Double() external double refEntry;
  @Double() external double entryDelta;
  @Double() external double minKmh;
  @Double() external double refMin;
  @Double() external double minDelta;
  @Double() external double exitKmh;
  @Double() external double refExit;
  @Double() external double exitDelta;
  @Double() external double latG;
  @Double() external double refLatG;
  @Double() external double latDelta;
  @Array(32) external Array<Uint8> segId;
}

final class CPipelineResult extends Struct {
  @Array(32)  external Array<Uint8> segId;
  @Array(32)  external Array<Uint8> cause;
  @Array(32)  external Array<Uint8> label;
  @Array(16)  external Array<Uint8> conf;
  @Array(256) external Array<Uint8> msg;
  @Double() external double entrySpd;
  @Double() external double minSpd;
  @Double() external double exitSpd;
  @Double() external double latG;
  @Double() external double eDelta;
  @Double() external double mDelta;
  @Double() external double xDelta;
  @Double() external double lDelta;
  @Double() external double lossMs;
  @Double() external double brakeDist;
  @Double() external double brakePeak;
  @Double() external double brakeDrop;
  @Int32()  external int priority;
  @Int32()  external int tier;
}

final class CBestLapResult extends Struct {
  @Int32()  external int bestLap;
  @Int32()  external int totalLaps;
  @Int64()  external int bestTime;
  @Int64()  external int totalTime;
  @Int64()  external int optimalTime;
  @Int32()  external int hasOpt;
}

final class CLapRecord extends Struct {
  @Int32()  external int lapNumber;
  @Int64()  external int lapTimeMs;
  @Double() external double lapDistanceM;
  @Double() external double avgSpeedKmh;
}

final class CSessionStats extends Struct {
  external CBestLapResult best;
  @Double() external double avgSpeed;
  @Double() external double consistency;
}

// ============================================================
// EngineFFI — complete C API bindings (48/48)
// ============================================================

class EngineFFI {
  static DynamicLibrary? _lib;

  static DynamicLibrary get lib {
    if (_lib != null) return _lib!;
    if (Platform.isAndroid) {
      _lib = DynamicLibrary.open('libcodriver_engine.so');
    } else if (Platform.isIOS) {
      _lib = DynamicLibrary.process();
    } else if (Platform.isLinux) {
      _lib = DynamicLibrary.open('libcodriver_engine.so');
    } else if (Platform.isMacOS) {
      _lib = DynamicLibrary.open('libcodriver_engine.dylib');
    } else if (Platform.isWindows) {
      _lib = DynamicLibrary.open('codriver_engine.dll');
    } else {
      throw UnsupportedError('Platform not supported');
    }
    return _lib!;
  }

  // ============================================================
  // Kalman Filter (7/7 complete)
  // ============================================================
  static Pointer<Void> kalmanCreate() =>
      lib.lookupFunction<Pointer<Void> Function(), Pointer<Void> Function()>('c_kalman_create')();
  static void kalmanDestroy(Pointer<Void> h) =>
      lib.lookupFunction<Void Function(Pointer<Void>), void Function(Pointer<Void>)>('c_kalman_destroy')(h);
  static void kalmanPredict(Pointer<Void> h, double dt) =>
      lib.lookupFunction<Void Function(Pointer<Void>, Double), void Function(Pointer<Void>, double)>('c_kalman_predict')(h, dt);
  static void kalmanUpdateGPS(Pointer<Void> h, double lat, double lon, double alt,
                                double spd, double hdg, double acc) =>
      lib.lookupFunction<Void Function(Pointer<Void>, Double, Double, Double, Double, Double, Double),
          void Function(Pointer<Void>, double, double, double, double, double, double)>('c_kalman_update_gps')(h, lat, lon, alt, spd, hdg, acc);
  static void kalmanUpdateIMU(Pointer<Void> h, double ax, double ay, double az,
                                double gx, double gy, double gz) =>
      lib.lookupFunction<Void Function(Pointer<Void>, Double, Double, Double, Double, Double, Double),
          void Function(Pointer<Void>, double, double, double, double, double, double)>('c_kalman_update_imu')(h, ax, ay, az, gx, gy, gz);
  static int kalmanGetState(Pointer<Void> h, Pointer<CFusedPoint> out) =>
      lib.lookupFunction<Int32 Function(Pointer<Void>, Pointer<CFusedPoint>),
          int Function(Pointer<Void>, Pointer<CFusedPoint>)>('c_kalman_get_state')(h, out);

  // ============================================================
  // Corner Detector (5/5 complete)
  // ============================================================
  static Pointer<Void> cornerDetectorCreate() =>
      lib.lookupFunction<Pointer<Void> Function(), Pointer<Void> Function()>('c_corner_detector_create')();
  static void cornerDetectorDestroy(Pointer<Void> h) =>
      lib.lookupFunction<Void Function(Pointer<Void>), void Function(Pointer<Void>)>('c_corner_detector_destroy')(h);
  static int cornerDetectorProcessPoint(Pointer<Void> h, double dist, double lat, double lon, double speed) =>
      lib.lookupFunction<Int32 Function(Pointer<Void>, Double, Double, Double, Double),
          int Function(Pointer<Void>, double, double, double, double)>('c_corner_detector_process_point')(h, dist, lat, lon, speed);
  static int cornerDetectorGetSegmentCount(Pointer<Void> h) =>
      lib.lookupFunction<Int32 Function(Pointer<Void>), int Function(Pointer<Void>)>('c_corner_detector_get_segment_count')(h);
  static int cornerDetectorGetSegment(Pointer<Void> h, int idx, Pointer<CCornerInfo> out) =>
      lib.lookupFunction<Int32 Function(Pointer<Void>, Int32, Pointer<CCornerInfo>),
          int Function(Pointer<Void>, int, Pointer<CCornerInfo>)>('c_corner_detector_get_segment')(h, idx, out);

  // ============================================================
  // Root Cause Engine (3/3 complete)
  // ============================================================
  static Pointer<Void> rootCauseCreate() =>
      lib.lookupFunction<Pointer<Void> Function(), Pointer<Void> Function()>('c_root_cause_create')();
  static void rootCauseDestroy(Pointer<Void> h) =>
      lib.lookupFunction<Void Function(Pointer<Void>), void Function(Pointer<Void>)>('c_root_cause_destroy')(h);
  static int rootCauseAnalyze(Pointer<Void> h, double bd, double ed, double md,
      double xd, double lg, double trail, double line, Pointer<CRootCause> out) =>
      lib.lookupFunction<Int32 Function(Pointer<Void>, Double, Double, Double, Double, Double, Double, Double, Pointer<CRootCause>),
          int Function(Pointer<Void>, double, double, double, double, double, double, double, Pointer<CRootCause>)>('c_root_cause_analyze')(h, bd, ed, md, xd, lg, trail, line, out);

  // ============================================================
  // Coach Template (3/3 complete)
  // ============================================================
  static Pointer<Void> coachCreate() =>
      lib.lookupFunction<Pointer<Void> Function(), Pointer<Void> Function()>('c_coach_template_create')();
  static void coachDestroy(Pointer<Void> h) =>
      lib.lookupFunction<Void Function(Pointer<Void>), void Function(Pointer<Void>)>('c_coach_template_destroy')(h);
  static int coachTemplateGenerate(Pointer<Void> h, Pointer<Utf8> seg,
      Pointer<Utf8> cause, double delta, int tier, Pointer<Utf8> buf, int maxLen) =>
      lib.lookupFunction<Int32 Function(Pointer<Void>, Pointer<Utf8>, Pointer<Utf8>, Double, Int32, Pointer<Utf8>, Int32),
          int Function(Pointer<Void>, Pointer<Utf8>, Pointer<Utf8>, double, int, Pointer<Utf8>, int)>('c_coach_template_generate')(h, seg, cause, delta, tier, buf, maxLen);

  // ============================================================
  // Lap Timer (6/6 complete)
  // ============================================================
  static Pointer<Void> lapTimerCreate() =>
      lib.lookupFunction<Pointer<Void> Function(), Pointer<Void> Function()>('c_lap_timer_create')();
  static void lapTimerDestroy(Pointer<Void> h) =>
      lib.lookupFunction<Void Function(Pointer<Void>), void Function(Pointer<Void>)>('c_lap_timer_destroy')(h);
  static void lapTimerSetLine(Pointer<Void> h, double la1, double lo1, double la2, double lo2) =>
      lib.lookupFunction<Void Function(Pointer<Void>, Double, Double, Double, Double),
          void Function(Pointer<Void>, double, double, double, double)>('c_lap_timer_set_line')(h, la1, lo1, la2, lo2);
  static int lapTimerProcess(Pointer<Void> h, double lat, double lon, int ts,
      Pointer<Double> dist, Pointer<Int32> dir) =>
      lib.lookupFunction<Int64 Function(Pointer<Void>, Double, Double, Int64, Pointer<Double>, Pointer<Int32>),
          int Function(Pointer<Void>, double, double, int, Pointer<Double>, Pointer<Int32>)>('c_lap_timer_process')(h, lat, lon, ts, dist, dir);
  static int lapTimerCount(Pointer<Void> h) =>
      lib.lookupFunction<Int32 Function(Pointer<Void>), int Function(Pointer<Void>)>('c_lap_timer_count')(h);
  static double lapTimerTotalDist(Pointer<Void> h) =>
      lib.lookupFunction<Double Function(Pointer<Void>), double Function(Pointer<Void>)>('c_lap_timer_total_dist')(h);

  // ============================================================
  // Coord Transform — Phase 2 (6/6)
  // ============================================================
  static Pointer<Void> coordTransformCreate() =>
      lib.lookupFunction<Pointer<Void> Function(), Pointer<Void> Function()>('c_coord_transform_create')();
  static void coordTransformDestroy(Pointer<Void> h) =>
      lib.lookupFunction<Void Function(Pointer<Void>), void Function(Pointer<Void>)>('c_coord_transform_destroy')(h);
  static int coordTransformCalibrate(Pointer<Void> h, double ax, double ay, double az) =>
      lib.lookupFunction<Int32 Function(Pointer<Void>, Double, Double, Double),
          int Function(Pointer<Void>, double, double, double)>('c_coord_transform_calibrate')(h, ax, ay, az);
  static int coordTransformTransform(Pointer<Void> h, double ax, double ay, double az,
                                       Pointer<Double> clg, Pointer<Double> clat, Pointer<Double> cv) =>
      lib.lookupFunction<Int32 Function(Pointer<Void>, Double, Double, Double, Pointer<Double>, Pointer<Double>, Pointer<Double>),
          int Function(Pointer<Void>, double, double, double, Pointer<Double>, Pointer<Double>, Pointer<Double>)>('c_coord_transform_transform')(h, ax, ay, az, clg, clat, cv);
  static int coordTransformIsCalibrated(Pointer<Void> h) =>
      lib.lookupFunction<Int32 Function(Pointer<Void>), int Function(Pointer<Void>)>('c_coord_transform_is_calibrated')(h);
  static int coordTransformDetectDrift(Pointer<Void> h, double gpsHdg, double imuHdg) =>
      lib.lookupFunction<Int32 Function(Pointer<Void>, Double, Double),
          int Function(Pointer<Void>, double, double)>('c_coord_transform_detect_drift')(h, gpsHdg, imuHdg);

  // ============================================================
  // Brake Detector — Phase 2.2 (5/5)
  // ============================================================
  static Pointer<Void> brakeDetectorCreate() =>
      lib.lookupFunction<Pointer<Void> Function(), Pointer<Void> Function()>('c_brake_detector_create')();
  static void brakeDetectorDestroy(Pointer<Void> h) =>
      lib.lookupFunction<Void Function(Pointer<Void>), void Function(Pointer<Void>)>('c_brake_detector_destroy')(h);
  static int brakeDetectorProcessPoint(Pointer<Void> h, double lat, double lon,
      double dist, double speed, double longG, int ts) =>
      lib.lookupFunction<Int32 Function(Pointer<Void>, Double, Double, Double, Double, Double, Int64),
          int Function(Pointer<Void>, double, double, double, double, double, int)>('c_brake_detector_process_point')(h, lat, lon, dist, speed, longG, ts);
  static int brakeDetectorGetEventCount(Pointer<Void> h) =>
      lib.lookupFunction<Int32 Function(Pointer<Void>), int Function(Pointer<Void>)>('c_brake_detector_get_event_count')(h);
  static int brakeDetectorGetEvent(Pointer<Void> h, int idx, Pointer<CBrakeEvent> out) =>
      lib.lookupFunction<Int32 Function(Pointer<Void>, Int32, Pointer<CBrakeEvent>),
          int Function(Pointer<Void>, int, Pointer<CBrakeEvent>)>('c_brake_detector_get_event')(h, idx, out);
  static void brakeDetectorReset(Pointer<Void> h) =>
      lib.lookupFunction<Void Function(Pointer<Void>), void Function(Pointer<Void>)>('c_brake_detector_reset')(h);

  // ============================================================
  // Corner Speed Compare — Phase 2.3 (3/3)
  // ============================================================
  static Pointer<Void> cornerSpeedCreate() =>
      lib.lookupFunction<Pointer<Void> Function(), Pointer<Void> Function()>('c_corner_speed_create')();
  static void cornerSpeedDestroy(Pointer<Void> h) =>
      lib.lookupFunction<Void Function(Pointer<Void>), void Function(Pointer<Void>)>('c_corner_speed_destroy')(h);
  static int cornerSpeedCompare(Pointer<Void> h,
      Pointer<Utf8> segId, double refEntry, double refMin, double refExit, double refLat,
      double actEntry, double actMin, double actExit, double actLat,
      Pointer<CCornerSpeedDelta> out) =>
      lib.lookupFunction<Int32 Function(Pointer<Void>, Pointer<Utf8>, Double, Double, Double, Double,
          Double, Double, Double, Double, Pointer<CCornerSpeedDelta>),
          int Function(Pointer<Void>, Pointer<Utf8>, double, double, double, double,
              double, double, double, double, Pointer<CCornerSpeedDelta>)>('c_corner_speed_compare')(
          h, segId, refEntry, refMin, refExit, refLat, actEntry, actMin, actExit, actLat, out);

  // ============================================================
  // Analysis Pipeline — Phase 2.4 (5/5)
  // ============================================================
  static Pointer<Void> pipelineCreate() =>
      lib.lookupFunction<Pointer<Void> Function(), Pointer<Void> Function()>('c_pipeline_create')();
  static void pipelineDestroy(Pointer<Void> h) =>
      lib.lookupFunction<Void Function(Pointer<Void>), void Function(Pointer<Void>)>('c_pipeline_destroy')(h);
  static int pipelineProcessPoint(Pointer<Void> h, double lat, double lon, double dist,
      double speed, double longG, double latG) =>
      lib.lookupFunction<Int32 Function(Pointer<Void>, Double, Double, Double, Double, Double, Double),
          int Function(Pointer<Void>, double, double, double, double, double, double)>('c_pipeline_process_point')(h, lat, lon, dist, speed, longG, latG);
  static int pipelineGetResultCount(Pointer<Void> h) =>
      lib.lookupFunction<Int32 Function(Pointer<Void>), int Function(Pointer<Void>)>('c_pipeline_get_result_count')(h);
  static int pipelineGetResult(Pointer<Void> h, int idx, Pointer<CPipelineResult> out) =>
      lib.lookupFunction<Int32 Function(Pointer<Void>, Int32, Pointer<CPipelineResult>),
          int Function(Pointer<Void>, int, Pointer<CPipelineResult>)>('c_pipeline_get_result')(h, idx, out);
  static void pipelineReset(Pointer<Void> h) =>
      lib.lookupFunction<Void Function(Pointer<Void>), void Function(Pointer<Void>)>('c_pipeline_reset')(h);
  static int pipelineFinalize(Pointer<Void> h) =>
      lib.lookupFunction<Int32 Function(Pointer<Void>), int Function(Pointer<Void>)>('c_pipeline_finalize')(h);

  // ============================================================
  // Best Lap Finder — Phase 2.5 (6/6)
  // ============================================================
  static Pointer<Void> bestLapCreate() =>
      lib.lookupFunction<Pointer<Void> Function(), Pointer<Void> Function()>('c_best_lap_create')();
  static void bestLapDestroy(Pointer<Void> h) =>
      lib.lookupFunction<Void Function(Pointer<Void>), void Function(Pointer<Void>)>('c_best_lap_destroy')(h);
  static int bestLapRecord(Pointer<Void> h, int lapTimeMs, double lapDistM) =>
      lib.lookupFunction<Int32 Function(Pointer<Void>, Int64, Double),
          int Function(Pointer<Void>, int, double)>('c_best_lap_record')(h, lapTimeMs, lapDistM);
  static int bestLapGetBest(Pointer<Void> h, Pointer<CBestLapResult> out) =>
      lib.lookupFunction<Int32 Function(Pointer<Void>, Pointer<CBestLapResult>),
          int Function(Pointer<Void>, Pointer<CBestLapResult>)>('c_best_lap_get_best')(h, out);
  static int bestLapCount(Pointer<Void> h) =>
      lib.lookupFunction<Int32 Function(Pointer<Void>), int Function(Pointer<Void>)>('c_best_lap_count')(h);
  static int bestLapGetLap(Pointer<Void> h, int idx, Pointer<CLapRecord> out) =>
      lib.lookupFunction<Int32 Function(Pointer<Void>, Int32, Pointer<CLapRecord>),
          int Function(Pointer<Void>, int, Pointer<CLapRecord>)>('c_best_lap_get_lap')(h, idx, out);
  static int bestLapRecordSector(Pointer<Void> h, int sectorIdx, int sectorTimeMs) =>
      lib.lookupFunction<Int32 Function(Pointer<Void>, Int32, Int64),
          int Function(Pointer<Void>, int, int)>('c_best_lap_record_sector')(h, sectorIdx, sectorTimeMs);
  static void bestLapReset(Pointer<Void> h) =>
      lib.lookupFunction<Void Function(Pointer<Void>), void Function(Pointer<Void>)>('c_best_lap_reset')(h);

  // ============================================================
  // Session Stats — Phase 2.8 (5/5)
  // ============================================================
  static Pointer<Void> sessionStatsCreate() =>
      lib.lookupFunction<Pointer<Void> Function(), Pointer<Void> Function()>('c_session_stats_create')();
  static void sessionStatsDestroy(Pointer<Void> h) =>
      lib.lookupFunction<Void Function(Pointer<Void>), void Function(Pointer<Void>)>('c_session_stats_destroy')(h);
  static void sessionStatsRecordLap(Pointer<Void> h, int lapTimeMs, double lapDistM) =>
      lib.lookupFunction<Void Function(Pointer<Void>, Int64, Double),
          void Function(Pointer<Void>, int, double)>('c_session_stats_record_lap')(h, lapTimeMs, lapDistM);
  static void sessionStatsRecordSector(Pointer<Void> h, int sectorIdx, int sectorTimeMs) =>
      lib.lookupFunction<Void Function(Pointer<Void>, Int32, Int64),
          void Function(Pointer<Void>, int, int)>('c_session_stats_record_sector')(h, sectorIdx, sectorTimeMs);
  static int sessionStatsCompute(Pointer<Void> h, Pointer<CSessionStats> out) =>
      lib.lookupFunction<Int32 Function(Pointer<Void>, Pointer<CSessionStats>),
          int Function(Pointer<Void>, Pointer<CSessionStats>)>('c_session_stats_compute')(h, out);
  static int sessionStatsCount(Pointer<Void> h) =>
      lib.lookupFunction<Int32 Function(Pointer<Void>), int Function(Pointer<Void>)>('c_session_stats_count')(h);
  static void sessionStatsReset(Pointer<Void> h) =>
      lib.lookupFunction<Void Function(Pointer<Void>), void Function(Pointer<Void>)>('c_session_stats_reset')(h);
}
