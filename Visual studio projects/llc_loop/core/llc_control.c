/* Adapted from AppUser/mathR02.c; V/A requests standardized; PFM explicitly unavailable. */
#include "llc_context.h"
#include <string.h>
extern const filter_str FilterOne,FilterTwo;
void RequestMode(LlcContext *ctx, CtrMode_st next);
void llcSampleLpfHandle(LlcContext *ctx);
static void DriveSet(LlcContext *ctx, uint8_t h, uint8_t l, uint8_t syn, float plv, float duty);
static void UpdateCurrentMaxFromCan(LlcContext *ctx);
void RefRampPwmCurr(LlcContext *ctx);
void RefRampPfmCurr(LlcContext *ctx);
static void PwmSyncDrvUpdate(LlcContext *ctx);
void ConIdleHandle(LlcContext *ctx);
void ConPwmHoldHandle(LlcContext *ctx);
void ConTransHandle(LlcContext *ctx);
void ConPfmHandle(LlcContext *ctx);
void llcPowerCtrHandle(LlcContext *ctx);
void llcPowerIniPidVar(LlcContext *ctx);
void SoftStart(LlcContext *ctx);
void ConVoltHandle(LlcContext *ctx);
void ConCurrHandle(LlcContext *ctx);
void SoftCurStart(LlcContext *ctx);
void ConVoltHandleTwo(LlcContext *ctx);
void ConVoltCurHandle(LlcContext *ctx);
/**
 * @brief Handle 请求切换快环模式，真正改 CtrMode 只在 llcPowerCtrHandle 末尾
 */
void RequestMode(LlcContext *ctx, CtrMode_st next)
{
	if(next > PwmHold)
	{
		return;
	}
	ctx->control.reqMode = next;
}

/**
 * @brief 功率电压/电流统一一阶低通后再进环（FilterOne，约 4 kHz）
 * @note  慢 FIR（*_FIR）仍在 ADC0_Sample，供 CAN/切模式；保护仍用 *_adc
 */
void llcSampleLpfHandle(LlcContext *ctx)
{
	/* Persistent state belongs to ctx. */
	if(ctx->lpf_initialized == 0)
	{
		memcpy((uint8_t*)&ctx->filter_rly,(uint8_t*)&FilterOne,sizeof(filter_str));
		memcpy((uint8_t*)&ctx->filter_v,(uint8_t*)&FilterOne,sizeof(filter_str));
		memcpy((uint8_t*)&ctx->filter_i,(uint8_t*)&FilterOne,sizeof(filter_str));
		ctx->lpf_initialized = 1;
	}
	ctx->sample.v_relay_lpf = OneOrderForm(ctx->sample.v_relay, &ctx->filter_rly);
	ctx->sample.v_bat_lpf = OneOrderForm(ctx->sample.v_bat, &ctx->filter_v);
	ctx->sample.i_bat_lpf = OneOrderForm(ctx->sample.i_bat, &ctx->filter_i);
}

/**
 * @brief 写驱动指令。plv<=0 或 duty 越 [0,1] 时钳位
 */
static void DriveSet(LlcContext *ctx, uint8_t h, uint8_t l, uint8_t syn, float plv, float duty)
{
	if(h > 1u) h = 1u;
	if(l > 1u) l = 1u;
	if(syn > 1u) syn = 1u;
	if(plv < 1.0f) plv = POW_MAX_PLV;
	if(duty < 0.0f) duty = 0.0f;
	else if(duty > 1.0f) duty = 1.0f;

	ctx->driver.DrvH = h;
	ctx->driver.DrvL = l;
	ctx->driver.SynDrv = syn;
	ctx->driver.Plv = plv;
	ctx->driver.Duty = duty;
}

/** 从 CAN 更新 CurrentMax；非 BMS/PMS 握手时保持原值 */
static void UpdateCurrentMaxFromCan(LlcContext *ctx)
{
	if(ctx->handshake == 1)
	{
		ctx->control.CurrentMax = ctx->i_request;
	}
	else if(ctx->handshake == 2)
	{
		ctx->control.CurrentMax = ctx->i_request;
	}
}

