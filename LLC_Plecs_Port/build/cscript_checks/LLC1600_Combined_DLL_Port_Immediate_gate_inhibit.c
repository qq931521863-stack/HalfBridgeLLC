#include <stddef.h>
#include <math.h>
static double inputs[4][64],outputs[4][64],disc[64],CurrentTime,NextSampleHit;
#define InputSignal(i,j) inputs[i][j]
#define OutputSignal(i,j) outputs[i][j]
#define DiscState(i) disc[i]
#define IsSampleHit(i) (1)
#define SetErrorMessage(s) ((void)(s))

void start(void){}
void output(void){int k; double allow=(InputSignal(1,21)==0); for(k=0;k<4;k++) OutputSignal(0,k)=InputSignal(0,k)*allow;}
void update(void){}
