#include <stddef.h>
#include <math.h>
static double inputs[4][64],outputs[4][64],disc[64],CurrentTime,NextSampleHit;
#define InputSignal(i,j) inputs[i][j]
#define OutputSignal(i,j) outputs[i][j]
#define DiscState(i) disc[i]
#define IsSampleHit(i) (1)
#define SetErrorMessage(s) ((void)(s))
#include "D:/Work/1600W/代码分析/LLC_Plecs_Port/model/gate/llc_gate_events.c"
#include <float.h>
static LlcGate committed,candidate;
void start(void){llcGateReset(&committed); llcGateReset(&candidate); DiscState(0)=0; NextSampleHit=DBL_MAX;}
void output(void){double u[15],y[4],next; int k;
for(k=0;k<15;k++) u[k]=InputSignal(0,k);
if(!llcGateEvaluate(&committed,CurrentTime,IsSampleHit(0),u,&candidate,y,&next)) SetErrorMessage("Invalid LLC timer frame");
for(k=0;k<4;k++) OutputSignal(0,k)=y[k];
NextSampleHit=isfinite(next)?next:DBL_MAX;}
void update(void){committed=candidate; DiscState(0)+=1;}
