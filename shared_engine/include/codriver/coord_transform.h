#ifndef CODRIVER_COORD_TRANSFORM_H
#define CODRIVER_COORD_TRANSFORM_H

#include <Eigen/Dense>

namespace codriver {

class CoordTransform {
public:
    CoordTransform();
    ~CoordTransform();

    // Calibrate phone orientation (user places phone flat)
    void calibrate(double phone_accel_x, double phone_accel_y, double phone_accel_z,
                   double phone_gyro_x, double phone_gyro_y, double phone_gyro_z);

    // Transform phone-frame accel to car-frame
    void transform(double accel_x, double accel_y, double accel_z,
                   double* car_long_g, double* car_lat_g, double* car_vert_g);

    // Check if calibration is valid
    bool isCalibrated() const;

    // Detect orientation drift (GPS heading vs IMU heading mismatch)
    bool detectDrift(double gps_heading_deg, double imu_heading_deg) const;

private:
    class Impl;
    Impl* impl_;
};

} // namespace codriver

#endif
