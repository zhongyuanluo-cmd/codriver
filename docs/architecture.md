# CoDriver Architecture

## System Overview

```
Phone App (Flutter)               Cloud Backend (FastAPI)
┌─────────────────────┐          ┌─────────────────────────┐
│  UI (Riverpod)       │          │  API Gateway             │
│  ├── HomeScreen      │  HTTPS   │  ├── /api/health         │
│  ├── TrackScreen     │◄────────►│  ├── /api/sessions       │
│  ├── AnalysisScreen  │          │  ├── /api/tracks         │
│  └── SettingsScreen  │          │  └── /api/coach          │
├─────────────────────┤          ├─────────────────────────┤
│  Platform Bridge     │          │  LLM Client (instructor) │
│  ├── sensor_channel  │          │  ├── Qwen (CN)           │
│  └── engine_ffi      │          │  └── Llama (Overseas)    │
├─────────────────────┤          ├─────────────────────────┤
│  C++ Shared Engine   │          │  Supabase                │
│  ├── Kalman Filter   │          │  ├── PostgreSQL          │
│  ├── Corner Detector │          │  ├── Auth                │
│  ├── Root Cause      │          │  └── pgvector (RAG)      │
│  ├── Coach Template  │          └─────────────────────────┘
│  └── Coord Transform │
├─────────────────────┤
│  Native Sensors      │          Car Head Unit (Linux)
│  ├── iOS: CoreMotion │          ┌─────────────────────────┐
│  └── Android: Service│          │  Qt 6 + QML + C++        │
└─────────────────────┘          │  Shared C++ Engine ──────┘
                                 └─────────────────────────┘
```

## Key Design Decisions

1. Analysis engine runs locally (C++) for offline reliability
2. LLM calls are Tier 3 only (post-session), cloud-based
3. Platform Plugin bridges native sensors to C++ engine via EventChannel/FFI
4. Shared C++ engine between phone and car head unit
