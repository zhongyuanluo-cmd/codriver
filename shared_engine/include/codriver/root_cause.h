#ifndef CODRIVER_ROOT_CAUSE_H
#define CODRIVER_ROOT_CAUSE_H

#include "types.h"
#include <vector>

namespace codriver {

class RootCauseEngine {
public:
    RootCauseEngine();
    ~RootCauseEngine();

    RootCauseResult analyze(double brake_point_delta_m,
                            double entry_speed_delta,
                            double min_speed_delta,
                            double exit_speed_delta,
                            double lat_g_ratio,
                            double trail_brake_duration_ms,
                            double line_deviation_m);

private:
    class Impl;
    Impl* impl_;
};

} // namespace codriver

#endif
