#ifndef CODRIVER_CORNER_DETECTOR_H
#define CODRIVER_CORNER_DETECTOR_H

#include "types.h"
#include <vector>

namespace codriver {

class CornerDetector {
public:
    CornerDetector();
    ~CornerDetector();

    // Menger Curvature-based corner detection
    void processPoint(double distance_m, double lat, double lon,
                      double speed_kmh, double curvature);

    std::vector<TrackSegment> getSegments() const;
    int getSegmentCount() const;
    void reset();

private:
    class Impl;
    Impl* impl_;
};

} // namespace codriver

#endif
