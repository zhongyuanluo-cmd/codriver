import 'dart:async';
import 'package:flutter/services.dart';

/// EventChannel bridge for FusedPoint stream from native (C++ engine)
class SensorChannel {
  static const _channel = EventChannel('com.codriver/sensors');

  /// Stream of FusedPoint objects from the C++ engine via EventChannel
  Stream<Map<String, dynamic>> get fusedPointStream {
    return _channel.receiveBroadcastStream().cast<Map<String, dynamic>>();
  }
}
