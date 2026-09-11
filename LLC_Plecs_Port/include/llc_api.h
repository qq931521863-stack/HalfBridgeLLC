#ifndef LLC_API_H
#define LLC_API_H
#include <stdint.h>
#include "llc_types.h"
#include "llc_timer.h"
#ifdef __cplusplus
extern "C" {
#endif
enum { LLC_FAST_IN_N=22, LLC_FAST_OUT_N=28, LLC_SLOW_IN_N=20, LLC_SLOW_OUT_N=17 };
enum { LLC_DIAG_MISSING_PFM=1, LLC_DIAG_INVALID_INPUT=2, LLC_DIAG_INVALID_TIMER=4,
       LLC_DIAG_CLEAR_DENIED=8 };
typedef struct LlcContext LlcContext;
typedef struct LlcSlowContext LlcSlowContext;
typedef struct { double fast_ts,slow_ts,timer_hz; uint32_t abi_version,algorithm_profile; } LlcConfig;
typedef struct {
    float v_relay,v_bat,i_bat,v_relay_lpf,v_bat_lpf,i_bat_lpf,v_bat_fir,i_bat_fir;
} LlcSample;
typedef struct {
    int32_t mode,pre_ok,litate,hold_phase;
    uint32_t mode_ticks,fast_tick,last_command_seq;
    float current_ref,current_limit,v_pi_prev_error,v_pi_oldout,i_pi_prev_error,i_pi_oldout;
    float pfm_integral,pfm_kp,pfm_ki,sr_atime,sr_btime,sr_dtime;
} LlcTrace;
LlcContext *llcCreate(const LlcConfig *cfg);
void llcReset(LlcContext *ctx);
void llcDestroy(LlcContext *ctx);
void llcSampleStep(LlcContext *ctx,float vr,float vb,float ib);
void llcReadSample(const LlcContext *ctx,LlcSample *out);
void llcFastStep(LlcContext *ctx,const double in[22],double out[28]);
void llcReadTrace(const LlcContext *ctx,LlcTrace *out);
float PiPwmCompute(pi_str *p,float err,float kp,float ki);
float Compensator_Pid(pi_str *p,float err);
float RampToward(float ref,float target,float step);
float OneOrderForm(float input,filter_str *p);
float SecondOrderForm(float input,filter_str *p);
#ifdef __cplusplus
}
#endif
#endif
