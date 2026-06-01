"""CoDriver Pydantic data models - aligned with data-structures.md"""

from datetime import datetime
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
    is_valid: bool = True
    is_personal_best: bool = False
    max_speed_kmh: float = 0.0
    max_lat_g: float = 0.0


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
