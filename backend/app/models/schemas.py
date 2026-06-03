"""CoDriver Pydantic data models - aligned with data-structures.md"""

from datetime import datetime
from typing import List
from pydantic import BaseModel, Field


class FusedPoint(BaseModel):
    timestamp_ms: int
    latitude: float
    longitude: float
    altitude_m: float
    speed_kmh: float
    heading_deg: float
    accuracy_m: float
    long_g: float
    lat_g: float
    vert_g: float
    track_distance_m: float
    confidence: float = Field(ge=0.0, le=1.0)


class TrackSegment(BaseModel):
    segment_id: str
    segment_type: str  # "corner" | "straight" | "complex"
    turn_direction: str  # "left" | "right" | "straight"
    entry_distance_m: float
    apex_distance_m: float
    exit_distance_m: float
    radius_m: float
    angle_deg: float
    entry_lat: float | None = None
    entry_lon: float | None = None
    apex_lat: float | None = None
    apex_lon: float | None = None
    exit_lat: float | None = None
    exit_lon: float | None = None
    reference_speed_kmh: float | None = None
    reference_brake_point_m: float | None = None
    reference_entry_speed_kmh: float | None = None
    reference_exit_speed_kmh: float | None = None
    reference_lateral_g: float | None = None
    difficulty: int = Field(ge=1, le=5)


class CornerMetrics(BaseModel):
    corner_id: str
    lap_id: str
    segment_id: str
    entry_speed_kmh: float
    min_speed_kmh: float
    exit_speed_kmh: float
    peak_lat_g: float
    braking_score: float = Field(ge=0.0, le=100.0)
    speed_score: float = Field(ge=0.0, le=100.0)
    throttle_score: float = Field(ge=0.0, le=100.0)
    line_score: float = Field(ge=0.0, le=100.0)


class LapRecord(BaseModel):
    lap_id: str
    session_id: str
    lap_number: int
    lap_time_ms: int
    lap_distance_m: float
    avg_speed_kmh: float
    is_valid: bool = True
    is_personal_best: bool = False
    timestamp_start: datetime | None = None
    timestamp_end: datetime | None = None


# --- Phase 2.7: Analysis schemas ---

class CornerAnalysis(BaseModel):
    """Per-corner analysis result from pipeline"""
    segment_id: str
    entry_speed_kmh: float
    min_speed_kmh: float
    exit_speed_kmh: float
    max_lat_g: float
    entry_delta_kmh: float
    min_delta_kmh: float
    exit_delta_kmh: float
    lat_g_delta: float
    root_cause: str = ""
    root_cause_label: str = ""
    confidence: str = "medium"
    time_loss_ms: float = 0.0
    coach_message: str = ""
    coach_priority: int = 0
    brake_distance_m: float = 0.0
    brake_peak_g: float = 0.0
    speed_drop_kmh: float = 0.0
    feedback_tier: int = 2


class LapAnalysis(BaseModel):
    """Full lap analysis result"""
    lap_number: int
    lap_time_ms: int
    best_lap_time_ms: int | None = None
    corners: List[CornerAnalysis] = []


class SessionSummary(BaseModel):
    """Session-level statistics"""
    session_id: str
    total_laps: int
    best_lap_number: int
    best_lap_time_ms: int
    total_time_ms: int
    optimal_lap_time_ms: int | None = None
    avg_speed_kmh: float = 0.0


class SpeedCurvePoint(BaseModel):
    """Single point on the speed vs distance curve"""
    distance: float
    current_speed: float
    reference_speed: float = 0.0


class AnalyzeResponse(BaseModel):
    """Complete analysis response for a session"""
    session_id: str
    summary: SessionSummary
    laps: List[LapAnalysis] = []
    speed_curve: List[SpeedCurvePoint] = []


class Session(BaseModel):
    session_id: str
    track_id: str
    user_id: str
    car_id: str | None = None
    start_timestamp_ms: int
    end_timestamp_ms: int | None = None
    total_laps: int = 0
    valid_laps: int = 0
    best_lap_time_ms: int | None = None
