# Backend tests

def test_health():
    """Health endpoint should return ok"""
    from app.main import app
    from httpx import ASGITransport, AsyncClient
    import pytest
    # TODO: Add proper test
    pass
