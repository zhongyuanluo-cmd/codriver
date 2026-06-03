"""Session analysis API — Phase 2.7"""
from fastapi import APIRouter, HTTPException
from app.models.schemas import (
    AnalyzeResponse, SessionSummary, LapAnalysis, CornerAnalysis, SpeedCurvePoint
)

router = APIRouter(prefix="/api/sessions", tags=["analysis"])


def _mock_corner_analysis(seg_id: str, base_speed: float) -> CornerAnalysis:
    """Generate mock corner analysis — placeholder until C++ engine integration."""
    return CornerAnalysis(
        segment_id=seg_id,
        entry_speed_kmh=base_speed,
        min_speed_kmh=base_speed - 20,
        exit_speed_kmh=base_speed - 5,
        max_lat_g=1.2,
        entry_delta_kmh=base_speed - 105,
        min_delta_kmh=-18,
        exit_delta_kmh=-3,
        lat_g_delta=-0.1,
        root_cause="entry_too_early",
        root_cause_label="Braking too early",
        confidence="medium",
        time_loss_ms=250.0,
        coach_message="Try braking 8m later for T1. Your entry speed is good.",
        coach_priority=2,
    )


def _mock_lap_analysis(lap_num: int, lap_time_ms: int) -> LapAnalysis:
    """Generate mock lap analysis."""
    corners = [
        _mock_corner_analysis("T1", 105.0),
        CornerAnalysis(
            segment_id="T2",
            entry_speed_kmh=95.0,
            min_speed_kmh=72.0,
            exit_speed_kmh=88.0,
            max_lat_g=1.1,
            entry_delta_kmh=-3.0,
            min_delta_kmh=-8.0,
            exit_delta_kmh=-2.0,
            lat_g_delta=-0.05,
            root_cause="entry_too_hot",
            root_cause_label="Entry too fast",
            confidence="high",
            time_loss_ms=400.0,
            coach_message="Entry speed too high for T2. Brake earlier to hit the apex.",
            coach_priority=1,
        ),
        CornerAnalysis(
            segment_id="T3",
            entry_speed_kmh=82.0,
            min_speed_kmh=68.0,
            exit_speed_kmh=78.0,
            max_lat_g=0.9,
            entry_delta_kmh=-4.0,
            min_delta_kmh=-2.0,
            exit_delta_kmh=0.0,
            lat_g_delta=0.0,
            root_cause="throttle_late",
            root_cause_label="Late on throttle",
            confidence="low",
            time_loss_ms=150.0,
            coach_message="Get on throttle earlier at T3 exit to carry more speed.",
            coach_priority=3,
        ),
    ]
    return LapAnalysis(
        lap_number=lap_num,
        lap_time_ms=lap_time_ms,
        best_lap_time_ms=108000,
        corners=corners,
    )


def _mock_speed_curve() -> list[SpeedCurvePoint]:
    """Generate mock speed vs distance curve data."""
    curve = []
    for i in range(40):
        d = i * 100.0
        if d < 400: curr = 60 + d * 0.1
        elif d < 800: curr = 100 - (d - 400) * 0.05
        elif d < 1200: curr = 80 - (d - 800) * 0.025
        elif d < 1400: curr = 70 + (d - 1200) * 0.15
        elif d < 1800: curr = 100
        elif d < 2200: curr = 100 - (d - 1800) * 0.06
        elif d < 2500: curr = 76 - (d - 2200) * 0.02
        elif d < 2800: curr = 70 + (d - 2500) * 0.12
        elif d < 3200: curr = 106 - (d - 2800) * 0.04
        elif d < 3400: curr = 90 - (d - 3200) * 0.03
        else: curr = 84 + (d - 3400) * 0.01
        curr = max(40, min(140, curr))
        ref = min(150, curr + 5)
        curve.append(SpeedCurvePoint(distance=d, current_speed=round(curr, 1), reference_speed=round(ref, 1)))
    return curve


@router.get("/{session_id}/analyze", response_model=AnalyzeResponse)
async def analyze_session(session_id: str):
    """
    Analyze a completed session.

    Returns session summary, per-lap analysis with corner breakdowns,
    and speed curve data for visualization.

    Phase 2.7: Currently returns mock data. Will integrate C++ AnalysisPipeline
    via FFI in Phase 3.
    """
    if not session_id:
        raise HTTPException(status_code=400, detail="session_id required")

    # TODO: Phase 3 — load actual FusedPoint data from Supabase/DB
    # TODO: Phase 3 — call C++ AnalysisPipeline via FFI/c_types

    laps = [
        _mock_lap_analysis(1, 112300),
        _mock_lap_analysis(2, 110500),
        _mock_lap_analysis(3, 108100),
    ]

    return AnalyzeResponse(
        session_id=session_id,
        summary=SessionSummary(
            session_id=session_id,
            total_laps=3,
            best_lap_number=3,
            best_lap_time_ms=108100,
            total_time_ms=330900,
            optimal_lap_time_ms=106500,
            avg_speed_kmh=118.5,
        ),
        laps=laps,
        speed_curve=_mock_speed_curve(),
    )
