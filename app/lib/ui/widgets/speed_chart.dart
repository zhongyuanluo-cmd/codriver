import 'package:flutter/material.dart';
import 'package:fl_chart/fl_chart.dart';

/// Speed vs distance chart for lap analysis.
/// Shows current lap speed (solid line) vs reference speed (dashed line).
/// Corner segments are highlighted with vertical bands.
class SpeedChart extends StatelessWidget {
  final List<SpeedPoint> currentLap;
  final List<SpeedPoint>? referenceLap;
  final List<CornerZone>? corners;
  final double maxSpeed;
  final String xLabel;
  final String yLabel;

  const SpeedChart({
    super.key,
    required this.currentLap,
    this.referenceLap,
    this.corners,
    this.maxSpeed = 160,
    this.xLabel = 'Distance (m)',
    this.yLabel = 'Speed (km/h)',
  });

  @override
  Widget build(BuildContext context) {
    return LineChart(
      LineChartData(
        minX: 0,
        maxX: currentLap.isNotEmpty ? currentLap.last.distance : 4000,
        minY: 0,
        maxY: maxSpeed,
        gridData: FlGridData(
          show: true,
          drawVerticalLine: false,
          horizontalInterval: maxSpeed / 8,
          getDrawingHorizontalLine: (value) => FlLine(
            color: Colors.white12,
            strokeWidth: 0.5,
          ),
        ),
        titlesData: FlTitlesData(
          bottomTitles: AxisTitles(
            sideTitles: SideTitles(
              showTitles: true,
              reservedSize: 30,
              interval: _calcXInterval(),
              getTitlesWidget: (value, meta) =>
                  Text('${value.toInt()}', style: const TextStyle(fontSize: 10, color: Colors.white54)),
            ),
          ),
          leftTitles: AxisTitles(
            sideTitles: SideTitles(
              showTitles: true,
              reservedSize: 40,
              interval: maxSpeed / 4,
              getTitlesWidget: (value, meta) =>
                  Text('${value.toInt()}', style: const TextStyle(fontSize: 10, color: Colors.white54)),
            ),
          ),
          topTitles: const AxisTitles(sideTitles: SideTitles(showTitles: false)),
          rightTitles: const AxisTitles(sideTitles: SideTitles(showTitles: false)),
        ),
        borderData: FlBorderData(show: false),
        lineBarsData: _buildLineBars(),
        lineTouchData: LineTouchData(
          touchTooltipData: LineTouchTooltipData(
            getTooltipItems: (touchedSpots) => touchedSpots.map((s) {
              return LineTooltipItem(
                '${s.y.toInt()} km/h\n${s.x.toInt()} m',
                const TextStyle(color: Colors.white, fontSize: 12),
              );
            }).toList(),
          ),
        ),
        extraLinesData: _corners != null && _corners!.isNotEmpty
            ? ExtraLinesData(verticalLines: _buildCornerLines())
            : null,
      ),
    );
  }

  List<CornerZone>? get _corners => corners;

  List<LineChartBarData> _buildLineBars() {
    final bars = <LineChartBarData>[];

    // Current lap (solid red)
    bars.add(LineChartBarData(
      spots: currentLap.map((p) => FlSpot(p.distance, p.speed)).toList(),
      color: const Color(0xFFE53935),
      barWidth: 2.5,
      isCurved: true,
      curveSmoothness: 0.2,
      dotData: const FlDotData(show: false),
      belowBarData: BarAreaData(
        show: true,
        color: const Color(0xFFE53935).withAlpha(30),
      ),
    ));

    // Reference lap (dashed blue)
    if (referenceLap != null && referenceLap!.isNotEmpty) {
      bars.add(LineChartBarData(
        spots: referenceLap!.map((p) => FlSpot(p.distance, p.speed)).toList(),
        color: Colors.lightBlueAccent,
        barWidth: 1.5,
        isCurved: true,
        curveSmoothness: 0.2,
        dashArray: [8, 4],
        dotData: const FlDotData(show: false),
        belowBarData: BarAreaData(show: false),
      ));
    }

    return bars;
  }

  List<VerticalLine> _buildCornerLines() {
    final lines = <VerticalLine>[];
    for (final c in _corners!) {
      lines.add(VerticalLine(
        x: c.startDistance,
        color: Colors.orange.withValues(alpha: 0.4),
        strokeWidth: 1.5,
        dashArray: [4, 4],
      ));
      lines.add(VerticalLine(
        x: c.endDistance,
        color: Colors.orange.withValues(alpha: 0.4),
        strokeWidth: 1.5,
        dashArray: [4, 4],
      ));
      // Label above the corner zone
      lines.add(VerticalLine(
        x: (c.startDistance + c.endDistance) / 2,
        color: Colors.transparent,
        strokeWidth: 0,
        label: VerticalLineLabel(
          show: true,
          alignment: Alignment.topCenter,
          labelResolver: (_) => c.label,
          style: const TextStyle(color: Colors.orange, fontSize: 11, fontWeight: FontWeight.bold),
        ),
      ));
    }
    return lines;
  }

  double _calcXInterval() {
    final max = currentLap.isNotEmpty ? currentLap.last.distance : 4000;
    if (max <= 1000) return 200;
    if (max <= 3000) return 500;
    return 1000;
  }
}

/// A single speed data point.
// TODO(Phase 3): Replace with API-aligned SpeedCurvePoint{distance, currentSpeed, referenceSpeed}
// to match backend/app/models/schemas.py SpeedCurvePoint
class SpeedPoint {
  final double distance;
  final double speed;
  const SpeedPoint({required this.distance, required this.speed});
}

/// A corner zone on the track (for annotating the chart).
// TODO(Phase 3): Replace with API-aligned CornerAnalysis model matching
// backend/app/models/schemas.py CornerAnalysis (15 fields)
class CornerZone {
  final double startDistance;
  final double endDistance;
  final String label;
  final String? rootCause;
  const CornerZone({
    required this.startDistance,
    required this.endDistance,
    required this.label,
    this.rootCause,
  });
}
