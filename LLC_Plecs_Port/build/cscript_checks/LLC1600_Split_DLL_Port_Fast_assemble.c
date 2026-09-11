#include <stddef.h>
#include <math.h>
static double inputs[4][64],outputs[4][64],disc[64],CurrentTime,NextSampleHit;
#define InputSignal(i,j) inputs[i][j]
#define OutputSignal(i,j) outputs[i][j]
#define DiscState(i) disc[i]
#define IsSampleHit(i) (1)
#define SetErrorMessage(s) ((void)(s))

void start(void){}
void output(void){int k; for(k=0;k<3;k++) OutputSignal(0,k)=InputSignal(0,k);
OutputSignal(0,3)=InputSignal(0,10);OutputSignal(0,4)=InputSignal(0,21);OutputSignal(0,5)=InputSignal(0,20);OutputSignal(0,6)=InputSignal(0,22);
for(k=0;k<10;k++)OutputSignal(0,7+k)=InputSignal(1,k);
OutputSignal(0,17)=InputSignal(0,19);for(k=0;k<4;k++)OutputSignal(0,18+k)=InputSignal(1,10+k);}
void update(void){}