/**
 * @brief 指令 ±step 逼近 target，到达 step 内则贴死。无跟踪窗、无限幅
 */
float RampToward(float ref, float target, float step)
{
	if(ref < target - step) return ref + step;
	else if(ref > target + step) return ref - step;
	else return target;
}

/**
 * @brief PWM 电流缓变：钳 10A，仅在实测电流 ±1A 窗内爬坡
 * @note  使能门槛 CurrentMax>0.1A 
 */
void RefRampPwmCurr(LlcContext *ctx)
{
	if(ctx->control.CurrentMax > PWM_CURR_MAX_CLAMP)
	{
		ctx->control.CurrentMax = PWM_CURR_MAX_CLAMP;
	}

	if(ctx->control.Curr_REF < ctx->control.CurrentMax)
	{
		if(ctx->control.Curr_REF > ctx->sample.i_bat_lpf - PWM_CURR_TRACK_BAND)
		{
			ctx->control.Curr_REF = RampToward(ctx->control.Curr_REF,
				ctx->control.CurrentMax, REF_RAMP_STEP);
		}
	}else
	{
		if(ctx->control.Curr_REF < ctx->sample.i_bat_lpf + PWM_CURR_TRACK_BAND)
		{
			ctx->control.Curr_REF = RampToward(ctx->control.Curr_REF,
				ctx->control.CurrentMax, REF_RAMP_STEP);
		}
	}
}

/**
 * @brief PFM 电流缓变：功率 P/V、CON_CURR_OUT、OTP 后再直追 CurrentMax
 * @note  使能门槛 CurrentMax>0.5A 仍在 ConPfmHandle；无 ±1A 跟踪窗
 */
void RefRampPfmCurr(LlcContext *ctx)
{
	float err, out = 0;
	float vBatFir;

	err = ctx->vac_rms_fir * 7.2f;
	if(err > 1600.0f)err = 1600.0f;
	vBatFir = ctx->sample.v_bat_fir;
	if(vBatFir > PFM_VBAT_DIV_MIN)
	{
		out = err / vBatFir;
		if(ctx->control.CurrentMax > out)ctx->control.CurrentMax = out;
	}
	if(ctx->control.CurrentMax > CON_CURR_OUT)ctx->control.CurrentMax = CON_CURR_OUT;
	if(ctx->otp_level == 2)
	{
		ctx->control.CurrentMax *= 0.5f;
	}else if(ctx->otp_level == 1)
	{
		ctx->control.CurrentMax *= 0.75f;
	}

	ctx->control.Curr_REF = RampToward(ctx->control.Curr_REF,
		ctx->control.CurrentMax, REF_RAMP_STEP);
}

/**
 * @brief PWM 同步管按输出电流开通（原 SHRTIMERdrive 的 ctx->delay_sr）
 *        >4A 连续 1000 拍开通；已开通后 <2A 关闭
 */
static void PwmSyncDrvUpdate(LlcContext *ctx)
{
	/* Persistent state belongs to ctx. */

	if(ctx->driver.SynDrv == 0)
	{
		if(ctx->sample.i_bat > 4.0f)
		{
			ctx->delay_sr++;
			if(ctx->delay_sr >= 1000)
			{
				ctx->delay_sr = 1000;
				ctx->driver.SynDrv = 1;
			}
		}else ctx->delay_sr = 0;
	}else
	{
		ctx->delay_sr = 0;
		if(ctx->sample.i_bat < 2.0f)ctx->driver.SynDrv = 0;
	}
}

/** 空闲关波。故障/PFC 未就绪时放电与继电器已由 HandleFast 配好 */
void ConIdleHandle(LlcContext *ctx)
{
	if(((ctx->fast_fault_bits | ctx->slow_fault_bits) == 0) && (ctx->pfc_ok != 0))
	{
		(ctx->discharge = 0);
		(ctx->relay = 0);
	}
	llcPowerIniPidVar(ctx);
	ctx->control.preOK = 0;
	DriveSet(ctx, 0, 0, 0, POW_MAX_PLV, 0.02f);
	ctx->control.time = 0;
}

