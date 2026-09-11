/* Host boundary and fast protection adapted from ConsoleFast.c/user_sample.c.
 * PWM/SR timer composition follows ConsoleSlow.c SHRTIMERdrive.
 * No HAL state and no process-global mutable controller state. */
#include "llc_context.h"
#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

LlcContext *llcCreate(const LlcConfig *cfg) {
    LlcContext *c;
    if(!cfg || cfg->abi_version!=1 || cfg->algorithm_profile!=1 ||
       cfg->fast_ts!=25e-6 || cfg->slow_ts!=1e-3 || cfg->timer_hz!=680e6) return NULL;
    c=(LlcContext*)calloc(1,sizeof(*c));
    if(c) { c->cfg=*cfg; llcReset(c); } return c;
}
void llcReset(LlcContext *c) {
    LlcConfig cfg;
    if(!c)return;
    cfg=c->cfg; memset(c,0,sizeof(*c)); c->cfg=cfg;
    c->lim_ref=.05f; c->driver.Plv=250000; llcPowerIniPidVar(c);
}
void llcDestroy(LlcContext *c) { free(c); }
void llcSampleStep(LlcContext *c,float vr,float vb,float ib) {
    if(!c)return;
    if(!isfinite(vr)||!isfinite(vb)||!isfinite(ib) ||
       fabsf(vr)>1e6f || fabsf(vb)>1e6f || fabsf(ib)>1e6f) {
        c->diagnostic_bits|=LLC_DIAG_INVALID_INPUT; return;
    }
    c->sample.v_relay=vr; c->sample.v_bat=vb; c->sample.i_bat=ib;
    c->sample.v_bat_fir=c->sample.v_bat_fir*.999f+vb*.001f;
    c->sample.i_bat_fir=c->sample.i_bat_fir*.999f+ib*.001f;
    llcSampleLpfHandle(c);
}
void llcReadSample(const LlcContext *c,LlcSample *s) { if(c&&s)*s=c->sample; }
void llcReadTrace(const LlcContext *c,LlcTrace *t) {
    if(!c||!t)return;
    memset(t,0,sizeof(*t)); t->mode=c->control.CtrMode; t->pre_ok=c->control.preOK;
    t->litate=c->control.litate; t->hold_phase=c->control.holdPhase;
    t->mode_ticks=c->control.time; t->fast_tick=c->fast_tick; t->last_command_seq=c->last_command_seq;
    t->current_ref=c->control.Curr_REF; t->current_limit=c->control.CurrentMax;
    t->v_pi_prev_error=c->v_pi.integral; t->v_pi_oldout=c->v_pi.oldout;
    t->i_pi_prev_error=c->i_pi.integral; t->i_pi_oldout=c->i_pi.oldout;
    t->pfm_integral=c->pfm_pi.integral; t->pfm_kp=c->pfm_pi.Kp; t->pfm_ki=c->pfm_pi.Ki;
    t->sr_atime=c->driver.Sr_Atime; t->sr_btime=c->driver.Sr_Btime; t->sr_dtime=c->driver.Sr_Dtime;
}
static int integer(double v,double max) { return isfinite(v)&&v>=0&&v<=max&&floor(v)==v; }
static int valid(const double *in) {
    int j;
    for(j=0;j<22;j++)if(!isfinite(in[j]))return 0;
    for(j=0;j<3;j++)if(fabs(in[j])>1e6)return 0;
    if(in[5]<0||in[5]>1e6||in[10]<0||in[10]>1e6||in[11]<0||in[11]>1e6)return 0;
    return integer(in[3],1)&&integer(in[4],4294967295.0)&&integer(in[6],2)&&
        integer(in[7],4294967295.0)&&(in[8]==-1||integer(in[8],8))&&integer(in[9],2047)&&
        integer(in[12],2)&&integer(in[13],2)&&integer(in[14],5)&&integer(in[15],1)&&
        integer(in[16],4294967295.0)&&integer(in[17],1)&&integer(in[18],7)&&
        integer(in[19],1)&&integer(in[20],1)&&integer(in[21],1);
}
static void command(LlcContext *c,const double *in) {
    uint32_t seq=(uint32_t)in[7],flags=(uint32_t)in[9],mask=(uint32_t)in[18];
    if(seq==0||seq==c->last_command_seq)return;
    if(in[8]>=0)c->control.CtrMode=(CtrMode_st)(int)in[8];
    if(flags&1)c->control.Curr_REF=0;
    if(flags&2)c->control.preOK=0;
    if(flags&4)c->control.litate=0;
    if(flags&8) { c->control.holdPhase=0; c->control.time=0; }
    if(flags&16)memset(&c->v_pi,0,sizeof(c->v_pi));
    if(flags&32)memset(&c->i_pi,0,sizeof(c->i_pi));
    if(flags&64)llcPowerIniPidVar(c);
    if(flags&128)c->i_pi.oldout=0;
    if(flags&256)c->v_pi.oldout=.01f;
    /* SysReset has no complete timer/iniOk context in this developer Fast API.
       Low AC (<30 V) is its explicit iniOk=0 condition; other cases are denied. */
    if(flags&512) {
        if(c->vac_rms_fir<30 && in[4]==0 && in[16]==0)c->fast_fault_bits=0;
        else c->diagnostic_bits|=LLC_DIAG_CLEAR_DENIED;
    }
    if(flags&1024)memset(&c->driver,0,sizeof(c->driver));
    if(mask&1)c->relay=(uint8_t)in[19];
    if(mask&2)c->discharge=(uint8_t)in[20];
    if(mask&4)c->llc_enable=(uint8_t)in[21];
    c->last_command_seq=seq;
}
static void protection(LlcContext *c,uint32_t hw) {
    float err,temp;
    c->fast_fault_bits|=hw;
    /* Source gates BOTH software OCP and short circuit on RelayOld. */
    if(c->relay_old) {
        if(c->sample.v_bat<10 && c->sample.i_bat>10)c->fast_fault_bits|=4096;
        if(c->sample.i_bat>42)c->fast_fault_bits|=512;
    }
    if(!c->ov_fault) {
        if(c->control.CtrMode==OvLoadPFM || c->control.CtrMode==ConCurPWM || c->control.CtrMode==PwmHold) {
            err=c->sample.i_bat;
            if(err>1)c->vtemp=c->sample.v_bat;
            if(c->control.Curr_REF>1) {
                temp=c->vtemp+3;
                if((c->sample.v_bat>=temp&&c->vtemp>30) ||
                   (c->control.Curr_REF>5&&c->sample.i_bat<1)) {
                    c->ov_fault=1; c->control.CtrMode=NoSelect; c->ov_time=0;
                    c->discharge=1; c->relay=0;
                }
            } else c->vtemp=0;
        } else c->vtemp=0;
    } else {
        c->vtemp=0;
        if(c->sample.v_bat<=58)c->ov_time++; else c->ov_time=0;
        if(c->ov_time>=140000) { c->ov_fault=0; c->ov_time=0; c->control.Curr_REF=0; }
    }
    if(c->sample.v_bat>=62)c->fast_fault_bits|=128;
}
static void timer_image(LlcContext *c,LlcTimerImage *t) {
    uint32_t half,quarter,bemp,duty,start,width; float time1;
    if(!llcBuildPrimaryImage(c->control.CtrMode==OvLoadPFM,c->driver.Plv,c->driver.Duty,t)) {
        c->diagnostic_bits|=LLC_DIAG_INVALID_TIMER; return;
    }
    half=t->pre/2; quarter=t->pre/4; bemp=t->pre-quarter;
    time1=(float)half*c->driver.Duty; duty=(uint32_t)time1;
    if(c->control.CtrMode==ConCurPWM||c->control.CtrMode==PwmHold) {
        if(c->driver.SynDrv&&c->driver.DrvH) {
            c->driver.Sr_Atime=200; time1=time1*2-c->driver.Sr_Atime;
            if(time1<150) { time1=0; c->driver.SynDrv=0; c->driver.Sr_Dtime=0; }
        } else { time1=0; c->driver.SynDrv=0; c->driver.Sr_Dtime=0; }
    } else { time1=0; c->driver.SynDrv=0; c->driver.Sr_Dtime=0; }
    c->driver.Sr_Dtime=c->driver.Sr_Dtime*.999f+time1*.001f;
    if(c->driver.Sr_Dtime>time1)c->driver.Sr_Dtime=time1;
    width=(uint32_t)c->driver.Sr_Dtime; c->driver.DrvDtime=(uint16_t)width;
    if(!c->driver.SynDrv) {
        t->d[0]=bemp-1; t->d[1]=bemp+1; t->d[2]=quarter-1; t->d[3]=quarter+1;
    } else {
        t->d[0]=(uint32_t)((float)(bemp-duty)+c->driver.Sr_Atime); t->d[1]=bemp+duty;
        start=(uint32_t)((float)(quarter-duty)+c->driver.Sr_Atime);
        t->d[2]=start; t->d[3]=start+width;
        if(t->d[0]>t->d[1]||t->d[1]>t->pre||t->d[2]>t->d[3]||t->d[3]>half) {
            c->diagnostic_bits|=LLC_DIAG_INVALID_TIMER; c->driver.SynDrv=0;
        }
    }
    t->drv_h=c->driver.DrvH; t->drv_l=c->driver.DrvL; t->sr_enable=c->driver.SynDrv;
    if(!c->driver.SynDrv)c->driver.Sr_Dtime=0;
}
void llcFastStep(LlcContext *c,const double in[22],double out[28]) {
    LlcTimerImage t={0}; int j,blocked;
    if(!out)return; memset(out,0,28*sizeof(*out)); if(!c)return;
    if(in && integer(in[17],1)) {
        if(in[17]&&!c->reset_high)llcReset(c);
        c->reset_high=(uint8_t)in[17];
    }
    c->fast_tick++;
    if(!in||!valid(in)) { c->diagnostic_bits|=LLC_DIAG_INVALID_INPUT; goto emit; }
    c->pfc_ok=(uint8_t)in[3]; c->vac_rms_fir=(float)in[5]; c->otp_level=(uint8_t)in[6];
    c->i_request=(float)in[10]; c->v_request=(float)in[11]; c->handshake=(uint8_t)in[12];
    c->cv_mode=(uint8_t)in[13]; c->run_state=(uint8_t)in[14]; c->relay_old=(uint8_t)in[15];
    c->slow_fault_bits=(uint32_t)in[16]; command(c,in);
    llcSampleStep(c,(float)in[0],(float)in[1],(float)in[2]);
    protection(c,(uint32_t)in[4]);
    if(c->fast_fault_bits||c->slow_fault_bits) {
        c->discharge=1; c->llc_enable=0; c->relay=0; c->control.Curr_REF=0; c->control.CtrMode=NoSelect;
    } else if(!c->pfc_ok) {
        c->discharge=1; c->llc_enable=1; c->control.CtrMode=NoSelect; c->control.Curr_REF=0;
    } else c->llc_enable=1;
    llcPowerCtrHandle(c);
    if(c->control.CtrMode==OvLoadPFM)c->diagnostic_bits|=LLC_DIAG_MISSING_PFM;
    /* Refuse PFM on the transition adoption frame as well as direct requests. */
    if(!(c->diagnostic_bits&LLC_DIAG_MISSING_PFM))timer_image(c,&t);
emit:
    if(c->diagnostic_bits&(LLC_DIAG_MISSING_PFM|LLC_DIAG_INVALID_INPUT|LLC_DIAG_INVALID_TIMER)) {
        c->llc_enable=0; c->relay=0; c->discharge=1;
    }
    blocked=c->fast_fault_bits||c->slow_fault_bits||c->ov_fault||!c->pfc_ok||
        (c->diagnostic_bits&(LLC_DIAG_MISSING_PFM|LLC_DIAG_INVALID_INPUT|LLC_DIAG_INVALID_TIMER));
    if(blocked) { t.drv_h=t.drv_l=t.sr_enable=0; c->driver.DrvH=c->driver.DrvL=c->driver.SynDrv=0; }
    out[0]=t.pre; for(j=0;j<4;j++){out[1+j]=t.c[j]; out[5+j]=t.d[j];}
    out[9]=t.drv_h; out[10]=t.drv_l; out[11]=t.sr_enable; out[12]=c->llc_enable;
    out[13]=c->relay; out[14]=c->discharge; out[15]=c->control.CtrMode; out[16]=c->control.preOK;
    out[17]=c->control.Curr_REF; out[18]=c->control.CurrentMax; out[19]=c->driver.Plv; out[20]=c->driver.Duty;
    out[21]=c->sample.v_bat_fir; out[22]=c->sample.i_bat_fir; out[23]=c->fast_fault_bits;
    out[24]=c->ov_fault; out[25]=c->last_command_seq; out[26]=c->fast_tick; out[27]=c->diagnostic_bits;
}
