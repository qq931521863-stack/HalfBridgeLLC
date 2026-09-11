#ifndef LLC_API_H
#define LLC_API_H
#include <stdint.h>
#include "llc_types.h"
#include "loop_abi.h"
#ifdef __cplusplus
extern "C" {
#endif

enum { LLC_DIAG_MISSING_PFM = 1, LLC_DIAG_INVALID_INPUT = 2 };

typedef struct LlcContext LlcContext;
typedef struct LlcSlowContext LlcSlowContext;
typedef struct {
	double fast_ts;
	double slow_ts;
	uint32_t abi_version;
} LlcConfig;

typedef struct {
	float v_relay, v_bat, i_bat;
	float v_relay_lpf, v_bat_lpf, i_bat_lpf;
	float v_bat_fir, i_bat_fir;
} LlcSample;

LlcContext *llcCreate(const LlcConfig *cfg);
void llcReset(LlcContext *ctx);
void llcDestroy(LlcContext *ctx);
void llcSampleStep(LlcContext *ctx, float vr, float vb, float ib);
void llcFastStep(LlcContext *ctx, const double in[LOOP_FAST_IN_N], double out[LOOP_FAST_OUT_N]);

LlcSlowContext *llcSlowCreate(const LlcConfig *cfg);
void llcSlowReset(LlcSlowContext *ctx);
void llcSlowDestroy(LlcSlowContext *ctx);
void llcSlowStep(LlcSlowContext *ctx, const double in[LOOP_SLOW_IN_N], double out[LOOP_SLOW_OUT_N]);

float PiPwmCompute(pi_str *p, float err, float kp, float ki);
float Compensator_Pid(pi_str *p, float err);
float RampToward(float ref, float target, float step);
float OneOrderForm(float input, filter_str *p);
float SecondOrderForm(float input, filter_str *p);

#ifdef __cplusplus
}
#endif
#endif
