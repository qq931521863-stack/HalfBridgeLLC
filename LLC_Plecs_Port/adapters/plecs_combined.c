#include "DllHeader.h"
#include "llc_api.h"
#include "llc_slow.h"
#include <stdlib.h>
#include <string.h>
typedef struct {
    LlcContext *fast; LlcSlowContext *slow;
    double last_time, fast_out[28], slow_out[17];
    unsigned int ticks;
} Combined;
DLLEXPORT void plecsSetSizes(struct SimulationSizes *s) {
    s->numInputs=23;s->numOutputs=45;s->numStates=0;s->numParameters=5;
}
DLLEXPORT void plecsStart(struct SimulationState *s) {
    LlcConfig cfg={25e-6,1e-3,680e6,1,1};Combined *a;
    s->userData=0;s->errorMessage=0;memset(s->outputs,0,45*sizeof(double));
    if(s->parameters[0]!=1||s->parameters[1]!=25e-6||s->parameters[2]!=1e-3||s->parameters[3]!=680e6||s->parameters[4]!=1) {
        s->errorMessage="LLC_Combined requires [1,25e-6,1e-3,680e6,1]";return;
    }
    a=(Combined*)calloc(1,sizeof(*a));
    if(a) { a->fast=llcCreate(&cfg);a->slow=llcSlowCreate(&cfg);a->last_time=s->time;a->slow_out[1]=-1;a->slow_out[16]=1; }
    if(!a||!a->fast||!a->slow) {
        if(a) { llcDestroy(a->fast);llcSlowDestroy(a->slow);free(a); }
        s->errorMessage="Combined context initialization failed";return;
    }
    s->userData=a;memcpy(s->outputs+28,a->slow_out,sizeof(a->slow_out));
}
DLLEXPORT void plecsOutput(struct SimulationState *s) {
    Combined *a=(Combined*)s->userData;
    double fi[22],si[20];const double *u=s->inputs,*v;
    if(!a) { memset(s->outputs,0,45*sizeof(double));return; }
    if(s->time>a->last_time) {
        /* Slow samples the last completed fast frame, BEFORE this fast call. */
        memcpy(si,u,20*sizeof(double));
        si[3]=a->fast_out[21];si[4]=a->fast_out[22];si[5]=a->fast_out[13];
        si[6]=a->fast_out[15];si[7]=a->fast_out[16];si[8]=a->fast_out[24];si[9]=a->fast_out[23];
        v=a->slow_out;
        fi[0]=u[0];fi[1]=u[1];fi[2]=u[2];fi[3]=u[10];fi[4]=u[21];fi[5]=u[20];fi[6]=u[22];
        fi[7]=v[0];fi[8]=v[1];fi[9]=v[2];fi[10]=v[3];fi[11]=v[4];fi[12]=v[5];fi[13]=v[6];
        fi[14]=v[7];fi[15]=v[8];fi[16]=v[9];fi[17]=u[19];fi[18]=v[10];fi[19]=v[11];fi[20]=v[12];fi[21]=v[13];
        llcFastStep(a->fast,fi,a->fast_out);
        a->ticks++;
        /* Publishing after Fast means the command first acts on the next fast tick. */
        if(a->ticks%40==0)llcSlowStep(a->slow,si,a->slow_out);
        a->last_time=s->time;
    }
    memcpy(s->outputs,a->fast_out,sizeof(a->fast_out));
    memcpy(s->outputs+28,a->slow_out,sizeof(a->slow_out));
}
DLLEXPORT void plecsTerminate(struct SimulationState *s) {
    Combined *a=(Combined*)s->userData;
    if(a) { llcDestroy(a->fast);llcSlowDestroy(a->slow);free(a); }s->userData=0;
}
