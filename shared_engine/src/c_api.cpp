#include "codriver/c_api.h"
#include "codriver/kalman_filter.h"
#include "codriver/corner_detector.h"
#include "codriver/root_cause.h"
#include "codriver/coach_template.h"
#include "codriver/lap_timer.h"
#include "codriver/types.h"
#include <cstring>

extern "C" {

// ============================================================
// Kalman Filter
// ============================================================
void* c_kalman_create() { return new codriver::KalmanFilter(); }
void c_kalman_destroy(void* h) { if(!h)return; auto o=reinterpret_cast<codriver::KalmanFilter*>(h); delete o; }
void c_kalman_predict(void* h, double dt) { if(!h)return; auto o=reinterpret_cast<codriver::KalmanFilter*>(h); o->predict(dt); }
void c_kalman_update_gps(void* h, double lat, double lon, double alt, double spd, double hdg, double acc) {
    if(!h)return; auto o=reinterpret_cast<codriver::KalmanFilter*>(h); o->updateGPS(lat,lon,alt,spd,hdg,acc);
}
void c_kalman_update_imu(void* h, double ax, double ay, double az, double gx, double gy, double gz) {
    if(!h)return; auto o=reinterpret_cast<codriver::KalmanFilter*>(h); o->updateIMU(ax,ay,az,gx,gy,gz);
}
int c_kalman_get_state(void* h, CFusedPoint* out) {
    if(!h||!out)return -1; auto o=reinterpret_cast<codriver::KalmanFilter*>(h);
    auto fp=o->getState(); out->lat=fp.latitude; out->lon=fp.longitude; out->alt=fp.altitude_m;
    out->speed=fp.speed_kmh; out->heading=fp.heading_deg; out->long_g=fp.long_g;
    out->lat_g=fp.lat_g; out->vert_g=fp.vert_g; out->confidence=fp.confidence; return 0;
}

// ============================================================
// Corner Detector
// ============================================================
void* c_corner_detector_create() { return new codriver::CornerDetector(); }
void c_corner_detector_destroy(void* h) { if(!h)return; auto o=reinterpret_cast<codriver::CornerDetector*>(h); delete o; }
int c_corner_detector_process_point(void* h, double d, double lat, double lon, double spd) {
    if(!h)return 0; auto o=reinterpret_cast<codriver::CornerDetector*>(h);
    int prev=o->getSegmentCount(); o->processPoint(d,lat,lon,spd);
    return (o->getSegmentCount()>prev)?1:0;
}
int c_corner_detector_get_segment_count(void* h) {
    if(!h)return 0; auto o=reinterpret_cast<codriver::CornerDetector*>(h); return o->getSegmentCount();
}
int c_corner_detector_get_segment(void* h, int idx, CCornerInfo* out) {
    if(!h||!out||idx<0)return -1; auto o=reinterpret_cast<codriver::CornerDetector*>(h);
    auto segs=o->getSegments(); if(idx>=(int)segs.size())return -1;
    auto& s=segs[idx];
    std::snprintf(out->seg_id,sizeof(out->seg_id),"%s",s.segment_id?s.segment_id:"");
    std::snprintf(out->type,sizeof(out->type),"%s",s.segment_type?s.segment_type:"");
    std::snprintf(out->dir,sizeof(out->dir),"%s",s.turn_direction?s.turn_direction:"");
    out->entry_d=s.entry_distance_m; out->apex_d=s.apex_distance_m; out->exit_d=s.exit_distance_m;
    out->entry_la=s.entry_lat; out->entry_lo=s.entry_lon;
    out->apex_la=s.apex_lat; out->apex_lo=s.apex_lon;
    out->exit_la=s.exit_lat; out->exit_lo=s.exit_lon;
    out->radius=s.radius_m; out->angle=s.angle_deg; out->diff=s.difficulty;
    return 0;
}

// ============================================================
// Root Cause Engine
// ============================================================
void* c_root_cause_create() { return new codriver::RootCauseEngine(); }
void c_root_cause_destroy(void* h) { if(!h)return; auto o=reinterpret_cast<codriver::RootCauseEngine*>(h); delete o; }
int c_root_cause_analyze(void* h, double bd,double ed,double md,double xd,double lg,double trail,double line,CRootCause* out) {
    if(!h||!out)return-1; auto o=reinterpret_cast<codriver::RootCauseEngine*>(h);
    auto r=o->analyze(bd,ed,md,xd,lg,trail,line);
    std::snprintf(out->cause,sizeof(out->cause),"%s",r.root_cause?r.root_cause:"");
    std::snprintf(out->label,sizeof(out->label),"%s",r.root_cause_label?r.root_cause_label:"");
    std::snprintf(out->conf,sizeof(out->conf),"%s",r.confidence?r.confidence:"");
    std::snprintf(out->sugg,sizeof(out->sugg),"%s",r.suggestion?r.suggestion:"");
    out->loss=r.time_loss_ms; return 0;
}

// ============================================================
// Coach Template
// ============================================================
void* c_coach_template_create() { return new codriver::CoachTemplate(); }
void c_coach_template_destroy(void* h) { if(!h)return; auto o=reinterpret_cast<codriver::CoachTemplate*>(h); delete o; }
int c_coach_template_generate(void* h, const char* seg, const char* cause, double delta, int tier, char* buf, int len) {
    if(!h||!buf||len<=0)return 0; auto o=reinterpret_cast<codriver::CoachTemplate*>(h);
    auto msg=o->generate(seg,cause,delta,tier);
    int n=std::snprintf(buf,len,"%s",msg.text?msg.text:""); return (n<len)?n:len-1;
}

// ============================================================
// Lap Timer
// ============================================================
void* c_lap_timer_create() { return new codriver::LapTimer(); }
void c_lap_timer_destroy(void* h) { if(!h)return; auto o=reinterpret_cast<codriver::LapTimer*>(h); delete o; }
void c_lap_timer_set_line(void* h, double la1,double lo1,double la2,double lo2) {
    if(!h)return; auto o=reinterpret_cast<codriver::LapTimer*>(h); o->setStartLine(la1,lo1,la2,lo2);
}
int64_t c_lap_timer_process(void* h, double lat, double lon, int64_t ts, double* dist, int* dir) {
    if(!h)return 0; auto o=reinterpret_cast<codriver::LapTimer*>(h);
    double d=0; int dr=0; auto lap=o->processPoint(lat,lon,ts,&d,&dr);
    if(dist)*dist=d; if(dir)*dir=dr; return lap;
}
int c_lap_timer_count(void* h) { if(!h)return 0; auto o=reinterpret_cast<codriver::LapTimer*>(h); return o->lapCount(); }
double c_lap_timer_total_dist(void* h) { if(!h)return 0; auto o=reinterpret_cast<codriver::LapTimer*>(h); return o->totalDistance(); }

} // extern "C"
