#include "DllHeader.h"
#include "llc_slow.h"
#include <stdlib.h>
#include <string.h>
typedef struct { LlcSlowContext *core; double last_time, held[17]; } SlowAdapter;
DLLEXPORT void plecsSetSizes(struct SimulationSizes *s) {
    s->numInputs=20;s->numOutputs=17;s->numStates=0;s->numParameters=5;
}
DLLEXPORT void plecsStart(struct SimulationState *s) {
    LlcConfig cfg={25e-6,1e-3,680e6,1,1};SlowAdapter *a;
    s->userData=0;s->errorMessage=0;memset(s->outputs,0,17*sizeof(double));
    if(s->parameters[0]!=1||s->parameters[1]!=25e-6||s->parameters[2]!=1e-3||s->parameters[3]!=680e6||s->parameters[4]!=1) {
        s->errorMessage="LLC_Slow requires [1,25e-6,1e-3,680e6,1]";return;
    }
    a=(SlowAdapter*)calloc(1,sizeof(*a));
    if(a) { a->core=llcSlowCreate(&cfg);a->last_time=s->time;a->held[1]=-1;a->held[16]=1; }
    if(!a||!a->core) { free(a);s->errorMessage="Slow context initialization failed";return; }
    s->userData=a;memcpy(s->outputs,a->held,sizeof(a->held));
}
DLLEXPORT void plecsOutput(struct SimulationState *s) {
    SlowAdapter *a=(SlowAdapter*)s->userData;
    if(!a) { memset(s->outputs,0,17*sizeof(double));return; }
    if(s->time>a->last_time) { llcSlowStep(a->core,s->inputs,a->held);a->last_time=s->time; }
    memcpy(s->outputs,a->held,sizeof(a->held));
}
DLLEXPORT void plecsTerminate(struct SimulationState *s) {
    SlowAdapter *a=(SlowAdapter*)s->userData;
    if(a) { llcSlowDestroy(a->core);free(a); }s->userData=0;
}
