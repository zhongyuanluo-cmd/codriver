from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
from app.api import sessions, tracks, coach, analysis

app = FastAPI(
    title="CoDriver API",
    description="AI Track Driving Coach Backend",
    version="0.1.0",
)

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],  # DEV ONLY: restrict in production
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# Register routers
app.include_router(sessions.router)
app.include_router(tracks.router)
app.include_router(coach.router)
app.include_router(analysis.router)


@app.get("/api/health")
async def health():
    return {"status": "ok", "version": "0.1.0"}


@app.get("/api/health/readiness")
async def readiness():
    # TODO: Check Supabase connection
    return {"status": "ready", "supabase": "not_configured"}
