#include "llc_gate_events.h"
#include <math.h>
#include <string.h>
#define LLC_GATE_HZ 680000000.0

void llcGateReset(LlcGate *g) { memset(g,0,sizeof(*g)); }
static int gateFrameValid(const double *u) {
    int k;
    for(k=0;k<15;k++) if(!isfinite(u[k]) || u[k]!=floor(u[k]) || u[k]<0) return 0;
    if(u[0]<4 || u[0]>65532 || fmod(u[0],4)!=0 || u[12]>4294967295.0) return 0;
    for(k=1;k<9;k++) if(u[k]>u[0]) return 0;
    for(k=1;k<9;k+=2) if(u[k]>u[k+1]) return 0;
    for(k=9;k<12;k++) if(u[k]>1) return 0;
    if(u[13]>1 || u[14]>1) return 0;
    /* Overlap is never a legal primary/SR frame. Empty intervals are allowed. */
    if(u[1]<u[4] && u[3]<u[2]) return 0;
    if(u[5]<u[8] && u[7]<u[6]) return 0;
    return 1;
}
int llcGateEvaluate(const LlcGate *old, double time, int capture,
                    const double u[15], LlcGate *g, double y[4], double *next)
{
    double tick, phase, edge, nearest;
    int k;
    *g=*old;
    for(k=0;k<4;k++) y[k]=0;
    *next=INFINITY;
    if(!isfinite(time) || time<0) { llcGateReset(g); return 0; }
    tick=time*LLC_GATE_HZ;
    /* Only snap numerical round-off around an integer, not arbitrary timestamps. */
    if(fabs(tick-floor(tick+.5))<1e-5) tick=floor(tick+.5);
    if(capture) {
        if(!gateFrameValid(u)) {
            /* Zero image is the deliberately disabled controller reset image. */
            if(u[0]==0 && u[9]==0 && u[10]==0 && u[11]==0 && u[13]==0) {
                llcGateReset(g); return 1;
            }
            llcGateReset(g); return 0;
        }
        if(u[14] && !g->reset_old) llcGateReset(g);
        g->reset_old=(int)u[14];
        if(!u[13]) { g->running=0; g->pending_valid=0; return 1; }
        if(!g->running || u[12]!=g->sequence) {
            g->sequence=u[12];
            g->mask[0]=(int)u[10]; g->mask[1]=(int)u[9];
            g->mask[2]=g->mask[3]=(int)u[11];
            if(!g->running) {
                memcpy(g->active,u,9*sizeof(double)); g->epoch=tick; g->running=1;
            } else { memcpy(g->pending,u,9*sizeof(double)); g->pending_valid=1; }
        }
    }
    if(!g->running) return 1;
    if(tick>=g->epoch+g->active[0]) {
        g->epoch+=g->active[0];
        if(g->pending_valid) { memcpy(g->active,g->pending,sizeof(g->active));g->pending_valid=0; }
        g->epoch+=floor((tick-g->epoch)/g->active[0])*g->active[0];
    }
    phase=tick-g->epoch;
    nearest=g->epoch+g->active[0];
    for(k=0;k<4;k++) {
        double on=g->active[1+2*k], off=g->active[2+2*k];
        y[k]=(g->mask[k] && phase>=on && phase<off) ? 1.0:0.0;
        if(g->mask[k] && on<off) {
            edge=g->epoch+on; if(edge>tick && edge<nearest) nearest=edge;
            edge=g->epoch+off; if(edge>tick && edge<nearest) nearest=edge;
        }
    }
    *next=nearest/LLC_GATE_HZ;
    if(*next<=time) { llcGateReset(g); memset(y,0,4*sizeof(double)); *next=INFINITY; return 0; }
    return 1;
}
