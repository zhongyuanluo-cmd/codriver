from fastapi import APIRouter

router = APIRouter(prefix="/api/coach", tags=["coach"])

@router.post("/chat")
async def coach_chat():
    # TODO: LLM integration via instructor + httpx
    return {"message": "CoDriver AI Coach endpoint - not yet implemented"}

@router.post("/brief")
async def session_brief():
    # TODO: Session brief generation
    return {"message": "Session brief generation - not yet implemented"}
