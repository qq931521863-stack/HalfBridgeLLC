#include "../model/gate/llc_gate_events.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#define HZ 680000000.0
int main(void) {
    LlcGate g,a,b;
    double y[4],z[4],n,m;
    double u[15]={8500,1700,2550,5950,6800,0,0,0,0,1,1,0,1,1,0};
    llcGateReset(&g);
    assert(llcGateEvaluate(&g,0,1,u,&a,y,&n));
    assert(n==1700/HZ && !y[0] && !y[1]);
    assert(!memcmp(&g,&(LlcGate){0},sizeof(g)));
    assert(llcGateEvaluate(&g,0,1,u,&b,z,&m));
    assert(!memcmp(&a,&b,sizeof(a)) && n==m);
    g=a;
    assert(llcGateEvaluate(&g,1700/HZ,0,u,&a,y,&n) && y[0]==1 && y[1]==0);
    g=a;
    assert(llcGateEvaluate(&g,2550/HZ,0,u,&a,y,&n) && y[0]==0);
    g=a;
    /* New comparisons pending; immediate mask off while old period survives. */
    u[0]=2720; u[1]=120;u[2]=1240;u[3]=1480;u[4]=2600;u[9]=0;u[12]=2;
    assert(llcGateEvaluate(&g,4000/HZ,1,u,&a,y,&n));
    assert(a.active[0]==8500 && a.pending_valid && a.mask[1]==0);
    g=a;
    assert(llcGateEvaluate(&g,8500/HZ,0,u,&a,y,&n));
    assert(a.active[0]==2720 && a.epoch==8500 && n>8500/HZ);
    g=a;
    u[13]=0;u[12]=3;
    assert(llcGateEvaluate(&g,9000/HZ,1,u,&a,y,&n));
    assert(!a.running && y[0]==0 && y[1]==0 && isinf(n));
    u[13]=1;u[0]=NAN;
    assert(!llcGateEvaluate(&g,9000/HZ,1,u,&a,y,&n));
    assert(!y[0]&&!y[1]&&!y[2]&&!y[3]);
    /* Reset discards both an old period and pending comparisons. Holding the
     * input high must not restart the phase at every subsequent fast sample. */
    u[0]=2720;u[9]=1;u[13]=1;u[14]=1;u[12]=1;
    assert(llcGateEvaluate(&g,10000/HZ,1,u,&a,y,&n));
    assert(a.epoch==10000 && a.active[0]==2720 && !a.pending_valid);
    g=a;
    assert(llcGateEvaluate(&g,10120/HZ,1,u,&a,y,&n) && y[0]==1);
    assert(a.epoch==10000 && a.reset_old==1);
    g=a;u[14]=0;
    assert(llcGateEvaluate(&g,10500/HZ,1,u,&a,y,&n));
    g=a;u[14]=1;
    assert(llcGateEvaluate(&g,11000/HZ,1,u,&a,y,&n));
    assert(a.epoch==11000 && !y[0] && !y[1]);
    puts("gate: PASS (exact edges, pure evaluation, pending commit, mask, stop, NaN, reset edges)");
    return 0;
}
