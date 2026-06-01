#ifndef CODRIVER_TYPES_H
#define CODRIVER_TYPES_H

#include <cstdint>
#include <vector>

namespace codriver {

// --- 传感器融合定位点 ---
struct FusedPoint {
    int64_t timestamp_ms;
    double latitude;
    double longitude;
    double altitude_m;
    double speed_kmh;
    double heading_deg;
    double accuracy_m;
    double long_g;        // 纵向 G (车身坐标, 正值=加速)
    double lat_g;         // 侧向 G (车身坐标, 正值=右转)
    double vert_g;        // 垂直 G
    double track_distance_m;
    double confidence;    // 定位置信度 (0-1)
};

// --- 赛道分段 ---
struct TrackSegment {
    const char* segment_id;       // "T3"
    const char* segment_type;     // "corner" | "straight" | "complex"
    const char* turn_direction;   // "left" | "right" | "straight"

    double entry_distance_m;
    double apex_distance_m;
    double exit_distance_m;

    double radius_m;
    double angle_deg;

    // 参考数据 (新赛道可为 NaN)
    double reference_speed_kmh;
    double reference_brake_point_m;
    double reference_entry_speed_kmh;
    double reference_exit_speed_kmh;
    double reference_lateral_g;

    int difficulty;  // 1-5
};

// --- 弯道表现 ---
struct CornerMetrics {
    double entry_speed_kmh;
    double min_speed_kmh;
    double exit_speed_kmh;
    double peak_lat_g;
    double line_deviation_avg_m;
    double apex_proximity_m;

    double braking_score;      // 0-100
    double speed_score;
    double throttle_score;
    double line_score;
};

// --- 根因推断结果 ---
struct RootCauseResult {
    const char* segment_id;
    const char* root_cause;         // "entry_too_early"
    const char* root_cause_label;   // "入弯太早"
    const char* confidence;         // "high" | "medium" | "low"
    const char* suggestion;
    double time_loss_ms;
};

// --- 模板播报消息 ---
struct CoachMessage {
    const char* text;
    int tier;              // 1, 2, or 3
    int priority;          // 0=立即, 1=直道期间, 2=圈后
};

} // namespace codriver

#endif // CODRIVER_TYPES_H
