#include "codriver/corner_speed_compare.h"
#include <cmath>
#include <cstring>
#include <vector>

namespace codriver {

class CornerSpeedCompare::Impl {
public:
    // String storage for segment_id copies (prevents dangling pointers)
    std::vector<char> id_buffer;
    size_t id_offset = 0;

    const char* copyId(const char* src) {
        if (!src || !src[0]) return "";
        size_t len = std::strlen(src) + 1;
        if (id_offset + len > id_buffer.size()) {
            id_buffer.resize(id_offset + len + 256);
        }
        std::memcpy(&id_buffer[id_offset], src, len);
        const char* result = &id_buffer[id_offset];
        id_offset += len;
        return result;
    }
};

CornerSpeedCompare::CornerSpeedCompare() : impl_(new Impl()) {}
CornerSpeedCompare::~CornerSpeedCompare() { delete impl_; }

CornerSpeedDelta CornerSpeedCompare::compare(
    const TrackSegment& segment,
    double actual_entry_kmh,
    double actual_min_kmh,
    double actual_exit_kmh,
    double actual_lat_g)
{
    CornerSpeedDelta d{};

    d.segment_id = impl_->copyId(segment.segment_id);

    // Entry speed
    d.actual_entry_kmh = actual_entry_kmh;
    d.reference_entry_kmh = segment.reference_entry_speed_kmh;
    d.entry_delta_kmh = std::isnan(segment.reference_entry_speed_kmh)
        ? 0.0 : actual_entry_kmh - segment.reference_entry_speed_kmh;

    // Minimum speed
    d.actual_min_kmh = actual_min_kmh;
    d.reference_min_kmh = segment.reference_speed_kmh;
    d.min_delta_kmh = std::isnan(segment.reference_speed_kmh)
        ? 0.0 : actual_min_kmh - segment.reference_speed_kmh;

    // Exit speed
    d.actual_exit_kmh = actual_exit_kmh;
    d.reference_exit_kmh = segment.reference_exit_speed_kmh;
    d.exit_delta_kmh = std::isnan(segment.reference_exit_speed_kmh)
        ? 0.0 : actual_exit_kmh - segment.reference_exit_speed_kmh;

    // Lateral G
    d.actual_lat_g = actual_lat_g;
    d.reference_lat_g = segment.reference_lateral_g;
    d.lat_g_delta = std::isnan(segment.reference_lateral_g)
        ? 0.0 : actual_lat_g - segment.reference_lateral_g;

    return d;
}

int CornerSpeedCompare::compareAll(
    const TrackSegment* segments, int segment_count,
    const double* actual_entry, const double* actual_min,
    const double* actual_exit, const double* actual_lat_g,
    CornerSpeedDelta* results, int max_results)
{
    if (!segments || !actual_entry || !actual_min || !actual_exit ||
        !actual_lat_g || !results || max_results <= 0) {
        return 0;
    }

    int count = (segment_count < max_results) ? segment_count : max_results;
    for (int i = 0; i < count; i++) {
        results[i] = compare(segments[i],
            actual_entry[i], actual_min[i], actual_exit[i], actual_lat_g[i]);
    }
    return count;
}

} // namespace codriver