/**
 * @brief PWM→PFM 前保持：phase1 降占空比等电流；phase2 切 Transition
 */
void ConPwmHoldHandle(LlcContext *ctx)
{
	if(ctx->control.holdPhase == 2)
	{
		ctx->control.time = 0;
		ctx->control.Curr_REF = 0;
		RequestMode(ctx, Transition);
		ctx->control.flvOut = POW_MAX_PLV;
		ctx->pfm_pi.integral = PID_MAX_PLV;
		ctx->pfm_pi.fir = 0;
		DriveSet(ctx, 0, 1, 0, ctx->control.flvOut, 0.05f);
	}else
	{
		DriveSet(ctx, 0, 1, 0, 80000.0f, 0.04f);
		if(ctx->sample.i_bat_fir < 1.0f)
		{
			ctx->control.time++;
			if(ctx->control.time >= 40)
			{
				ctx->control.holdPhase = 2;
				ctx->control.time = 0;
			}
		}
	}
	PwmSyncDrvUpdate(ctx);
}

/**
 * @brief PWM→PFM 过渡：有电流请求则 Duty 斜坡 400 拍后进 PFM
 */
void ConTransHandle(LlcContext *ctx)
{
	UpdateCurrentMaxFromCan(ctx);
	if(ctx->control.CurrentMax > 0.5f)
	{
		ctx->control.time++;
		if(ctx->control.time >= 400)
		{
			ctx->control.flvOut = POW_MAX_PLV;
			ctx->pfm_pi.integral = PID_MAX_PLV;
			ctx->pfm_pi.fir = 0;
			ctx->control.time = 0;
			RequestMode(ctx, OvLoadPFM);
		}
		ctx->control.Curr_REF = 0;
		DriveSet(ctx, 1, 1, 0, POW_MAX_PLV, ctx->control.time * 0.001f + 0.05f);
	}else
	{
		ctx->control.flvOut = POW_MAX_PLV;
		ctx->pfm_pi.integral = PID_MAX_PLV;
		ctx->pfm_pi.fir = 0;
		ctx->control.Curr_REF = 0;
		DriveSet(ctx, 0, 0, 0, POW_MAX_PLV, ctx->driver.Duty);
	}
}

/**
 * @brief PFM 电流环
 */
void ConPfmHandle(LlcContext *ctx)
{
    ctx->diagnostic_bits |= LLC_DIAG_MISSING_PFM;
    DriveSet(ctx, 0,0,0,POW_MAX_PLV,0.0f);
}

/**
 * @brief 快环状态机：只调 Handle，再采纳 reqMode
 */
void llcPowerCtrHandle(LlcContext *ctx)
{
	ctx->control.reqMode = ctx->control.CtrMode;

	switch(ctx->control.CtrMode)
	{
		case  NoSelect :
			ConIdleHandle(ctx);
		break;
		case  SoftStar :
			SoftStart(ctx);
		break;
		case BmsStar :
			SoftCurStart(ctx);
		break;
		case SoftCurSt:
			ConVoltCurHandle(ctx);
			ctx->control.time = 0;
		break;
		case ConVolt :
			ConVoltHandle(ctx);
		break;
		case  ConCurPWM :
			ConCurrHandle(ctx);
		break;
		case  PwmHold :
			ConPwmHoldHandle(ctx);
		break;
		case  Transition:
			ConTransHandle(ctx);
		break;
		case  OvLoadPFM :
			ConPfmHandle(ctx);
		break;
		default:
		break;
	}

	if(ctx->control.reqMode != ctx->control.CtrMode)
	{
		if(ctx->control.reqMode <= PwmHold)
		{
			ctx->control.CtrMode = ctx->control.reqMode;
		}
	}
}

