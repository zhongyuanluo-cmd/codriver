from fastapi import APIRouter

router = APIRouter(prefix="/api/tracks", tags=["tracks"])

@router.get("/")
async def list_tracks():
    # TODO: Get track list from Supabase
    return {"tracks": [], "status": "not_implemented"}

@router.get("/{track_id}")
async def get_track(track_id: str):
    return {"track_id": track_id, "status": "not_implemented"}
