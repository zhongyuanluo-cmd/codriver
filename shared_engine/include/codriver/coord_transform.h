#ifndef CODRIVER_COORD_TRANSFORM_H
#define CODRIVER_COORD_TRANSFORM_H

#include <Eigen/Dense>

namespace codriver {

class CoordTransform {
public:
    CoordTransform();
    ~CoordTransform();

    // Calibrate phone orientation using gravity vector (phone stationary).
    // Returns true if calibration succeeded (gravity within plausible range).
    // Phone should be mounted in its final position; top edge ≈ forward.
    bool calibrate(double phone_accel_x, double phone_accel_y, double phone_accel_z);

    // Transform phone-frame accel (m/s²) to car-frame G-force.
    // Returns 0 on success, -1 if not calibrated (outputs set to 0).
    int transform(double accel_x, double accel_y, double accel_z,
                  double* car_long_g, double* car_lat_g, double* car_vert_g);

    // Check if calibration is valid
    bool isCalibrated() const;

    // Detect orientation drift (GPS heading vs IMU heading mismatch).
    // Static: does not depend on calibration state.
    static bool detectDrift(double gps_heading_deg, double imu_heading_deg);

private:
    class Impl;
    Impl* impl_;
};

} // namespace codriver

#endif