/** 复位 PFM PI 与进环滤波器状态 */
void llcPowerIniPidVar(LlcContext *ctx)
{
	memset((uint8_t*)&ctx->pfm_pi, 0, sizeof(pi_str));
	memcpy((uint8_t*)&ctx->filter_two,(uint8_t*)&FilterTwo,sizeof(filter_str));
	memcpy((uint8_t*)&ctx->filter_one,(uint8_t*)&FilterOne,sizeof(filter_str));
	memcpy((uint8_t*)&ctx->filter_rly,(uint8_t*)&FilterOne,sizeof(filter_str));
	memcpy((uint8_t*)&ctx->filter_v,(uint8_t*)&FilterOne,sizeof(filter_str));
	memcpy((uint8_t*)&ctx->filter_i,(uint8_t*)&FilterOne,sizeof(filter_str));
}

/** 增量核薄封装：读结构体内 Kp/Ki */
float piExter_Compute(float input,pi_str* p)
{
	if(p == 0) return 0.0f;
	return PiPwmCompute(p, input, p->Kp, p->Ki);
}

/** PFM 位置式 PI：integral += Ki*e ; y = integral + Kp*e */
float Compensator_Pid(pi_str* p, float err)
{
	if(p == 0) return 0.0f;
	p->err = err;
	p->integral += p->Ki * p->err;
	p->output = p->integral + p->Kp * p->err;
	return p->output;
}

/** PWM 增量核：Δu = Kp*(e-e_prev)+Ki*e，integral 存上一拍误差。各环 Kp/Ki 自备 */
float PiPwmCompute(pi_str *p, float err, float kp, float ki)
{
	if(p == 0) return 0.0f;
	p->err = err;
	p->output = kp * (p->err - p->integral) + ki * p->err;
	p->integral = p->err;
	return p->output;
}


float oneOrderForm(float input, filter_str *p)
{
	return OneOrderForm(input, p);
}

/** 一阶 IIR：y = B0*x + w1；w1 = B1*x - A1*y。p 为空则直通 */
float OneOrderForm(float input, filter_str *p)
{
	float vr;
	if(p == 0) return input;
	vr = (input * p->coeff_B0) + p->filter_W1;
	p->filter_W1 = (input * p->coeff_B1) - (vr * p->coeff_A1);
	return vr;
}

/** 二阶 IIR。p 为空则直通 */
float SecondOrderForm(float input, filter_str *p)
{
	float vr;
	if(p == 0) return input;
	vr = (input * p->coeff_B0) + p->filter_W1;
	p->filter_W1 = (input * p->coeff_B1) + p->filter_W2 - (vr * p->coeff_A1);
	p->filter_W2 = (input * p->coeff_B2) - (vr * p->coeff_A2);
	return vr;
}
/* FilterOne：约 4 kHz 一阶；FilterTwo：二阶（预留） */
const filter_str FilterOne = {0.2391f,0.2391f,0,-0.5219f,0,0,0};
const filter_str FilterTwo = {7.4499f,-9.2174f,2.8510f,0.1758f,-0.0922f,0,0};

const pi_str pwmExpi = {0,0,0,0,0,0};

/* Persistent state belongs to ctx. */

/** 无握手唤醒：开环抬继电器电压到 30V 后切恒压 */
void SoftStart(LlcContext *ctx)
{
	/* Persistent state belongs to ctx. */
	float vRly;

	vRly = ctx->sample.v_relay_lpf;

	ctx->control.Volt_Ref = 30.0f;
	if(ctx->relay == 0)
	{
		if(vRly < ctx->control.Volt_Ref)
		{
			(ctx->relay = 0);
			(ctx->discharge = 0);
			ctx->control.preOK = 0;
			ctx->driver.Plv = 120000.0f;
			ctx->driver.DrvH = 1;
			ctx->driver.DrvL = 1;
			ctx->driver.SynDrv = 0;
			ctx->driver.Duty = 0.035f;
			if(vRly > 25.0f)ctx->driver.Duty = 0.05f;
			else if(vRly > 20.0f)ctx->driver.Duty = 0.047f;
			else if(vRly > 15.0f)ctx->driver.Duty = 0.044f;
			else if(vRly > 10.0f) ctx->driver.Duty = 0.04f;
		}else
		{
			(ctx->relay = 1);
			ctx->soft_pre_num = 0;
		}
	}else
	{
		ctx->soft_pre_num++;
		if(ctx->soft_pre_num > 1000)
		{
			RequestMode(ctx, ConVolt);
			ctx->soft_pre_num = 0;
			ctx->star_time = 0;
		}
		ctx->control.preOK = 1;
		ctx->driver.Plv = POW_MAX_PLV;
		ctx->driver.DrvH = 0;
		ctx->driver.DrvL = 0;
		ctx->driver.SynDrv = 0;
		ctx->driver.Duty = 0.01f;
		ctx->v_pi.oldout = 0.01f;
	}
}

