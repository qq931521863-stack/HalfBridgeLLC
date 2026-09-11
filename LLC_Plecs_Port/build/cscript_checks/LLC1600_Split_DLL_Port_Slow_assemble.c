#include <stddef.h>
#include <math.h>
static double inputs[4][64],outputs[4][64],disc[64],CurrentTime,NextSampleHit;
#define InputSignal(i,j) inputs[i][j]
#define OutputSignal(i,j) outputs[i][j]
#define DiscState(i) disc[i]
#define IsSampleHit(i) (1)
#define SetErrorMessage(s) ((void)(s))

void start(void){}
void output(void){int k; int ix[7]={21,22,13,15,16,24,23};for(k=0;k<20;k++)OutputSignal(0,k)=InputSignal(0,k);for(k=0;k<7;k++)OutputSignal(0,3+k)=InputSignal(1,ix[k]);}
void update(void){}
