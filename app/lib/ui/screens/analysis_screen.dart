import 'package:flutter/material.dart';
import '../widgets/speed_chart.dart';

class AnalysisScreen extends StatefulWidget {
  const AnalysisScreen({super.key});

  @override
  State<AnalysisScreen> createState() => _AnalysisScreenState();
}

class _AnalysisScreenState extends State<AnalysisScreen> {
  // TODO(Phase 3): Replace mock data with Backend API call to /api/analysis/sessions/{id}
  // Current mock logic duplicates backend/app/api/analysis.py _mock_speed_curve().
  // TODO(Phase 3): Replace SpeedPoint/CornerZone with API-aligned models.

  // Mock data — will be replaced with real pipeline data in Phase 2.7
  static final _mockCurrent = List.generate(40, (i) {
    final d = i * 100.0;
    final speed = _mockSpeed(d, 100, 80, 70, 100, 76, 70, 106, 90, 84);
    return SpeedPoint(distance: d, speed: speed.clamp(40, 140));
  });

  static final _mockReference = List.generate(40, (i) {
    final d = i * 100.0;
    final speed = _mockSpeed(d, 105, 85, 75, 105, 81, 75, 111, 95, 90);
    return SpeedPoint(distance: d, speed: speed.clamp(40, 150));
  });

  static double _mockSpeed(
    double d, double s1, double s2, double s3, double s4,
    double s5, double s6, double s7, double s8, double s9,
  ) {
    if (d < 400) { return 60 + d * 0.1; }
    if (d < 800) { return s1 - (d - 400) * 0.05; }
    if (d < 1200) { return s2 - (d - 800) * 0.025; }
    if (d < 1400) { return s3 + (d - 1200) * 0.15; }
    if (d < 1800) { return s4; }
    if (d < 2200) { return s5 - (d - 1800) * 0.06; }
    if (d < 2500) { return s6 - (d - 2200) * 0.02; }
    if (d < 2800) { return s7 + (d - 2500) * 0.12; }
    if (d < 3200) { return s8 - (d - 2800) * 0.04; }
    if (d < 3400) { return s9 - (d - 3200) * 0.03; }
    return 84 + (d - 3400) * 0.01;
  }

  static final _mockCorners = [
    const CornerZone(startDistance: 400, endDistance: 1400, label: 'T1', rootCause: 'entry_too_early'),
    const CornerZone(startDistance: 1800, endDistance: 2800, label: 'T2', rootCause: 'entry_too_hot'),
    const CornerZone(startDistance: 2800, endDistance: 3600, label: 'T3', rootCause: 'throttle_late'),
  ];

  @override
  Widget build(BuildContext context) {
    final theme = Theme.of(context);
    return Scaffold(
      appBar: AppBar(
        title: const Text('Lap Analysis'),
        centerTitle: true,
      ),
      body: SingleChildScrollView(
        padding: const EdgeInsets.all(16),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            // Summary cards
            Row(
              children: [
                _buildStatCard('Lap Time', '1:52.3', Icons.timer, theme),
                const SizedBox(width: 12),
                _buildStatCard('Best Lap', '1:48.1', Icons.emoji_events, theme),
                const SizedBox(width: 12),
                _buildStatCard('Top Speed', '138 km/h', Icons.speed, theme),
              ],
            ),
            const SizedBox(height: 16),

            // Chart section
            Card(
              child: Padding(
                padding: const EdgeInsets.all(16),
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    Row(
                      children: [
                        Text('Speed Profile', style: theme.textTheme.titleMedium),
                        const Spacer(),
                        _buildLegend('Current', const Color(0xFFE53935)),
                        const SizedBox(width: 12),
                        _buildLegend('Reference', Colors.lightBlueAccent),
                      ],
                    ),
                    const SizedBox(height: 8),
                    SizedBox(
                      height: 220,
                      child: SpeedChart(
                        currentLap: _mockCurrent,
                        referenceLap: _mockReference,
                        corners: _mockCorners,
                      ),
                    ),
                  ],
                ),
              ),
            ),
            const SizedBox(height: 16),

            // Corner analysis
            Text('Corner Analysis', style: theme.textTheme.titleMedium),
            const SizedBox(height: 8),
            ..._mockCorners.map((c) => Card(
                  child: ListTile(
                    leading: CircleAvatar(
                      backgroundColor: _rootCauseColor(c.rootCause),
                      child: Text(c.label, style: const TextStyle(color: Colors.white, fontWeight: FontWeight.bold)),
                    ),
                    title: Text('${c.label} — ${_rootCauseLabel(c.rootCause)}'),
                    subtitle: Text('${c.startDistance.toInt()}m — ${c.endDistance.toInt()}m'),
                    trailing: const Icon(Icons.chevron_right),
                  ),
                )),
          ],
        ),
      ),
    );
  }

  Widget _buildStatCard(String label, String value, IconData icon, ThemeData theme) {
    return Expanded(
      child: Card(
        child: Padding(
          padding: const EdgeInsets.all(12),
          child: Column(
            children: [
              Icon(icon, color: theme.colorScheme.primary, size: 24),
              const SizedBox(height: 4),
              Text(value, style: theme.textTheme.titleMedium?.copyWith(fontWeight: FontWeight.bold)),
              Text(label, style: theme.textTheme.bodySmall?.copyWith(color: Colors.white54)),
            ],
          ),
        ),
      ),
    );
  }

  Widget _buildLegend(String label, Color color) {
    return Row(
      mainAxisSize: MainAxisSize.min,
      children: [
        Container(width: 16, height: 3, color: color),
        const SizedBox(width: 4),
        Text(label, style: const TextStyle(fontSize: 12, color: Colors.white70)),
      ],
    );
  }

  Color _rootCauseColor(String? cause) {
    switch (cause) {
      case 'entry_too_early': return Colors.orange;
      case 'entry_too_hot': return Colors.red;
      case 'throttle_late': return Colors.amber;
      default: return Colors.grey;
    }
  }

  String _rootCauseLabel(String? cause) {
    switch (cause) {
      case 'entry_too_early': return 'Braking too early';
      case 'entry_too_hot': return 'Entry too fast';
      case 'throttle_late': return 'Late on throttle';
      default: return 'Analysis pending';
    }
  }
}