/** PWM 恒压 30V，电流限制 5.4A，双环取小后累加占空比 */
void ConVoltHandle(LlcContext *ctx)
{
	float temp, vRly, outV, outI;

	ctx->control.Volt_Ref = 30.0f;
	vRly = ctx->sample.v_relay_lpf;
	if(vRly < ctx->control.Volt_Ref)temp = vRly + 0.01f;
	else temp = ctx->control.Volt_Ref;

	outV = PiPwmCompute(&ctx->v_pi, temp - vRly, PWM_V_KP, PWM_V_KI);
	outI = PiPwmCompute(&ctx->i_pi, 5.4f - ctx->sample.i_bat_lpf, PWM_I_LIM_KP, PWM_I_LIM_KI);
	if(outV > outI)outV = outI;
	outV += ctx->v_pi.oldout;
	if(outV > 0.4f)outV = 0.4f;
	ctx->v_pi.oldout = outV;
	ctx->v_pi.output = outV;

	(ctx->discharge = 1);
	DriveSet(ctx, (outV < 0.02f) ? 0 : 1, 1, 0, 120000.0f, outV);
}

/** PWM 恒流；电池电压≥40V 请求进入 PwmHold */
void ConCurrHandle(LlcContext *ctx)
{
	if(ctx->sample.v_bat >= 40.0f)
	{
		ctx->control.holdPhase = 1;
		ctx->control.time = 0;
		ctx->control.Curr_REF = 0;
		RequestMode(ctx, PwmHold);
		DriveSet(ctx, 0, 1, 0, 80000.0f, 0.02f);
		PwmSyncDrvUpdate(ctx);
		return;
	}

	if(ctx->handshake == 0)
	{
		ctx->driver.DrvH = 0;
		ctx->driver.DrvL = 0;
		ctx->driver.SynDrv = 0;
	}else
	{
		UpdateCurrentMaxFromCan(ctx);
		if(ctx->control.CurrentMax > 0.1f)
		{
			RefRampPwmCurr(ctx);

			ctx->i_pi.err = ctx->control.Curr_REF - ctx->sample.i_bat_lpf;
			if(ctx->i_pi.err > 0.2f)ctx->i_pi.err = 0.2f;
			else if(ctx->i_pi.err < -0.2f)ctx->i_pi.err = -0.2f;
			ctx->i_pi.output = PiPwmCompute(&ctx->i_pi, ctx->i_pi.err, PWM_CUR_KP, PWM_CUR_KI);
			ctx->test_duty = ctx->i_pi.oldout;
			ctx->i_pi.output = ctx->i_pi.oldout + ctx->i_pi.output;

			if(ctx->i_pi.output > 0.4f)ctx->i_pi.output = 0.4f;
			if(ctx->i_pi.output < 0.02f)
			{
				ctx->i_pi.output = 0.02f;
				ctx->i_pi.oldout = ctx->i_pi.output;
				DriveSet(ctx, 0, 1, 0, 80000.0f, ctx->i_pi.output);
			}
			else
			{
				ctx->i_pi.oldout = ctx->i_pi.output;
				DriveSet(ctx, 1, 1, 0, 80000.0f, ctx->i_pi.output);
			}
		}else
		{
			ctx->driver.DrvH = 0;
			ctx->driver.DrvL = 0;
			ctx->driver.SynDrv = 0;
			ctx->i_pi.output = 0;
			ctx->i_pi.oldout = 0;
			ctx->driver.Duty = 0;
			ctx->driver.Sr_Dtime = 0;
			ctx->control.Curr_REF = 0;
		}
	}
	PwmSyncDrvUpdate(ctx);
}

