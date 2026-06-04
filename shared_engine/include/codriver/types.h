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
    // Note: FusedPoint does not include GPS raw metadata (satellites, fix_quality).
    // These are consumed internally by Kalman Filter and not exposed to downstream.
};

// --- 赛道分段 ---
// P1-6: segment_id 指向 CornerDetector::Impl::id_store 中的 std::string
// 生命周期约束：在下次 processPoint() 或 reset() 调用前有效
// segment_type/turn_direction 指向字符串字面量，生命周期永久
struct TrackSegment {
    const char* segment_id;       // "T3"
    const char* segment_type;     // "corner" | "straight" | "complex"
    const char* turn_direction;   // "left" | "right" | "straight"

    double entry_distance_m;
    double apex_distance_m;
    double exit_distance_m;

    double radius_m;
    double angle_deg;

    // 关键点坐标 (WGS84)
    double entry_lat, entry_lon;
    double apex_lat, apex_lon;
    double exit_lat, exit_lon;

    // 参考数据 — 用 quiet_NaN() 表示"无参考"(新赛道/自动检测赛道)
    // 初始化: TrackSegment seg{}; seg.reference_speed_kmh = std::numeric_limits<double>::quiet_NaN();
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
// P1-6: segment_id 当前初始化 ""（C API 不暴露该字段）
// root_cause/label/confidence/suggestion 指向硬编码字符串字面量，生命周期永久
struct RootCauseResult {
    const char* segment_id;
    const char* root_cause;         // "entry_too_early"
    const char* root_cause_label;   // "入弯太早"
    const char* confidence;         // "high" | "medium" | "low"
    const char* suggestion;
    double time_loss_ms;
};

// --- 模板播报消息 ---
// tier: 主排序键，决定播报时机 (1=弯中即时<200ms, 2=直道期间, 3=圈后)
// priority: 同 tier 内次排序键 (0=最高优, 越大越可推迟)
struct CoachMessage {
    std::string text;
    int tier;
    int priority;
};

} // namespace codriver

#endif // CODRIVER_TYPES_H
