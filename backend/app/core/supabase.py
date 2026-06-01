from supabase import create_client, Client
from app.core.config import settings

_supabase: Client | None = None


def get_supabase() -> Client:
    global _supabase
    if _supabase is None:
        if not settings.supabase_url:
            raise RuntimeError("SUPABASE_URL not configured")
        _supabase = create_client(settings.supabase_url, settings.supabase_anon_key)
    return _supabase
