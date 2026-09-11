#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "llc_api.h"

static void nearf(float a,float b) { assert(fabsf(a-b)<0.00001f); }
static LlcConfig cfg={25e-6,1e-3,680e6,1,1};
static void inputs(double *in,int mode) {
    memset(in,0,22*sizeof(double));
    in[0]=32; in[1]=32; in[3]=1; in[5]=230; in[7]=1;
    in[8]=mode; in[10]=20; in[11]=40; in[12]=1; in[13]=1; in[14]=3;
}
static void math_sample(void) {
    pi_str p={0}; LlcSample s; LlcContext *c=llcCreate(&cfg);
    assert(c); nearf(PiPwmCompute(&p,2,.04f,.001f),.082f);
    nearf(PiPwmCompute(&p,2,.04f,.001f),.002f);
    memset(&p,0,sizeof(p)); p.Kp=.04f; p.Ki=.001f;
    nearf(Compensator_Pid(&p,2),.082f); nearf(Compensator_Pid(&p,2),.084f);
    nearf(RampToward(0,1,.0002f),.0002f);
    nearf(RampToward(1,1.0001f,.0002f),1.0001f);
    llcSampleStep(c,10,20,2); llcReadSample(c,&s);
    nearf(s.v_relay_lpf,2.391f); nearf(s.v_bat_lpf,4.782f);
    nearf(s.i_bat_lpf,.4782f); nearf(s.v_bat_fir,.02f); nearf(s.i_bat_fir,.002f);
    llcSampleStep(c,10,20,2); llcReadSample(c,&s);
    nearf(s.v_bat_fir,.03998f);
    llcReset(c); llcReadSample(c,&s); nearf(s.v_bat_fir,0);
    llcDestroy(c); cfg.algorithm_profile=0; assert(!llcCreate(&cfg)); cfg.algorithm_profile=1;
}
static void pwm_transactions(void) {
    LlcContext *a=llcCreate(&cfg),*b=llcCreate(&cfg); double in[22],x[28],y[28];
    LlcTrace t; inputs(in,ConCurPWM); in[9]=1;
    llcFastStep(a,in,x); llcFastStep(b,in,y); assert(!memcmp(x,y,sizeof(x)));
    assert(x[15]==5 && x[18]==10 && x[19]==80000); nearf((float)x[17],.0002f);
    llcFastStep(a,in,x); nearf((float)x[17],.0004f);
    llcReadTrace(b,&t); nearf(t.current_ref,.0002f);
    in[7]=2; llcFastStep(a,in,x); nearf((float)x[17],.0002f);
    in[1]=39.9; llcFastStep(a,in,x); assert(x[15]==5);
    in[1]=40; llcFastStep(a,in,x); assert(x[15]==8 && x[9]==0 && x[10]==1);
    in[1]=62; in[7]=3; in[8]=ConCurPWM; llcFastStep(a,in,x);
    assert(x[9]==0 && x[10]==0 && x[11]==0 && x[23]==128);
    llcReset(a); inputs(in,OvLoadPFM); llcFastStep(a,in,x);
    assert(((unsigned)x[27]&1)!=0 && x[9]==0 && x[10]==0 && x[11]==0 && x[12]==0 && x[13]==0);
    llcReset(a); inputs(in,ConCurPWM); in[4]=4194304; llcFastStep(a,in,x);
    assert(x[23]==4194304 && x[9]==0 && x[10]==0 && x[11]==0);
    in[4]=0; llcFastStep(a,in,x); assert(x[23]==4194304 && x[10]==0);
    llcReset(a); inputs(in,ConCurPWM); in[7]=0; llcFastStep(a,in,x); assert(x[15]==0 && x[25]==0);
    in[7]=1; in[10]=NAN; llcFastStep(a,in,x); assert(x[9]==0 && x[10]==0 && x[11]==0 && x[12]==0 && x[13]==0 && x[27]!=0);
    llcReset(a); inputs(in,ConCurPWM); llcFastStep(a,in,x);
    in[7]=2; in[8]=-1; in[10]=2; llcFastStep(a,in,x);
    assert(x[15]==5 && x[25]==2 && x[18]==2 && x[27]==0);
    in[12]=3; llcFastStep(a,in,x); assert(x[27]==2 && x[12]==0);
    llcDestroy(a); llcDestroy(b);
}
static void startup(void) {
    LlcContext *c=llcCreate(&cfg); double in[22],out[28]; int n;
    inputs(in,SoftStar); in[0]=0; in[1]=0; in[12]=0;
    llcFastStep(c,in,out); assert(out[19]==120000); nearf((float)out[20],.035f);
    in[0]=31;
    for(n=0;n<100 && !out[13];n++)llcFastStep(c,in,out);
    assert(out[13]==1 && out[15]==1);
    for(n=0;n<1000;n++)llcFastStep(c,in,out);
    assert(out[15]==1 && out[9]==0 && out[10]==0);
    llcFastStep(c,in,out); assert(out[15]==4);
    llcReset(c); inputs(in,SoftCurSt); in[0]=41; in[12]=2; in[13]=2;
    for(n=0;n<100;n++) { llcFastStep(c,in,out); if(out[13])break; }
    assert(out[13]==1 && out[16]==0);
    for(n=0;n<999;n++)llcFastStep(c,in,out);
    assert(out[16]==0); llcFastStep(c,in,out); assert(out[16]==1 && out[15]==3);
    llcFastStep(c,in,out); assert(out[19]==40000);
    llcDestroy(c);
}
static void settle(LlcContext *c,float vr,float vb,float ib) {
    int n; for(n=0;n<40;n++)llcSampleStep(c,vr,vb,ib);
}
static void bms_hold_protection(void) {
    LlcContext *c=llcCreate(&cfg); double in[22],out[28]; LlcTrace t; int n;
    inputs(in,BmsStar); in[0]=35; settle(c,35,32,0); llcFastStep(c,in,out);
    llcReadTrace(c,&t); assert(t.litate==1 && t.pre_ok==0);
    for(n=0;n<19;n++)llcFastStep(c,in,out);
    assert(out[16]==0); llcFastStep(c,in,out); assert(out[16]==1);
    in[0]=31; settle(c,31,32,0);
    for(n=0;n<19;n++)llcFastStep(c,in,out);
    assert(out[16]==1); llcFastStep(c,in,out); assert(out[16]==2 && out[13]==1);
    for(n=0;n<19999;n++)llcFastStep(c,in,out);
    assert(out[15]==2); llcFastStep(c,in,out); assert(out[15]==5);
    llcReset(c); inputs(in,ConCurPWM); in[1]=40; llcFastStep(c,in,out);
    for(n=0;n<39;n++)llcFastStep(c,in,out);
    llcReadTrace(c,&t); assert(t.mode==8 && t.hold_phase==1 && t.mode_ticks==39);
    llcFastStep(c,in,out); llcReadTrace(c,&t); assert(t.hold_phase==2 && t.mode==8);
    llcFastStep(c,in,out); assert(out[15]==6 && out[9]==0 && out[10]==1);
    for(n=0;n<399;n++)llcFastStep(c,in,out);
    assert(out[15]==6 && out[27]==0); nearf((float)out[20],.449f);
    llcFastStep(c,in,out); assert(out[15]==7 && out[27]==1 && out[9]==0 && out[10]==0 && out[11]==0 && out[12]==0);
    llcReset(c); inputs(in,ConCurPWM); in[1]=9; in[2]=43; in[15]=0;
    llcFastStep(c,in,out); assert(out[23]==0);
    in[15]=1; llcFastStep(c,in,out); assert(out[23]==4608 && out[9]==0 && out[10]==0);
    llcReset(c); inputs(in,ConCurPWM); in[17]=1; llcFastStep(c,in,out); assert(out[26]==1);
    llcFastStep(c,in,out); assert(out[26]==2); nearf((float)out[17],.0004f);
    in[17]=0; llcFastStep(c,in,out); in[17]=1; llcFastStep(c,in,out); assert(out[26]==1);
    llcDestroy(c);
}
static void ov_recovery_and_limits(void) {
    LlcContext *c=llcCreate(&cfg); double in[22],out[28]={0}; int n;
    inputs(in,ConCurPWM);
    for(n=0;n<6000;n++) { in[2]=out[17]; llcFastStep(c,in,out); }
    assert(out[17]>1 && out[17]<1.3 && out[24]==0);
    in[1]=36; in[2]=0; llcFastStep(c,in,out);
    assert(out[24]==1 && out[9]==0 && out[10]==0);
    in[1]=58;
    for(n=0;n<100;n++)llcFastStep(c,in,out);
    in[1]=59; llcFastStep(c,in,out); in[1]=58;
    for(n=0;n<139999;n++)llcFastStep(c,in,out);
    assert(out[24]==1); llcFastStep(c,in,out); assert(out[24]==0 && out[17]==0);
    llcReset(c); inputs(in,ConCurPWM);
    for(n=0;n<3000;n++)llcFastStep(c,in,out);
    nearf((float)out[20],.4f); assert(out[18]==10 && out[23]==0);
    in[10]=.1; llcFastStep(c,in,out); assert(out[9]==0 && out[10]==0 && out[17]==0);
    llcReset(c); inputs(in,ConCurPWM); in[9]=.5; llcFastStep(c,in,out);
    assert(out[25]==0 && out[27]==2 && out[12]==0);
    llcDestroy(c);
}
int main(void) {
    math_sample(); pwm_transactions(); startup(); bms_hold_protection(); ov_recovery_and_limits();
    puts("core tests passed"); return 0;
}
