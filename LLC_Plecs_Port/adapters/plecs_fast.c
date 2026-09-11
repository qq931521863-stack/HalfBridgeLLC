#include "DllHeader.h"
#include "llc_api.h"
#include <string.h>
#include <stdlib.h>
typedef struct { LlcContext *core; double last_time, held[28]; } FastAdapter;
DLLEXPORT void plecsSetSizes(struct SimulationSizes *s) {
    s->numInputs=22; s->numOutputs=28; s->numStates=0; s->numParameters=5;
}
DLLEXPORT void plecsStart(struct SimulationState *s) {
    LlcConfig cfg={25e-6,1e-3,680e6,1,1}; FastAdapter *a;
    s->errorMessage=0; s->userData=0;
    memset(s->outputs,0,28*sizeof(double));
    if(s->parameters[0]!=1 || s->parameters[1]!=25e-6 || s->parameters[2]!=1e-3 ||
       s->parameters[3]!=680e6 || s->parameters[4]!=1) {
        s->errorMessage="LLC_Fast requires [1,25e-6,1e-3,680e6,1]; original PFM library source unavailable";
        return;
    }
    a=(FastAdapter*)calloc(1,sizeof(*a));
    if(a) { a->core=llcCreate(&cfg); a->last_time=s->time; }
    if(!a || !a->core) { free(a); s->errorMessage="LLC context initialization failed";return; }
    s->userData=a;
}
DLLEXPORT void plecsOutput(struct SimulationState *s) {
    FastAdapter *a=(FastAdapter*)s->userData;
    if(!a) { memset(s->outputs,0,28*sizeof(double)); return; }
    /* t=0 initializes only. Repeated callbacks at the same time are idempotent. */
    if(s->time>a->last_time) {
        llcFastStep(a->core,s->inputs,a->held); a->last_time=s->time;
    }
    memcpy(s->outputs,a->held,sizeof(a->held));
}
DLLEXPORT void plecsTerminate(struct SimulationState *s) {
    FastAdapter *a=(FastAdapter*)s->userData;
    if(a) { llcDestroy(a->core); free(a); } s->userData=0;
}
