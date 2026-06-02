#define _USE_MATH_DEFINES
#include "codriver/coord_transform.h"
#include <Eigen/Dense>
#include <cmath>
#include <algorithm>

namespace codriver {

class CoordTransform::Impl {
public:
    Eigen::Quaterniond q_phone_to_car;
    bool calibrated = false;
    double gravity_mag = 9.81;  // cached gravity magnitude from calibration
};

CoordTransform::CoordTransform() : impl_(new Impl()) {
    impl_->q_phone_to_car.setIdentity();
}

CoordTransform::~CoordTransform() { delete impl_; }

bool CoordTransform::calibrate(double phone_accel_x, double phone_accel_y,
                                double phone_accel_z) {
    // Phase 2: Gravity-based orientation calibration.
    //
    // When the phone is stationary on a dashboard mount, the only acceleration
    // is gravity. We compute the rotation from the phone's gravity vector to
    // the car-frame gravity direction (0, 0, -1), i.e. "down" in car coordinates.
    //
    // Car coordinate frame:
    //   X = forward (longitudinal, positive = accelerating)
    //   Y = right   (lateral, positive = turning right)
    //   Z = up      (vertical, positive = upward)
    //
    // This calibration corrects pitch and roll. Yaw alignment assumes the
    // phone's top edge points roughly forward (standard dashboard mount).
    // GPS-based yaw calibration can be added in Phase 3+.

    Eigen::Vector3d g_phone(phone_accel_x, phone_accel_y, phone_accel_z);
    double mag = g_phone.norm();

    if (mag < 0.5 || mag > 15.0) {
        // Gravity reading out of plausible range; calibration failed
        return false;
    }

    impl_->gravity_mag = mag;
    g_phone.normalize();

    // Target: gravity in car frame points along -Z
    Eigen::Vector3d g_car(0.0, 0.0, -1.0);

    // Compute shortest rotation from g_phone to g_car
    double dot = g_phone.dot(g_car);

    if (dot > 0.9999) {
        // Already aligned — identity quaternion
        impl_->q_phone_to_car.setIdentity();
    } else if (dot < -0.9999) {
        // 180° apart — rotate about any perpendicular axis.
        // Pick (1,0,0) as arbitrary axis; cross with g_phone gives a valid perpendicular.
        Eigen::Vector3d axis = std::abs(g_phone.x()) < 0.9
            ? Eigen::Vector3d(1.0, 0.0, 0.0).cross(g_phone).normalized()
            : Eigen::Vector3d(0.0, 1.0, 0.0).cross(g_phone).normalized();
        impl_->q_phone_to_car = Eigen::Quaterniond(
            Eigen::AngleAxisd(M_PI, axis));
    } else {
        Eigen::Vector3d axis = g_phone.cross(g_car).normalized();
        double angle = std::acos(dot);
        impl_->q_phone_to_car = Eigen::Quaterniond(
            Eigen::AngleAxisd(angle, axis));
    }

    impl_->calibrated = true;
    return true;
}

int CoordTransform::transform(double accel_x, double accel_y, double accel_z,
                               double* car_long_g, double* car_lat_g,
                               double* car_vert_g) {
    if (!impl_->calibrated) {
        // P0-1: Uncalibrated — return zeros to prevent downstream misuse.
        // Phone-frame accel is NOT car-frame accel.
        *car_long_g = 0.0;
        *car_lat_g  = 0.0;
        *car_vert_g = 0.0;
        return -1;
    }

    // Apply rotation: car_accel = q * phone_accel * q_conjugate
    Eigen::Vector3d phone_accel(accel_x, accel_y, accel_z);
    Eigen::Vector3d car_accel = impl_->q_phone_to_car._transformVector(phone_accel);

    // Convert m/s² to G units
    *car_long_g = car_accel.x() / impl_->gravity_mag;
    *car_lat_g  = car_accel.y() / impl_->gravity_mag;
    *car_vert_g = car_accel.z() / impl_->gravity_mag;
    return 0;
}

bool CoordTransform::isCalibrated() const { return impl_->calibrated; }

bool CoordTransform::detectDrift(double gps_heading_deg, double imu_heading_deg) {
    constexpr double kDriftThreshold = 15.0;
    double diff = std::abs(gps_heading_deg - imu_heading_deg);
    if (diff > 180.0) diff = 360.0 - diff;
    return diff > kDriftThreshold;
}

} // namespace codriver
