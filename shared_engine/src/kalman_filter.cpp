#define _USE_MATH_DEFINES
#include "codriver/kalman_filter.h"
#include <Eigen/Dense>
#include <cmath>

namespace codriver {

static constexpr int kDim = 9;
using Vec = Eigen::Matrix<double, kDim, 1>;
using Mat = Eigen::Matrix<double, kDim, kDim>;

class KalmanFilter::Impl {
public:
    Vec x; Mat P, Q; bool init = false;
    double rlat=0, rlon=0; bool rset=false;
    double lax=0,lay=0,laz=0,lgz=0; bool imu=false;
};

KalmanFilter::KalmanFilter() : impl_(new Impl()) {
    impl_->x.setZero(); impl_->P.setIdentity(); impl_->P *= 100.0;
    impl_->Q.setIdentity(); impl_->Q *= 0.01;
}
KalmanFilter::~KalmanFilter() { delete impl_; }

void KalmanFilter::predict(double dt) {
    if (!impl_->init || !impl_->imu || dt<=0) return;
    auto& x=impl_->x;
    double n=x(0),e=x(1),vn=x(2),ve=x(3),h=x(4),ba=x(5),bv=x(6),bg=x(7);
    double ax=impl_->lax-ba, ay=impl_->lay-bv, gz=impl_->lgz-bg;
    double sh=std::sin(h), ch=std::cos(h), d2=dt*dt;
    Vec xp; xp.setZero();
    xp(0)=n+vn*dt+0.5*(ax*ch-ay*sh)*d2;
    xp(1)=e+ve*dt+0.5*(ax*sh+ay*ch)*d2;
    xp(2)=vn+(ax*ch-ay*sh)*dt;
    xp(3)=ve+(ax*sh+ay*ch)*dt;
    xp(4)=h+gz*dt; xp(5)=ba; xp(6)=bv; xp(7)=bg;
    Mat F=Mat::Identity();
    F(0,2)=dt; F(1,3)=dt;
    F(2,4)=-ax*sh-ay*ch; F(2,5)=-ch; F(2,6)=sh;
    F(3,4)=ax*ch-ay*sh; F(3,5)=-sh; F(3,6)=-ch;
    F(4,7)=-dt;
    impl_->Q.setIdentity();
    double s2=0.25*d2*d2; impl_->Q(0,0)=s2; impl_->Q(1,1)=s2;
    impl_->Q(2,2)=0.25*d2; impl_->Q(3,3)=0.25*d2;
    impl_->Q(4,4)=1e-4*d2; impl_->Q(5,5)=1e-6*dt; impl_->Q(6,6)=1e-6*dt;
    impl_->Q(8,8)=1e-8*dt;
    impl_->x=xp; impl_->P=(F*impl_->P*F.transpose()+impl_->Q).eval();
}

void KalmanFilter::updateGPS(double lat,double lon,double,double spd,double hdg,double acc){
    constexpr double MPD=111320.0, PI=3.14159265358979;
    if(!impl_->rset){impl_->rlat=lat;impl_->rlon=lon;impl_->rset=true;}
    double cl=std::cos(impl_->rlat*PI/180.0), MPL=MPD*cl;
    double nm=(lat-impl_->rlat)*MPD, em=(lon-impl_->rlon)*MPL;
    if(!impl_->init){impl_->x(0)=nm;impl_->x(1)=em;impl_->x(2)=spd*std::cos(hdg);impl_->x(3)=spd*std::sin(hdg);impl_->x(4)=hdg;impl_->init=true;return;}
    Eigen::Matrix<double,4,1> z; z<<nm,em,spd,hdg;
    Eigen::Matrix<double,4,kDim> H=Eigen::Matrix<double,4,kDim>::Zero();
    H(0,0)=H(1,1)=H(2,2)=H(3,4)=1.0;
    Eigen::Matrix<double,4,4> R=Eigen::Matrix<double,4,4>::Zero();
    double pv=acc*acc; R(0,0)=R(1,1)=pv; R(2,2)=1.0; R(3,3)=0.01;
    Eigen::Matrix<double,4,1> y=z-H*impl_->x;
    y(3)=std::atan2(std::sin(y(3)),std::cos(y(3)));
    Eigen::Matrix<double,kDim,4> PHt=impl_->P*H.transpose();
    Eigen::Matrix<double,4,4> S=H*PHt+R;
    Eigen::Matrix<double,kDim,4> K=PHt*S.inverse();
    impl_->x+=K*y; impl_->P=(Mat::Identity()-K*H)*impl_->P;
    impl_->x(4)=std::atan2(std::sin(impl_->x(4)),std::cos(impl_->x(4)));
}

void KalmanFilter::updateIMU(double ax,double ay,double az,double,double,double gz){
    impl_->lax=ax;impl_->lay=ay;impl_->laz=az;impl_->lgz=gz;impl_->imu=true;
}

FusedPoint KalmanFilter::getState() const {
    FusedPoint p{}; if(!impl_->init)return p;
    constexpr double MPD=111320.0, PI=3.14159265358979;
    double cl=std::cos(impl_->rlat*PI/180.0), MPL=MPD*cl;
    p.latitude=impl_->rlat+impl_->x(0)/MPD; p.longitude=impl_->rlon+impl_->x(1)/MPL;
    p.speed_kmh=std::sqrt(impl_->x(2)*impl_->x(2)+impl_->x(3)*impl_->x(3))*3.6;
    p.heading_deg=impl_->x(4)*180.0/PI;
    double h=impl_->x(4),ch=std::cos(h),sh=std::sin(h);
    double ax=impl_->lax-impl_->x(5),ay=impl_->lay-impl_->x(6);
    p.long_g=(ax*ch+ay*sh)/9.81; p.lat_g=(-ax*sh+ay*ch)/9.81;
    double tr=impl_->P.trace(); p.confidence=1.0/(1.0+tr*0.1);
    if(p.confidence>1.0)p.confidence=1.0; return p;
}

} // namespace codriver
