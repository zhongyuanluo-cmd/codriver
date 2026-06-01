#include "codriver/coord_transform.h"
#include <Eigen/Dense>
#include <cmath>

namespace codriver {

class CoordTransform::Impl {
public:
    Eigen::Quaterniond q_phone_to_car;
    bool calibrated = false;
};

CoordTransform::CoordTransform() : impl_(new Impl()) {
    impl_->q_phone_to_car.setIdentity();
}

CoordTransform::~CoordTransform() { delete impl_; }

void CoordTransform::calibrate(double phone_accel_x, double phone_accel_y,
                                double phone_accel_z, double /* phone_gyro_x */,
                                double /* phone_gyro_y */, double /* phone_gyro_z */) {
    // Assume phone is flat: gravity along -z in phone frame → align with car vertical
    // TODO: Full orientation calibration
    impl_->calibrated = true;
}

void CoordTransform::transform(double accel_x, double accel_y, double accel_z,
                                double* car_long_g, double* car_lat_g,
                                double* car_vert_g) {
    if (!impl_->calibrated) {
        *car_long_g = accel_x;
        *car_lat_g = accel_y;
        *car_vert_g = accel_z;
        return;
    }
    // TODO: Apply rotation matrix
    *car_long_g = accel_x;
    *car_lat_g = accel_y;
    *car_vert_g = accel_z;
}

bool CoordTransform::isCalibrated() const { return impl_->calibrated; }

bool CoordTransform::detectDrift(double gps_heading_deg, double imu_heading_deg) const {
    constexpr double kDriftThreshold = 15.0;
    double diff = std::abs(gps_heading_deg - imu_heading_deg);
    if (diff > 180.0) diff = 360.0 - diff;
    return diff > kDriftThreshold;
}

} // namespace codriver