/** 有握手预充：抬压吸合继电器后，按电池电压进 PWM 或直接 Transition */
void SoftCurStart(LlcContext *ctx)
{
	/* Persistent state belongs to ctx. */
	float vRly,vBat = 0;
	vBat = ctx->sample.v_bat;
	vRly = ctx->sample.v_relay_lpf;
	if(ctx->control.preOK == 0)
	{
		if(ctx->control.litate == 0)
		{
			(ctx->discharge = 0);
			(ctx->relay = 0);
			ctx->control.preOK = 0;
			ctx->precharge_time = 0;
			ctx->driver.Plv = 40000.0f;
			ctx->driver.DrvH = 1;
			ctx->driver.DrvL = 1;
			ctx->driver.SynDrv = 0;

			if(vRly < 10.0f)ctx->driver.Duty = 0.01f;
			else if(vRly < 20.0f)ctx->driver.Duty = 0.02f;
			else if(vRly < 30.0f)ctx->driver.Duty = 0.03f;
			else if(vRly < 40.0f)ctx->driver.Duty = 0.04f;
			else if(vRly < 50.0f)ctx->driver.Duty = 0.06f;
			else ctx->driver.Duty = 0.07f;
			if(vRly > vBat)ctx->driver.Duty = 0.05f;
			if(vRly >= vBat + 2.0f)
			{
				ctx->control.litate = 1;
				ctx->precharge_time = 0;
			}
		}else if(ctx->control.litate == 1)
		{
			(ctx->discharge = 1);
			(ctx->relay = 0);
			if(vRly < vBat + 0.5f)
			{
				ctx->control.litate = 0;
				ctx->precharge_time = 0;
			}else
			{
				ctx->precharge_time++;
				if(ctx->precharge_time >= 20)
				{
					ctx->control.preOK = 1;
					ctx->precharge_time = 0;
				}
			}
			ctx->driver.Plv = POW_MAX_PLV;
			ctx->driver.DrvH = 0;
			ctx->driver.DrvL = 0;
			ctx->driver.SynDrv = 0;
			ctx->driver.Duty = 0.02f;
		}
	}else if(ctx->control.preOK == 1)
	{
		if(vRly < vBat + 0.2f)
		{
			ctx->precharge_time++;
			if(ctx->precharge_time >= 20)
			{
				ctx->control.preOK = 2;
				ctx->precharge_time = 0;
				ctx->precharge_time2 = 0;
				(ctx->relay = 1);
			}
			(ctx->discharge = 0);
			ctx->driver.Plv = POW_MAX_PLV;
			ctx->driver.DrvH = 0;
			ctx->driver.DrvL = 0;
			ctx->driver.SynDrv = 0;
			ctx->driver.Duty = 0.02f;
		}
	}
	else if(ctx->relay == 1)
	{
		if(vBat <  40.0f)
		{
			ctx->precharge_time++;
			if(ctx->precharge_time >= 20000)
			{
				ctx->i_pi.integral = 0;
				ctx->i_pi.oldout = 0;
				ctx->precharge_time = 0;
				ctx->i_pi.oldout = 0;
				ctx->control.Curr_REF = 0;
				RequestMode(ctx, ConCurPWM);
			}
		}else if(vBat > 40.5f) ctx->precharge_time = 0;
		if(vBat >=  40.0f)
		{
			ctx->precharge_time2++;
			if(ctx->precharge_time2 >= 20000)
			{
				RequestMode(ctx, Transition);
				ctx->control.Curr_REF = 0;
				ctx->precharge_time2 = 0;
			}
		}else if(vBat <  39.5f) ctx->precharge_time2 = 0;

		(ctx->discharge = 0);
		ctx->driver.Plv = POW_MAX_PLV;
		ctx->driver.DrvH = 0;
		ctx->driver.DrvL = 0;
		ctx->driver.SynDrv = 0;
		ctx->driver.Duty = 0.02f;
	}
}

