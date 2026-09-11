#include <stddef.h>
#include <math.h>
static double inputs[4][64],outputs[4][64],disc[64],CurrentTime,NextSampleHit;
#define InputSignal(i,j) inputs[i][j]
#define OutputSignal(i,j) outputs[i][j]
#define DiscState(i) disc[i]
#define IsSampleHit(i) (1)
#define SetErrorMessage(s) ((void)(s))

void start(void){}
void output(void){int k; for(k=0;k<12;k++) OutputSignal(0,k)=InputSignal(0,k);
OutputSignal(0,12)=InputSignal(0,26);
OutputSignal(0,13)=(InputSignal(0,44)!=0 && InputSignal(0,0)>0 && (((unsigned int)InputSignal(0,27)&7)==0));
OutputSignal(0,14)=InputSignal(1,16);}
void update(void){}
