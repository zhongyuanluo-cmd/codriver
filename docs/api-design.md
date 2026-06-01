# $schema: https://raw.githubusercontent.com/zhongyuanluo-cmd/codriver/main/docs/api-design.md

# CoDriver API Design (Draft)

## Base URL

- Development: `http://localhost:8000/api`
- Production: TBD

## Endpoints

### Health

| Method | Path | Description |
|--------|------|-------------|
| GET | `/api/health` | Health check |
| GET | `/api/health/readiness` | Readiness probe |

### Sessions

| Method | Path | Description |
|--------|------|-------------|
| POST | `/api/sessions/upload` | Upload session telemetry |
| GET | `/api/sessions/{id}` | Get session details |

### Tracks

| Method | Path | Description |
|--------|------|-------------|
| GET | `/api/tracks` | List available tracks |
| GET | `/api/tracks/{id}` | Get track details |

### Coach

| Method | Path | Description |
|--------|------|-------------|
| POST | `/api/coach/chat` | AI coach chat |
| POST | `/api/coach/brief` | Generate session brief |

## Authentication

Supabase Auth via JWT Bearer token.
