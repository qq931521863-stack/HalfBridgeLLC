#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "llc_slow.h"
static LlcConfig cfg={25e-6,1e-3,680e6,1,1};
static void input(double *in) {
    memset(in,0,20*sizeof(*in)); in[10]=1; in[11]=230; in[12]=12;
    in[17]=40; in[18]=.5;
}
static void ticks(LlcSlowContext *c,double *in,double *out,int n) {
    int j; for(j=0;j<n;j++) { llcSlowStep(c,in,out); if(out[0]>0&&out[1]>=0)in[6]=out[1]; }
}
static void startup_and_instances(void) {
    LlcSlowContext *a=llcSlowCreate(&cfg),*b=llcSlowCreate(&cfg); double in[20],out[17],other[17];
    assert(a&&b); input(in); llcSlowStep(a,in,out); llcSlowStep(b,in,other);
    assert(!memcmp(out,other,sizeof(out))); assert(out[0]==0 && out[15]==1 && out[16]==1);
    ticks(a,in,out,3); assert(out[0]==0); llcSlowStep(a,in,out);
    assert(out[0]==1 && out[1]==0 && out[2]==1088 && out[7]==0);
    ticks(a,in,out,894); assert(out[15]==899 && out[7]==0);
    llcSlowStep(a,in,out); assert(out[7]==1 && out[1]==0);
    ticks(a,in,out,9); assert(out[1]==0); llcSlowStep(a,in,out);
    assert(out[1]==1 && out[2]==18 && out[10]==3 && out[12]==0);
    in[6]=out[1];{ double seq=out[0]; ticks(a,in,out,20); assert(out[0]==seq); }
    llcSlowStep(b,in,other); assert(other[15]==2 && other[0]==0);
    llcSlowDestroy(a); llcSlowDestroy(b);
}
static void cc_cv_requests(void) {
    LlcSlowContext *c=llcSlowCreate(&cfg); double in[20],out[17];
    input(in); in[1]=32; in[3]=32; in[13]=1; in[14]=1; in[15]=1;
    ticks(c,in,out,1094); assert(out[1]==0);
    llcSlowStep(c,in,out); assert(out[1]==2 && out[2]==135 && out[6]==1 && out[7]==3);
    in[6]=out[1];{ double seq=out[0]; ticks(c,in,out,100); assert(out[0]==seq); }
    in[14]=3; ticks(c,in,out,5); assert(out[7]==2 && out[1]==0 && out[6]==0);
    llcSlowReset(c); input(in); in[1]=32; in[3]=32; in[13]=3; in[14]=2; in[15]=1; in[16]=1;
    ticks(c,in,out,1044); assert(out[1]==0);
    llcSlowStep(c,in,out); assert(out[1]==3 && out[2]==6 && out[5]==2 && out[6]==2);
    llcSlowReset(c); input(in); in[1]=32; in[3]=32; in[13]=2; in[14]=2; in[15]=1; in[18]=.1;
    ticks(c,in,out,1100); assert(out[6]==1 && out[1]==2);
    llcSlowDestroy(c);
}
static void protection_and_reset(void) {
    LlcSlowContext *c=llcSlowCreate(&cfg); double in[20],out[17];
    input(in); in[5]=1; ticks(c,in,out,1995); assert(out[8]==0);
    llcSlowStep(c,in,out); assert(out[8]==1); in[5]=0; ticks(c,in,out,5); assert(out[8]==0);
    llcSlowReset(c); input(in); in[12]=13; ticks(c,in,out,2018); assert(out[9]==0);
    llcSlowStep(c,in,out); assert(out[9]==2048);
    in[12]=12; ticks(c,in,out,200); assert(out[9]==2048);
    llcSlowReset(c); input(in); in[5]=1; in[0]=3; ticks(c,in,out,2019); assert(out[9]==131072);
    llcSlowReset(c); input(in); in[19]=1; llcSlowStep(c,in,out); assert(out[15]==1);
    llcSlowStep(c,in,out); assert(out[15]==2); in[19]=0; llcSlowStep(c,in,out);
    in[19]=1; llcSlowStep(c,in,out); assert(out[15]==1);
    in[19]=0; in[18]=NAN; llcSlowStep(c,in,out); assert(out[9]!=0 && out[1]==0 && out[13]==0);
    llcSlowDestroy(c);
}
static void pfc_recovery(void) {
    LlcSlowContext *c=llcSlowCreate(&cfg);double in[20],out[17],seq;
    input(in);in[1]=32;in[3]=32;in[13]=1;in[14]=1;in[15]=1;
    ticks(c,in,out,1100);assert(out[1]==2);seq=out[0];
    in[10]=0;ticks(c,in,out,10);
    in[10]=1;ticks(c,in,out,210);
    assert(out[1]==2 && out[0]>seq);
    seq=out[0];in[6]=0; /* Fast captured a short PFC pulse between slow samples. */
    ticks(c,in,out,210);assert(out[1]==2 && out[0]>seq);
    llcSlowDestroy(c);
}
int main(void) { startup_and_instances(); cc_cv_requests(); protection_and_reset();pfc_recovery(); puts("slow tests passed"); return 0; }
