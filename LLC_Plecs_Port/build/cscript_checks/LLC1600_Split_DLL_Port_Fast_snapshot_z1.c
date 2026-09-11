#include <stddef.h>
#include <math.h>
static double inputs[4][64],outputs[4][64],disc[64],CurrentTime,NextSampleHit;
#define InputSignal(i,j) inputs[i][j]
#define OutputSignal(i,j) outputs[i][j]
#define DiscState(i) disc[i]
#define IsSampleHit(i) (1)
#define SetErrorMessage(s) ((void)(s))

void start(void){int k;for(k=0;k<28;k++)DiscState(k)=0;}
void output(void){int k;for(k=0;k<28;k++)OutputSignal(0,k)=DiscState(k);}
void update(void){int k;for(k=0;k<28;k++)DiscState(k)=InputSignal(0,k);}
