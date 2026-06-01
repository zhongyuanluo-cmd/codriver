import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

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
          seedColor: const Color(0xFFE53935), // Racing Red
          brightness: Brightness.dark,
        ),
        useMaterial3: true,
      ),
      home: const HomeScreen(),
    );
  }
}

class HomeScreen extends StatelessWidget {
  const HomeScreen({super.key});

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('CoDriver'),
        centerTitle: true,
      ),
      body: const Center(
        child: Text(
          'AI Track Driving Coach',
          style: TextStyle(fontSize: 24),
        ),
      ),
    );
  }
}
