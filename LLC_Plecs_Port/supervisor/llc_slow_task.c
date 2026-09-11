/* Pure-C adaptation of operateStatus.c, ConsoleSlow.c and ProtectionLLC.c.
 * One call is one elapsed millisecond; the wrapper owns the t=0 convention.
 * Timer counters start in main.c BEFORE Delay1ms and application initialization.
 * No HAL/CAN transport, temperature policy, or absent BMS status is invented. */
#include "llc_slow.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

enum { S_INIT=0,S_WAKE=1,S_STANDBY=2,S_CHARGE=3,S_FULL=4,S_FAULT=5 };
#define SLOW_INVALID_INPUT UINT32_C(2147483648)
#define AUX_FAULT UINT32_C(2048)
#define RELAY_FAULT UINT32_C(131072)
#define CAN_FAULT UINT32_C(524288)
/* Exact fields selected by get_CAN_States, intentionally excluding Out_ov. */
#define APP_FAULT_MASK (UINT32_C(4)|UINT32_C(32)|UINT32_C(256)|UINT32_C(512)|UINT32_C(1024)|UINT32_C(2048)|UINT32_C(4096)|UINT32_C(131072)|UINT32_C(65536)|UINT32_C(524288)|UINT32_C(2097152)|UINT32_C(4194304)|UINT32_C(16384)|UINT32_C(8192)|UINT32_C(8388608))
typedef struct { uint16_t time,delay[20]; uint8_t index,full; } SlowProtect;
typedef struct { int mode; uint32_t flags,mask; uint8_t relay,discharge,llc; } SlowCommand;
struct LlcSlowContext {
    LlcConfig cfg;
    uint32_t tick,power_on,seq,slow_faults,fast_faults;
    uint16_t delay_ini,relay_count,wake_count,charge_count,error_count;
    uint8_t phase,uart_count,normal_voltage_count,ini_ok,timer_running,reset_high;
    uint8_t sys_state,run_state,old_state,handshake,enable,cv_mode,last_cv_mode;
    uint8_t relay_old,relay_applied,pfc_ok,ov_fault,online,raw_cv;
    float vr,vb,ib,vb_fir,ib_fir,vac,aux,i_request,v_request;
    SlowProtect aux_protect,relay_protect;
    SlowCommand last,pending;
};
static void begin_command(LlcSlowContext *c) { memset(&c->pending,0,sizeof(c->pending)); c->pending.mode=-1; }
static void mode(LlcSlowContext *c,int m) { c->pending.mode=m; }
static void init(LlcSlowContext *c,uint32_t f) { c->pending.flags|=f; }
static void relay(LlcSlowContext *c,int value) { c->pending.mask|=1; c->pending.relay=(uint8_t)value; c->relay_applied=(uint8_t)value; }
static void discharge(LlcSlowContext *c,int value) { c->pending.mask|=2; c->pending.discharge=(uint8_t)value; }
static void llc_enable(LlcSlowContext *c,int value) { c->pending.mask|=4; c->pending.llc=(uint8_t)value; }
static void commit_command(LlcSlowContext *c) {
    if(c->pending.mode!=-1 || c->pending.flags || c->pending.mask) {
        c->seq++; if(c->seq==0)c->seq=1; c->last=c->pending;
    }
}
LlcSlowContext *llcSlowCreate(const LlcConfig *cfg) {
    LlcSlowContext *c;
    if(!cfg||cfg->abi_version!=1||cfg->algorithm_profile!=1||cfg->fast_ts!=25e-6||cfg->slow_ts!=1e-3||cfg->timer_hz!=680e6)return NULL;
    c=(LlcSlowContext*)calloc(1,sizeof(*c)); if(c) { c->cfg=*cfg; llcSlowReset(c); } return c;
}
void llcSlowReset(LlcSlowContext *c) {
    LlcConfig cfg; if(!c)return; cfg=c->cfg; memset(c,0,sizeof(*c)); c->cfg=cfg;
    c->last.mode=-1; c->pending.mode=-1; c->timer_running=1;
}
void llcSlowDestroy(LlcSlowContext *c) { free(c); }
static int integer(double v,double high) { return isfinite(v)&&v>=0&&v<=high&&floor(v)==v; }
static int valid(const double *in) {
    int k; for(k=0;k<20;k++)if(!isfinite(in[k]))return 0;
    for(k=0;k<5;k++)if(fabs(in[k])>1e6)return 0;
    if(in[11]<0||in[11]>1e6||fabs(in[12])>1e6||in[17]<0||in[17]>1e6||in[18]<0||in[18]>1e6)return 0;
    return integer(in[5],1)&&integer(in[6],8)&&integer(in[7],2)&&integer(in[8],1)&&
        integer(in[9],4294967295.0)&&integer(in[10],1)&&integer(in[13],3)&&integer(in[14],3)&&
        integer(in[15],1)&&integer(in[16],1)&&integer(in[19],1);
}
static void normalize(LlcSlowContext *c,const double *in) {
    c->vr=(float)in[0]; c->vb=(float)in[1]; c->ib=(float)in[2]; c->vb_fir=(float)in[3]; c->ib_fir=(float)in[4];
    c->relay_applied=(uint8_t)in[5]; c->ov_fault=(uint8_t)in[8]; c->fast_faults=(uint32_t)in[9];
    c->pfc_ok=(uint8_t)in[10]; c->vac=(float)in[11]; c->aux=(float)in[12];
    /* ConsoleFast's PFC loss branch invalidates OldState. Reproduce that
     * cross-task action when the explicit PFC input is sampled by Slow. */
    if(!c->pfc_ok)c->old_state=S_INIT;
    /* A fast-captured PFC pulse can fall between slow samples. An established
     * active state with a NoSelect fast snapshot means its startup was lost.
     * Standby/Init legitimately use NoSelect and must not rearm this way. */
    if((c->run_state==S_WAKE||c->run_state==S_CHARGE) &&
       c->old_state==c->run_state && in[6]==NoSelect)c->old_state=S_INIT;
    c->handshake=(uint8_t)(in[13]==3?2:in[13]); c->online=(uint8_t)in[14];
    c->enable=(uint8_t)(in[15]!=0&&c->online!=3); c->raw_cv=(uint8_t)in[16];
    c->v_request=(float)in[17]; c->i_request=(float)in[18];
}
static void application(LlcSlowContext *c) {
    int fault=((c->fast_faults|c->slow_faults)&APP_FAULT_MASK)!=0;
    switch(c->sys_state) {
    case S_INIT:
        if(fault)c->sys_state=S_FAULT; else if(!c->handshake)c->sys_state=S_WAKE; else c->sys_state=S_STANDBY;
        break;
    case S_WAKE:
        if(fault)c->sys_state=S_FAULT; else if(c->handshake)c->sys_state=S_STANDBY;
        if(c->online==3)c->sys_state=S_STANDBY;
        break;
    case S_STANDBY:
        if(fault)c->sys_state=S_FAULT;
        else if((c->handshake==1||c->handshake==2)&&c->enable)c->sys_state=S_CHARGE;
        else if(!c->handshake)c->sys_state=S_WAKE;
        if(c->online==3)c->sys_state=S_STANDBY;
        break;
    case S_CHARGE:
        /* Source compares raw 0.1-unit fields with 1 and 30 respectively. */
        if(c->handshake==2) {
            if(c->raw_cv==0&&c->i_request>=.1f)c->cv_mode=1;
            else if(c->raw_cv==1&&c->v_request>=3.0f)c->cv_mode=2;
            else c->cv_mode=0;
        } else c->cv_mode=1;
        if(fault)c->sys_state=S_FAULT;
        else if((c->handshake==1||c->handshake==2)&&c->enable) { /* remain */ }
        else if((c->handshake==1||c->handshake==2)&&!c->enable)c->sys_state=S_STANDBY;
        else if(c->online==0) {
            if(c->vb_fir>30) { c->sys_state=S_FAULT; c->slow_faults|=CAN_FAULT; }
            else c->sys_state=S_WAKE;
        } else if(!c->handshake)c->sys_state=S_WAKE;
        if(c->online==3)c->sys_state=S_STANDBY;
        if(c->sys_state!=S_CHARGE)c->cv_mode=0;
        break;
    case S_FULL:
        if(fault)c->sys_state=S_FAULT;
        else if(!c->handshake||c->online==3)c->sys_state=S_STANDBY;
        break;
    case S_FAULT:
        if(c->handshake&&!fault)c->sys_state=S_STANDBY; else if(!fault)c->sys_state=S_WAKE;
        break;
    default: c->sys_state=S_INIT; break;
    }
    if(c->delay_ini>=100) {
        if(c->run_state!=c->sys_state)c->old_state=S_INIT;
        c->run_state=c->sys_state;
    } else c->run_state=S_INIT;
}
static void charging(LlcSlowContext *c) {
    int start;
    if(c->handshake==1)c->cv_mode=1;
    if(c->ov_fault)c->old_state=S_INIT;
    start=c->old_state!=c->run_state || (c->cv_mode!=c->last_cv_mode&&c->cv_mode);
    if(!start||c->ov_fault)return;
    mode(c,NoSelect);
    if(c->ib_fir>=1)return; /* Original counter is retained on this branch. */
    if(c->cv_mode==1) {
        if(c->vr<c->vb-2 && c->v_request>=30) {
            if(c->vb>=29&&c->vb<60) {
                discharge(c,0); relay(c,0); c->charge_count++;
                if(c->charge_count>=40) {
                    init(c,1|2|4|128); c->old_state=c->run_state; mode(c,BmsStar);
                    c->last_cv_mode=c->cv_mode; c->charge_count=0;
                    /* Volt_Ref=Vbat+2 is overwritten by SoftCurStart before use. */
                }
            } else { c->charge_count=0; discharge(c,1); relay(c,0); }
        } else { c->charge_count=0; discharge(c,1); relay(c,0); }
    } else if(c->cv_mode==2) {
        if(c->vr<c->v_request-2 && c->v_request>=30) {
            discharge(c,0); relay(c,0); c->charge_count++;
            if(c->charge_count>=30) {
                /* OvFault is already known zero from this handler's entry guard. */
                c->old_state=c->run_state; init(c,2|4); mode(c,SoftCurSt);
                c->last_cv_mode=c->cv_mode; c->charge_count=0;
            }
        } else { c->charge_count=0; discharge(c,1); relay(c,0); }
    }
}
static void supervisor(LlcSlowContext *c) {
    switch(c->run_state) {
    case S_INIT:
        init(c,1024|64); mode(c,NoSelect); c->old_state=c->run_state; discharge(c,1);
        if(c->pfc_ok) { if(c->delay_ini<100)c->delay_ini++; } else c->delay_ini=0;
        break;
    case S_WAKE:
        if(c->delay_ini<100)break;
        if(c->old_state!=c->run_state) {
            if(c->vr<28&&c->vb<5&&!c->relay_applied) {
                c->wake_count++;
                if(c->wake_count>=3) {
                    c->old_state=c->run_state; discharge(c,0); relay(c,0);
                    init(c,2|16); mode(c,SoftStar); c->wake_count=0;
                }
            } else { c->wake_count=0; discharge(c,1); relay(c,0); }
        } else c->wake_count=0;
        break;
    case S_STANDBY:
    case S_FULL:
        if(c->delay_ini>=100&&c->old_state!=c->run_state) {
            c->old_state=c->run_state; discharge(c,1); relay(c,0); mode(c,NoSelect);
        }
        break;
    case S_CHARGE:
        if(c->delay_ini>=100)charging(c);
        break;
    case S_FAULT:
        discharge(c,1); llc_enable(c,0); relay(c,0); mode(c,NoSelect);
        if(!(c->fast_faults|c->slow_faults)) {
            c->error_count++;
            if(c->error_count>=200) { c->run_state=S_INIT; c->error_count=0; }
        } else c->error_count=0;
        break;
    default: break;
    }
}
static void relay_age(LlcSlowContext *c) {
    if(c->relay_applied) {
        if(c->relay_count<400)c->relay_count++;
        if(c->relay_count>=400)c->relay_old=1;
    } else { c->relay_count=0; c->relay_old=0; }
}
static void sys_reset(LlcSlowContext *c) {
    if(c->vac<30)c->ini_ok=0;
    if(c->vac>60) {
        if(c->normal_voltage_count<4)c->normal_voltage_count++;
        else if(!c->ini_ok) { c->timer_running=1; c->ini_ok=1; }
    } else c->normal_voltage_count=0;
    if(!c->ini_ok) {
        /* Cross-DLL Fast fault clear needs additional ownership/permission data.
         * Only Slow-owned faults are cleared here; no bit-9 transaction is sent. */
        c->slow_faults&=SLOW_INVALID_INPUT; c->delay_ini=0; c->run_state=S_INIT;
    }
}
static void protect(LlcSlowContext *c,SlowProtect *p,uint32_t bit,int auxiliary,float value) {
    uint32_t sum=0; int j,bad;
    if(c->power_on<2000)return;
    if(p->time<20000)p->time++;
    if(c->slow_faults&bit)return; /* Source entries 3/5 both ClearEnable=0. */
    if(!auxiliary&&!c->relay_old)return;
    bad=auxiliary?(value>12.8f||value<11.2f):(value>2||value< -2);
    if(!bad)return;
    p->delay[p->index]=p->time; p->time=0; p->index++;
    if(p->index>=20) { p->index=0; p->full=1; }
    if(p->full) {
        for(j=0;j<20;j++)sum+=p->delay[j];
        if(sum<1000) { memset(p->delay,0,sizeof(p->delay)); p->index=0; p->full=0; c->slow_faults|=bit; }
    }
}
static void emit(const LlcSlowContext *c,double *out) {
    out[0]=c->seq; out[1]=c->last.mode; out[2]=c->last.flags;
    out[3]=c->i_request; out[4]=c->v_request; out[5]=c->handshake; out[6]=c->cv_mode;
    out[7]=c->run_state; out[8]=c->relay_old; out[9]=c->slow_faults; out[10]=c->last.mask;
    out[11]=c->last.relay; out[12]=c->last.discharge; out[13]=c->last.llc;
    out[14]=c->power_on; out[15]=c->tick; out[16]=c->timer_running;
}
void llcSlowStep(LlcSlowContext *c,const double in[20],double out[17]) {
    if(!out)return; memset(out,0,17*sizeof(*out)); if(!c)return;
    if(in&&integer(in[19],1)) {
        if(in[19]&&!c->reset_high)llcSlowReset(c);
        c->reset_high=(uint8_t)in[19];
    }
    c->tick++; if(c->power_on<2000)c->power_on++;
    begin_command(c);
    if(!in||!valid(in))c->slow_faults|=SLOW_INVALID_INPUT;
    if(c->slow_faults&SLOW_INVALID_INPUT) {
        c->timer_running=0; c->run_state=S_FAULT; mode(c,NoSelect);
        relay(c,0); discharge(c,1); llc_enable(c,0); commit_command(c); emit(c,out); return;
    }
    normalize(c,in);
    c->phase++;
    if(c->phase>=5) { c->phase=0; application(c); supervisor(c); }
    else if(c->phase==1)relay_age(c);
    else if(c->phase==3) {
        c->uart_count++;
        if(c->uart_count>=20) { c->uart_count=0; sys_reset(c); }
    }
    protect(c,&c->relay_protect,RELAY_FAULT,0,c->vr-c->vb);
    protect(c,&c->aux_protect,AUX_FAULT,1,c->aux);
    commit_command(c); emit(c,out);
}
