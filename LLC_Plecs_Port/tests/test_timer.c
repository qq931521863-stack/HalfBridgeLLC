#include "llc_timer.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
int main(void) {
    LlcTimerImage x;
    assert(llcBuildPrimaryImage(0,80000.0f,0.1f,&x));
    assert(x.pre==8500 && x.c[0]==1700 && x.c[1]==2550);
    assert(x.c[2]==5950 && x.c[3]==6800);
    assert(x.d[0]==0 && x.drv_h==0 && x.sr_enable==0);
    assert(llcBuildPrimaryImage(1,250000.0f,0.0f,&x));
    assert(x.pre==2720 && x.c[0]==120 && x.c[1]==1240);
    assert(x.c[2]==1480 && x.c[3]==2600);
    assert(llcBuildPrimaryImage(0,120000.0f,0.05f,&x));
    assert(x.pre==5664);
    assert(!llcBuildPrimaryImage(0,0,0.1f,&x) && x.pre==0);
    assert(!llcBuildPrimaryImage(0,80000,NAN,&x));
    assert(!llcBuildPrimaryImage(0,80000,0.51f,&x));
    assert(!llcBuildPrimaryImage(0,1,0.1f,&x));
    assert(!llcBuildPrimaryImage(2,80000,0.1f,&x));
    puts("timer: PASS (PWM windows, PFM deadtime, rounding, invalid input)");
    return 0;
}
