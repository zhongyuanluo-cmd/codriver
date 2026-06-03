import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'ui/screens/analysis_screen.dart';

void main() {
  runApp(
    const ProviderScope(
      child: CoDriverApp(),
    ),
  );
}

class CoDriverApp extends StatelessWidget {
  const CoDriverApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'CoDriver',
      theme: ThemeData(
        colorScheme: ColorScheme.fromSeed(
          seedColor: const Color(0xFFE53935),
          brightness: Brightness.dark,
        ),
        useMaterial3: true,
      ),
      home: const MainShell(),
    );
  }
}

class MainShell extends StatefulWidget {
  const MainShell({super.key});

  @override
  State<MainShell> createState() => _MainShellState();
}

class _MainShellState extends State<MainShell> {
  int _selectedIndex = 0;

  static final _pages = <Widget>[
    const HomeScreen(),
    const TrackScreen(),
    AnalysisScreen(),
    const SettingsScreen(),
  ];

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: _pages[_selectedIndex],
      bottomNavigationBar: NavigationBar(
        selectedIndex: _selectedIndex,
        onDestinationSelected: (i) => setState(() => _selectedIndex = i),
        destinations: const [
          NavigationDestination(icon: Icon(Icons.speed), label: 'Home'),
          NavigationDestination(icon: Icon(Icons.map), label: 'Track'),
          NavigationDestination(icon: Icon(Icons.analytics), label: 'Analysis'),
          NavigationDestination(icon: Icon(Icons.settings), label: 'Settings'),
        ],
      ),
    );
  }
}

class HomeScreen extends StatelessWidget {
  const HomeScreen({super.key});

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: const Text('CoDriver'), centerTitle: true),
      body: const Center(child: Text('AI Track Driving Coach', style: TextStyle(fontSize: 24))),
    );
  }
}

class TrackScreen extends StatelessWidget {
  const TrackScreen({super.key});
  @override
  Widget build(BuildContext context) {
    return Scaffold(appBar: AppBar(title: const Text('Track')), body: const Center(child: Text('Track Database')));
  }
}

class SettingsScreen extends StatelessWidget {
  const SettingsScreen({super.key});
  @override
  Widget build(BuildContext context) {
    return Scaffold(appBar: AppBar(title: const Text('Settings')), body: const Center(child: Text('Settings')));
  }
}
