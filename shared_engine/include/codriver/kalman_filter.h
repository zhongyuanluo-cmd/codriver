#ifndef CODRIVER_KALMAN_FILTER_H
#define CODRIVER_KALMAN_FILTER_H

#include "types.h"
#include <vector>

namespace codriver {

class KalmanFilter {
public:
    KalmanFilter();
    ~KalmanFilter();

    void predict(double dt);
    void updateGPS(double lat, double lon, double alt,
                   double speed, double heading, double accuracy);
    void updateIMU(double accel_x, double accel_y, double accel_z,
                   double gyro_x, double gyro_y, double gyro_z);

    FusedPoint getState() const;

private:
    class Impl;
    Impl* impl_;
};

} // namespace codriver

#endif
