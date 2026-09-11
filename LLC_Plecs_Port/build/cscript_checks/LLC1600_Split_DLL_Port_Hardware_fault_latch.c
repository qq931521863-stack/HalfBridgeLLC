#include <stddef.h>
#include <math.h>
static double inputs[4][64],outputs[4][64],disc[64],CurrentTime,NextSampleHit;
#define InputSignal(i,j) inputs[i][j]
#define OutputSignal(i,j) outputs[i][j]
#define DiscState(i) disc[i]
#define IsSampleHit(i) (1)
#define SetErrorMessage(s) ((void)(s))
#include <math.h>
#include <stdint.h>
void start(void){DiscState(0)=0;DiscState(1)=0;}
void output(void){int k; uint32_t fault=(uint32_t)DiscState(0); double raw=InputSignal(0,21);
if(InputSignal(0,19)!=0 && DiscState(1)==0) fault=0;
if(!isfinite(raw)||raw<0||raw>4294967295.0||floor(raw)!=raw) { SetErrorMessage("Invalid hardware fault bits"); fault=0x80000000u; } else fault|=(uint32_t)raw;
for(k=0;k<23;k++) OutputSignal(0,k)=InputSignal(0,k); OutputSignal(0,21)=(double)fault;}
void update(void){double raw=InputSignal(0,21); uint32_t fault=(uint32_t)DiscState(0);
if(InputSignal(0,19)!=0 && DiscState(1)==0) fault=0;
if(isfinite(raw)&&raw>=0&&raw<=4294967295.0&&floor(raw)==raw) fault|=(uint32_t)raw; else fault|=0x80000000u;
DiscState(0)=(double)fault; DiscState(1)=(InputSignal(0,19)!=0);}
