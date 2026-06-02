#include "codriver/c_api.h"
#include "codriver/kalman_filter.h"
#include "codriver/corner_detector.h"
#include "codriver/root_cause.h"
#include "codriver/coach_template.h"
#include "codriver/lap_timer.h"
#include "codriver/coord_transform.h"
#include "codriver/brake_detector.h"
#include "codriver/corner_speed_compare.h"
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

// ============================================================
// Coord Transform (Phase 2)
// ============================================================
void* c_coord_transform_create() { return new codriver::CoordTransform(); }
void c_coord_transform_destroy(void* h) { if(!h)return; auto o=reinterpret_cast<codriver::CoordTransform*>(h); delete o; }
int c_coord_transform_calibrate(void* h, double ax, double ay, double az) {
    if(!h)return 0; auto o=reinterpret_cast<codriver::CoordTransform*>(h);
    return o->calibrate(ax, ay, az) ? 1 : 0;
}
int c_coord_transform_transform(void* h, double ax, double ay, double az,
                                 double* clg, double* clat, double* cv) {
    if(!h||!clg||!clat||!cv)return -1;
    auto o=reinterpret_cast<codriver::CoordTransform*>(h);
    return o->transform(ax, ay, az, clg, clat, cv);
}
int c_coord_transform_is_calibrated(void* h) {
    if(!h)return 0; auto o=reinterpret_cast<codriver::CoordTransform*>(h);
    return o->isCalibrated()?1:0;
}
int c_coord_transform_detect_drift(void* /*h*/, double gps_hdg, double imu_hdg) {
    return codriver::CoordTransform::detectDrift(gps_hdg, imu_hdg) ? 1 : 0;
}

// ============================================================
// Brake Detector (Phase 2.2)
// ============================================================
void* c_brake_detector_create() { return new codriver::BrakeDetector(); }
void c_brake_detector_destroy(void* h) { if(!h)return; auto o=reinterpret_cast<codriver::BrakeDetector*>(h); delete o; }
int c_brake_detector_process_point(void* h, double lat, double lon, double dist,
                                    double speed, double long_g, int64_t ts) {
    if(!h)return 0; auto o=reinterpret_cast<codriver::BrakeDetector*>(h);
    codriver::FusedPoint fp{};
    fp.latitude=lat; fp.longitude=lon; fp.track_distance_m=dist;
    fp.speed_kmh=speed; fp.long_g=long_g; fp.timestamp_ms=ts;
    return o->processPoint(fp)?1:0;
}
int c_brake_detector_get_event_count(void* h) {
    if(!h)return 0; auto o=reinterpret_cast<codriver::BrakeDetector*>(h);
    return o->getEventCount();
}
int c_brake_detector_get_event(void* h, int idx, CBrakeEvent* out) {
    if(!h||!out||idx<0)return -1; auto o=reinterpret_cast<codriver::BrakeDetector*>(h);
    auto e=o->getEvent(idx); if(!e)return -1;
    out->brake_lat=e->brake_lat; out->brake_lon=e->brake_lon;
    out->brake_dist=e->brake_distance_m; out->brake_spd=e->brake_speed_kmh;
    out->brake_ts=e->brake_timestamp_ms;
    out->peak_g=e->peak_decel_g; out->peak_dist=e->peak_decel_distance_m;
    out->rel_lat=e->release_lat; out->rel_lon=e->release_lon;
    out->rel_dist=e->release_distance_m; out->rel_spd=e->release_speed_kmh;
    out->release_ts=e->release_timestamp_ms;
    out->dur_ms=e->braking_duration_ms; out->trail_ms=e->trail_brake_duration_ms;
    out->release_ms=e->brake_release_duration_ms; out->speed_drop=e->speed_drop_kmh;
    std::snprintf(out->seg_id, sizeof(out->seg_id), "%s", e->segment_id[0] ? e->segment_id : "");
    return 0;
}
void c_brake_detector_reset(void* h) {
    if(!h)return; auto o=reinterpret_cast<codriver::BrakeDetector*>(h); o->reset();
}

// ============================================================
// Corner Speed Compare (Phase 2.3)
// ============================================================
void* c_corner_speed_create() { return new codriver::CornerSpeedCompare(); }
void c_corner_speed_destroy(void* h) { if(!h)return; auto o=reinterpret_cast<codriver::CornerSpeedCompare*>(h); delete o; }
int c_corner_speed_compare(void* h,
    const char* seg_id, double ref_entry, double ref_min, double ref_exit, double ref_lat,
    double act_entry, double act_min, double act_exit, double act_lat,
    CCornerSpeedDelta* out) {
    if(!h||!out)return -1; auto o=reinterpret_cast<codriver::CornerSpeedCompare*>(h);
    codriver::TrackSegment seg{};
    seg.segment_id = seg_id;
    seg.reference_entry_speed_kmh = ref_entry;
    seg.reference_speed_kmh = ref_min;
    seg.reference_exit_speed_kmh = ref_exit;
    seg.reference_lateral_g = ref_lat;
    auto d = o->compare(seg, act_entry, act_min, act_exit, act_lat);
    out->entry_kmh = d.actual_entry_kmh; out->ref_entry = d.reference_entry_kmh;
    out->entry_delta = d.entry_delta_kmh;
    out->min_kmh = d.actual_min_kmh; out->ref_min = d.reference_min_kmh;
    out->min_delta = d.min_delta_kmh;
    out->exit_kmh = d.actual_exit_kmh; out->ref_exit = d.reference_exit_kmh;
    out->exit_delta = d.exit_delta_kmh;
    out->lat_g = d.actual_lat_g; out->ref_lat_g = d.reference_lat_g;
    out->lat_delta = d.lat_g_delta;
    std::snprintf(out->seg_id, sizeof(out->seg_id), "%s", d.segment_id ? d.segment_id : "");
    return 0;
}

} // extern "C"
