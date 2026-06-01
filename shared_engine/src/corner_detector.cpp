#include "codriver/corner_detector.h"

namespace codriver {

class CornerDetector::Impl {
public:
    std::vector<TrackSegment> segments;
};

CornerDetector::CornerDetector() : impl_(new Impl()) {}
CornerDetector::~CornerDetector() { delete impl_; }

void CornerDetector::processPoint(double distance_m, double lat, double lon,
                                   double speed_kmh, double curvature) {
    // TODO: Menger Curvature corner detection
}

std::vector<TrackSegment> CornerDetector::getSegments() const { return impl_->segments; }
int CornerDetector::getSegmentCount() const { return static_cast<int>(impl_->segments.size()); }
void CornerDetector::reset() { impl_->segments.clear(); }

} // namespace codriver