/** CV 恒压（CAN 30~60V），占空比按电压/电流限幅 */
void ConVoltHandleTwo(LlcContext *ctx)
{
	float temp,vRly,limit = 0;
	/* Persistent state belongs to ctx. */
	vRly = ctx->sample.v_relay_lpf;
	temp = ctx->v_request;

	if((temp >= 30.0f)&&(temp <= 60.0f))
	{
		if(ctx->cv_mode == 1)
		{
			ctx->driver.DrvH = 0;
			ctx->driver.DrvL = 0;
			ctx->driver.SynDrv = 0;
		}else
		{
			(ctx->discharge = 1);
			ctx->v_pi.err = temp  - vRly;
			if(ctx->v_pi.err > 0.01f)ctx->v_pi.err = 0.01f;
			else if(ctx->v_pi.err < -0.9f)ctx->v_pi.err = -0.9f;
			ctx->v_pi.output = PiPwmCompute(&ctx->v_pi, ctx->v_pi.err, PWM_CV2_KP, PWM_CV2_KI);
			ctx->v_pi.output += ctx->v_pi.oldout;

			if(ctx->sample.i_bat_fir > 2.2f)
			{
				if(vRly >= 30.0f)limit = (vRly - 30.0f)*0.003f + 0.05f;
				else limit = vRly * 0.0015f + 0.02f;
			}else
			{
				limit = (temp - 30.0f)*0.004f + 0.06f;
			}
			ctx->lim_ref = 0.95f * ctx->lim_ref + limit * 0.05f;
			if(ctx->v_pi.output > ctx->lim_ref)
			{
				ctx->v_pi.output = ctx->lim_ref;
			}
			ctx->v_pi.oldout = ctx->v_pi.output;
			DriveSet(ctx, (ctx->v_pi.output < 0.007f) ? 0 : 1, 1, 0, 40000.0f, ctx->v_pi.output);
		}
	}else
	{
			ctx->driver.DrvH = 0;
			ctx->driver.DrvL = 0;
			ctx->driver.SynDrv = 0;
	}
}

/** SoftCurSt：预充抬压后转 ConVoltHandleTwo */
void ConVoltCurHandle(LlcContext *ctx)
{
	/* Persistent state belongs to ctx. */
	float temp,vRly;
	vRly = ctx->sample.v_relay_lpf;

	if(ctx->handshake == 1)temp = ctx->v_request;
	else temp = ctx->v_request;
	ctx->control.Volt_Ref = temp;
	if(ctx->control.preOK == 0)
	{
		if(ctx->control.litate == 0)
		{
			(ctx->discharge = 0);
			(ctx->relay = 0);
			ctx->control.preOK = 0;
			ctx->driver.Plv = 40000.0f;
			ctx->driver.DrvH = 1;
			ctx->driver.DrvL = 1;
			ctx->driver.SynDrv = 0;
			ctx->driver.Duty = 0.01f;
			if(vRly < 10.0f)ctx->driver.Duty = 0.01f;
			else if(vRly < 20.0f)ctx->driver.Duty = 0.02f;
			else if(vRly < 30.0f)ctx->driver.Duty = 0.03f;
			else if(vRly < 40.0f)ctx->driver.Duty = 0.04f;
			else ctx->driver.Duty = 0.05f;
			ctx->cv_delay = 0;
			if(vRly > ctx->control.Volt_Ref)ctx->control.litate = 1;
		}else
		{
			(ctx->relay = 1);
			ctx->cv_delay++;
			if(ctx->cv_delay > 1000)
			{
				ctx->control.preOK = 1;
				ctx->cv_delay = 0;
			}
			ctx->driver.Plv = 40000.0f;
			ctx->driver.DrvH = 0;
			ctx->driver.DrvL = 1;
			ctx->driver.SynDrv = 0;
			ctx->driver.Duty = 0.02f;
			ctx->v_pi.oldout = 0.01f;
		}
	}
	else
	{
		ConVoltHandleTwo(ctx);
	}
}
