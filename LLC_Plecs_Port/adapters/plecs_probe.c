#include "DllHeader.h"
DLLEXPORT void plecsSetSizes(struct SimulationSizes *s) {
    s->numInputs=22; s->numOutputs=28; s->numStates=1; s->numParameters=2;
}
DLLEXPORT void plecsStart(struct SimulationState *s) {
    int k;
    s->errorMessage=0; s->userData=0;
    for(k=0;k<28;k++) s->outputs[k]=0;
    s->states[0]=0;
    if(s->parameters[0]!=1 || s->parameters[1]!=25e-6)
        s->errorMessage="Probe requires ABI=1 and Ts=25e-6";
}
DLLEXPORT void plecsOutput(struct SimulationState *s) {
    int k;
    for(k=0;k<28;k++) s->outputs[k]=0;
    if(s->errorMessage) return;
    s->outputs[19]=s->inputs[0]; s->outputs[20]=s->inputs[1];
    s->outputs[21]=s->inputs[2]; s->outputs[26]=s->states[0]++;
}
DLLEXPORT void plecsTerminate(struct SimulationState *s) { s->userData=0; }
