#include "codriver/kalman_filter.h"
#include <Eigen/Dense>

namespace codriver {

class KalmanFilter::Impl {
public:
    // 15-state Kalman: position(3), velocity(3), attitude(4), accel_bias(3), gyro_bias(3)
    Eigen::Matrix<double, 15, 1> x;
    Eigen::Matrix<double, 15, 15> P;
    bool initialized = false;
};

KalmanFilter::KalmanFilter() : impl_(new Impl()) {
    impl_->x.setZero();
    impl_->P.setIdentity();
    impl_->P *= 100.0;
}

KalmanFilter::~KalmanFilter() { delete impl_; }

void KalmanFilter::predict(double dt) { /* TODO: EKF predict step */ }
void KalmanFilter::updateGPS(double lat, double lon, double alt,
                              double speed, double heading, double accuracy) { /* TODO */ }
void KalmanFilter::updateIMU(double accel_x, double accel_y, double accel_z,
                              double gyro_x, double gyro_y, double gyro_z) { /* TODO */ }

FusedPoint KalmanFilter::getState() const {
    FusedPoint p{};
    // TODO: extract state
    return p;
}

} // namespace codriver
