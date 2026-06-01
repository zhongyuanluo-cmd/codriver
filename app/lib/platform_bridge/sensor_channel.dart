import 'dart:async';
import 'package:flutter/services.dart';

/// EventChannel bridge for FusedPoint stream from native (C++ engine)
class SensorChannel {
  static const _channel = EventChannel('com.codriver/sensors');

  /// Stream of FusedPoint maps from the C++ engine via EventChannel.
  /// Each event is a Map with keys: lat, lon, speed_kmh, heading, long_g, lat_g, etc.
  Stream<Map<String, dynamic>> get fusedPointStream {
    return _channel.receiveBroadcastStream().map((event) {
      try {
        return Map<String, dynamic>.from(event as Map);
      } catch (e) {
        throw PlatformException(
          code: 'INVALID_EVENT',
          message: 'Expected Map event from EventChannel, got ${event.runtimeType}',
          details: e,
        );
      }
    });
  }
}
