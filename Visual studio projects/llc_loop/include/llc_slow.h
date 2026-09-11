#ifndef LLC_SLOW_H
#define LLC_SLOW_H
#include "llc_api.h"
#ifdef __cplusplus
extern "C" {
#endif
LlcSlowContext *llcSlowCreate(const LlcConfig *cfg);
void llcSlowReset(LlcSlowContext *ctx);
void llcSlowDestroy(LlcSlowContext *ctx);
void llcSlowStep(LlcSlowContext *ctx, const double in[LOOP_SLOW_IN_N], double out[LOOP_SLOW_OUT_N]);
#ifdef __cplusplus
}
#endif
#endif
