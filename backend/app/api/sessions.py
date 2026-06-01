from fastapi import APIRouter

router = APIRouter(prefix="/api/sessions", tags=["sessions"])

@router.post("/upload")
async def upload_session():
    # TODO: Session data async upload (BackgroundTasks)
    return {"message": "Session upload - not yet implemented"}

@router.get("/{session_id}")
async def get_session(session_id: str):
    # TODO: Get session data from Supabase
    return {"session_id": session_id, "status": "not_implemented"}
