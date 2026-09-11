#ifndef LLC_SLOW_H
#define LLC_SLOW_H
#include "llc_api.h"
#ifdef __cplusplus
extern "C" {
#endif
LlcSlowContext *llcSlowCreate(const LlcConfig *cfg);
void llcSlowReset(LlcSlowContext *ctx);
void llcSlowDestroy(LlcSlowContext *ctx);
void llcSlowStep(LlcSlowContext *ctx,const double in[20],double out[17]);
#ifdef __cplusplus
}
#endif
#endif
