import 'dart:ffi';
import 'dart:io';
import 'package:ffi/ffi.dart';

/// FFI bindings for CoDriver C++ engine (c_api.h)
class EngineFFI {
  static DynamicLibrary? _lib;

  static DynamicLibrary get lib {
    if (_lib != null) return _lib!;
    if (Platform.isAndroid) {
      _lib = DynamicLibrary.open('libcodriver_engine.so');
    } else if (Platform.isIOS) {
      _lib = DynamicLibrary.process(); // statically linked on iOS
    } else {
      throw UnsupportedError('Platform not supported');
    }
    return _lib!;
  }

  // Kalman Filter
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

  // Corner Detector
  static Pointer<Void> cornerDetectorCreate() =>
      lib.lookupFunction<Pointer<Void> Function(), Pointer<Void> Function()>('c_corner_detector_create')();
  static void cornerDetectorDestroy(Pointer<Void> h) =>
      lib.lookupFunction<Void Function(Pointer<Void>), void Function(Pointer<Void>)>('c_corner_detector_destroy')(h);
  static int cornerDetectorProcessPoint(Pointer<Void> h, double dist, double lat, double lon, double speed) =>
      lib.lookupFunction<Int32 Function(Pointer<Void>, Double, Double, Double, Double),
          int Function(Pointer<Void>, double, double, double, double)>('c_corner_detector_process_point')(h, dist, lat, lon, speed);

  // Root Cause Engine
  static Pointer<Void> rootCauseCreate() =>
      lib.lookupFunction<Pointer<Void> Function(), Pointer<Void> Function()>('c_root_cause_create')();
  static void rootCauseDestroy(Pointer<Void> h) =>
      lib.lookupFunction<Void Function(Pointer<Void>), void Function(Pointer<Void>)>('c_root_cause_destroy')(h);

  // Coach Template
  static Pointer<Void> coachCreate() =>
      lib.lookupFunction<Pointer<Void> Function(), Pointer<Void> Function()>('c_coach_template_create')();
  static void coachDestroy(Pointer<Void> h) =>
      lib.lookupFunction<Void Function(Pointer<Void>), void Function(Pointer<Void>)>('c_coach_template_destroy')(h);
}
