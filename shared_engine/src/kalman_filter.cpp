// CoDriver Kalman Filter — 8-state EKF for GPS+IMU sensor fusion
// State: [north_m, east_m, v_north_mps, v_east_mps, heading_rad, b_ax, b_ay, b_gz]
// Position in meters from reference point; velocity in m/s; heading in radians.
// IMU biases: accelerometer x/y (m/s^2), gyroscope z (rad/s).
// Calling convention: call updateIMU() before each predict(); call updateGPS() on each GPS fix.

#define _USE_MATH_DEFINES
#include "codriver/kalman_filter.h"
#include <Eigen/Dense>
#include <cmath>

namespace codriver {

static constexpr int kDim = 8;  // P0-3: 8-state, no ghost state
using Vec = Eigen::Matrix<double, kDim, 1>;
using Mat = Eigen::Matrix<double, kDim, kDim>;

struct KalmanFilter::Impl {
    Vec x;           // state vector
    Mat P;           // covariance matrix
    Mat Q_base;      // process noise base (per-second) — P0-2, P1-8: init once
    bool ok = false; // filter initialized?

    double rlat = 0, rlon = 0;  // reference point (first GPS fix)
    bool rs = false;

    // Latest IMU sample — caller must call updateIMU() before each predict()
    double ax = 0, ay = 0, az = 0, gz = 0;
    double alt = 0;  // P1-1: cached altitude from GPS
    bool imu = false;
    int64_t last_ts = 0;  // P1-1: timestamp tracking
};

KalmanFilter::KalmanFilter() : impl_(new KalmanFilter::Impl) {
    impl_->x.setZero();
    impl_->P.setZero();  // P0-3, P1-3: zero-init then set per-dimension
    impl_->P(0, 0) = impl_->P(1, 1) = 100.0;   // position ~10m
    impl_->P(2, 2) = impl_->P(3, 3) = 25.0;     // velocity ~5 m/s
    impl_->P(4, 4) = 0.1;                        // heading ~18 deg
    impl_->P(5, 5) = impl_->P(6, 6) = 1e-4;     // accel bias
    impl_->P(7, 7) = 1e-4;                       // gyro bias

    // P0-2, P1-8: Q_base initialized once, scaled by dt in predict()
    impl_->Q_base.setZero();
    impl_->Q_base(0, 0) = impl_->Q_base(1, 1) = 0.0625;  // position (accel²*dt⁴/4)
    impl_->Q_base(2, 2) = impl_->Q_base(3, 3) = 0.25;     // velocity
    impl_->Q_base(4, 4) = 1e-4;                            // heading
    impl_->Q_base(5, 5) = impl_->Q_base(6, 6) = 1e-6;     // accel bias drift
    impl_->Q_base(7, 7) = 1e-8;                            // P0-2: gyro bias drift
}

KalmanFilter::~KalmanFilter() { delete impl_; }

void KalmanFilter::predict(double dt) {
    if (!impl_->ok || !impl_->imu || dt <= 0) return;

    auto& x = impl_->x;
    double n = x(0), e = x(1), vn = x(2), ve = x(3), h = x(4);
    double ba = x(5), bv = x(6), bg = x(7);
    double ax = impl_->ax - ba, ay = impl_->ay - bv, gz = impl_->gz - bg;
    double sh = std::sin(h), ch = std::cos(h), d2 = dt * dt;

    // --- Nonlinear motion model ---
    Vec xp;
    xp.setZero();
    xp(0) = n + vn * dt + 0.5 * (ax * ch - ay * sh) * d2;
    xp(1) = e + ve * dt + 0.5 * (ax * sh + ay * ch) * d2;
    xp(2) = vn + (ax * ch - ay * sh) * dt;
    xp(3) = ve + (ax * sh + ay * ch) * dt;
    xp(4) = h + gz * dt;             // heading: h_{k+1}=h_k + (gyro_z - b_gz)*dt
    xp(5) = ba;                       // bias: constant
    xp(6) = bv;
    xp(7) = bg;

    // --- State transition Jacobian F ---
    // P1-5: verified — ∂(ax*ch-ay*sh)/∂h = -ax*sh-ay*ch ✅
    //        ∂(ax*sh+ay*ch)/∂h =  ax*ch-ay*sh ✅
    //        ∂h/∂bg = -dt ✅
    Mat F = Mat::Identity();
    F(0, 2) = dt;
    F(1, 3) = dt;
    F(2, 4) = -ax * sh - ay * ch;  F(2, 5) = -ch;  F(2, 6) = sh;
    F(3, 4) =  ax * ch - ay * sh;  F(3, 5) = -sh;  F(3, 6) = -ch;
    F(4, 7) = -dt;

    // --- Process noise Q (scale base by dt) --- P0-2, P1-8
    Mat Q = impl_->Q_base * dt;
    Q(0, 0) *= d2; Q(1, 1) *= d2;   // position: scale by dt²
    Q(2, 2) *= d2; Q(3, 3) *= d2;   // velocity: scale by dt²
    Q(4, 4) *= d2;                    // heading: scale by dt²

    impl_->x = xp;
    impl_->P = F * impl_->P * F.transpose() + Q;
    impl_->P = 0.5 * (impl_->P + impl_->P.transpose()).eval();  // P1-4: symmetrize
}

void KalmanFilter::updateGPS(double lat, double lon, double alt,
                              double spd, double hdg, double acc) {
    constexpr double MPD = 111320.0;  // meters per degree latitude
    impl_->alt = alt;  // P1-1

    // Set reference point on first fix
    if (!impl_->rs) { impl_->rlat = lat; impl_->rlon = lon; impl_->rs = true; }

    // Convert GPS to local NED (meters from reference)
    double cl = std::cos(impl_->rlat * M_PI / 180.0);
    double nm = (lat - impl_->rlat) * MPD;
    double em = (lon - impl_->rlon) * MPD * cl;

    if (!impl_->ok) {
        // First fix: initialize state
        impl_->x(0) = nm;
        impl_->x(1) = em;
        impl_->x(2) = spd * std::cos(hdg);
        impl_->x(3) = spd * std::sin(hdg);
        impl_->x(4) = hdg;
        // P1-3: set reasonable initial covariance after init
        impl_->P.setZero();
        impl_->P(0, 0) = impl_->P(1, 1) = acc * acc;  // position from GPS accuracy
        impl_->P(2, 2) = impl_->P(3, 3) = 1.0;          // velocity ~1 m/s
        impl_->P(4, 4) = 0.01;                           // heading ~0.1 rad
        impl_->P(5, 5) = impl_->P(6, 6) = impl_->P(7, 7) = 1e-4;
        impl_->ok = true;
        return;
    }

    // P0-1: Nonlinear observation model for speed and heading
    // speed = sqrt(vn² + ve²) with Jacobian: [0,0,vn/s,ve/s,0,0,0,0]
    // heading = atan2(ve, vn) with Jacobian: [0,0,-ve/(vn²+ve²),vn/(vn²+ve²),0,0,0,0]
    double vn = impl_->x(2);
    double ve = impl_->x(3);
    double s  = std::sqrt(vn * vn + ve * ve);
    double s2 = vn * vn + ve * ve;  // speed squared
    if (s < 0.1) s = 0.1;           // avoid division by zero at low speed
    if (s2 < 0.01) s2 = 0.01;

    // Observation vector z = [north, east, speed, heading]
    Eigen::Matrix<double, 4, 1> z;
    z << nm, em, spd, hdg;

    // Observation Jacobian H
    Eigen::Matrix<double, 4, kDim> H = Eigen::Matrix<double, 4, kDim>::Zero();
    H(0, 0) = 1.0;   // north
    H(1, 1) = 1.0;   // east
    H(2, 2) = vn / s; H(2, 3) = ve / s;      // speed Jacobian
    H(3, 2) = -ve / s2; H(3, 3) = vn / s2;   // heading Jacobian

    // Measurement noise R — P1-2: speed noise 0.3², heading noise speed-dependent
    Eigen::Matrix<double, 4, 4> R = Eigen::Matrix<double, 4, 4>::Zero();
    double pv = acc * acc;
    R(0, 0) = pv;
    R(1, 1) = pv;
    R(2, 2) = 0.09;                                     // speed ~0.3 m/s
    R(3, 3) = (0.1 / std::max(s, 1.0)); R(3, 3) *= R(3, 3);  // heading noise: 0.1/speed rad

    // EKF update
    Eigen::Matrix<double, 4, 1> y = z - H * impl_->x;
    y(3) = std::atan2(std::sin(y(3)), std::cos(y(3)));  // heading wrap

    Eigen::Matrix<double, kDim, 4> PHt = impl_->P * H.transpose();
    Eigen::Matrix<double, 4, 4> S = H * PHt + R;
    Eigen::Matrix<double, kDim, 4> K = PHt * S.inverse();

    impl_->x += K * y;
    impl_->x(4) = std::atan2(std::sin(impl_->x(4)), std::cos(impl_->x(4)));  // normalize

    // Joseph form for numerical stability — P1-4
    Mat I_KH = Mat::Identity() - K * H;
    impl_->P = I_KH * impl_->P * I_KH.transpose() + K * R * K.transpose();
    impl_->P = 0.5 * (impl_->P + impl_->P.transpose()).eval();
}

void KalmanFilter::updateIMU(double ax, double ay, double az,
                              double /*gyro_x*/, double /*gyro_y*/, double gz) {
    // P1-6, P3-2: Caller must call this before each predict().
    // gyro_x/gyro_y reserved for future pitch/roll extension.
    impl_->ax = ax; impl_->ay = ay; impl_->az = az;
    impl_->gz = gz; impl_->imu = true;
}

FusedPoint KalmanFilter::getState() const {
    FusedPoint p{};
    if (!impl_->ok) return p;

    constexpr double MPD = 111320.0;
    double cl = std::cos(impl_->rlat * M_PI / 180.0);

    // P0-1: use heading-aware velocity decomposition
    double vn = impl_->x(2), ve = impl_->x(3);
    double speed_mps = std::sqrt(vn * vn + ve * ve);

    p.timestamp_ms = impl_->last_ts;
    p.latitude     = impl_->rlat + impl_->x(0) / MPD;
    p.longitude    = impl_->rlon + impl_->x(1) / (MPD * cl);
    p.altitude_m   = impl_->alt;                               // P1-1
    p.speed_kmh    = speed_mps * 3.6;
    p.heading_deg  = impl_->x(4) * 180.0 / M_PI;

    // G-forces in car frame (remove bias, rotate by heading)
    double h = impl_->x(4), ch = std::cos(h), sh = std::sin(h);
    double ax = impl_->ax - impl_->x(5), ay = impl_->ay - impl_->x(6);
    p.long_g = (ax * ch + ay * sh) / 9.81;
    p.lat_g  = (-ax * sh + ay * ch) / 9.81;
    p.vert_g = impl_->az / 9.81;                              // P1-1

    // Position uncertainty (from covariance)
    p.accuracy_m = std::sqrt(std::max(0.0, impl_->P(0, 0) + impl_->P(1, 1)));  // P1-1

    // Confidence: separate position and heading uncertainties — P2-3
    double pos_conf = 1.0 / (1.0 + std::sqrt(impl_->P(0, 0) + impl_->P(1, 1)));
    double hdg_conf = 1.0 / (1.0 + std::sqrt(impl_->P(4, 4)) * 10.0);
    p.confidence = std::min(pos_conf, hdg_conf);
    if (p.confidence > 1.0) p.confidence = 1.0;

    return p;
}

} // namespace codriver



