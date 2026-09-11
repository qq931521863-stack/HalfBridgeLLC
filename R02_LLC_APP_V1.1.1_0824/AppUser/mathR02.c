/**
 * @file mathR02.c
 * @brief LLC 快环：状态机、PWM/PFM 环路、滤波与 PI
 * @note  公式/阈值/斜坡与状态切换保持原逻辑；此处仅整理注释与边界保护
 */

#include "mathR02.h"
#include <stdint.h>
Power_interFace_t Ctrl_interFace;
pi_str piStructInter,vParamPid,iParamPid;
filter_str kFilterOne,kFilterTwo;
filter_str kFilterRly,kFilterBatV,kFilterBatI;

/**
 * @brief Handle 请求切换快环模式，真正改 CtrMode 只在 PowerCtrHandle 末尾
 */
void RequestMode(CtrMode_st next)
{
	if(next > PwmHold)
	{
		return;
	}
	Ctrl_interFace.reqMode = next;
}

/**
 * @brief 功率电压/电流统一一阶低通后再进环（FilterOne，约 4 kHz）
 * @note  慢 FIR（*_FIR）仍在 ADC0_Sample，供 CAN/切模式；保护仍用 *_adc
 */
void SampleLpfHandle(void)
{
	static uint8_t ini = 0;
	if(ini == 0)
	{
		memcpy((uint8_t*)&kFilterRly,(uint8_t*)&FilterOne,sizeof(filter_str));
		memcpy((uint8_t*)&kFilterBatV,(uint8_t*)&FilterOne,sizeof(filter_str));
		memcpy((uint8_t*)&kFilterBatI,(uint8_t*)&FilterOne,sizeof(filter_str));
		ini = 1;
	}
	ADSample_Info.vOut_Rly_LPF = OneOrderForm(ADSample_Info.vOut_Rly_adc, &kFilterRly);
	ADSample_Info.vOut_Bat_LPF = OneOrderForm(ADSample_Info.vOut_Bat_adc, &kFilterBatV);
	ADSample_Info.iOut_Bat_LPF = OneOrderForm(ADSample_Info.iOut_Bat_adc, &kFilterBatI);
}

/**
 * @brief 写驱动指令。plv<=0 或 duty 越 [0,1] 时钳位
 */
static void DriveSet(uint8_t h, uint8_t l, uint8_t syn, float plv, float duty)
{
	if(h > 1u) h = 1u;
	if(l > 1u) l = 1u;
	if(syn > 1u) syn = 1u;
	if(plv < 1.0f) plv = POW_MAX_PLV;
	if(duty < 0.0f) duty = 0.0f;
	else if(duty > 1.0f) duty = 1.0f;

	DriverPwm.DrvH = h;
	DriverPwm.DrvL = l;
	DriverPwm.SynDrv = syn;
	DriverPwm.Plv = plv;
	DriverPwm.Duty = duty;
}

/** 从 CAN 更新 CurrentMax；非 BMS/PMS 握手时保持原值 */
static void UpdateCurrentMaxFromCan(void)
{
	if(gSys_State.xp_HandShake == 1)
	{
		Ctrl_interFace.CurrentMax = gCAN_DATA.sBMS2_ChrgMsg0_Data.BMS2_ChrgReqCur * 0.1f;
	}
	else if(gSys_State.xp_HandShake == 2)
	{
		Ctrl_interFace.CurrentMax = gCAN_DATA.sPMS_ChrgMsg0_Data.PMS_ChrgReqCur * 0.1f;
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
void RefRampPwmCurr(void)
{
	if(Ctrl_interFace.CurrentMax > PWM_CURR_MAX_CLAMP)
	{
		Ctrl_interFace.CurrentMax = PWM_CURR_MAX_CLAMP;
	}

	if(Ctrl_interFace.Curr_REF < Ctrl_interFace.CurrentMax)
	{
		if(Ctrl_interFace.Curr_REF > ADSample_Info.iOut_Bat_LPF - PWM_CURR_TRACK_BAND)
		{
			Ctrl_interFace.Curr_REF = RampToward(Ctrl_interFace.Curr_REF,
				Ctrl_interFace.CurrentMax, REF_RAMP_STEP);
		}
	}else
	{
		if(Ctrl_interFace.Curr_REF < ADSample_Info.iOut_Bat_LPF + PWM_CURR_TRACK_BAND)
		{
			Ctrl_interFace.Curr_REF = RampToward(Ctrl_interFace.Curr_REF,
				Ctrl_interFace.CurrentMax, REF_RAMP_STEP);
		}
	}
}

/**
 * @brief PFM 电流缓变：功率 P/V、CON_CURR_OUT、OTP 后再直追 CurrentMax
 * @note  使能门槛 CurrentMax>0.5A 仍在 ConPfmHandle；无 ±1A 跟踪窗
 */
void RefRampPfmCurr(void)
{
	float err, out = 0;
	float vBatFir;

	err = pfc_DataFlowFace.ACinVolRmsFir * 7.2f;
	if(err > 1600.0f)err = 1600.0f;
	vBatFir = ADSample_Info.vOut_Bat_FIR;
	if(vBatFir > PFM_VBAT_DIV_MIN)
	{
		out = err / vBatFir;
		if(Ctrl_interFace.CurrentMax > out)Ctrl_interFace.CurrentMax = out;
	}
	if(Ctrl_interFace.CurrentMax > CON_CURR_OUT)Ctrl_interFace.CurrentMax = CON_CURR_OUT;
	if(HwOtpStr.ErrStaErr == 2)
	{
		Ctrl_interFace.CurrentMax *= 0.5f;
	}else if(HwOtpStr.ErrStaErr == 1)
	{
		Ctrl_interFace.CurrentMax *= 0.75f;
	}

	Ctrl_interFace.Curr_REF = RampToward(Ctrl_interFace.Curr_REF,
		Ctrl_interFace.CurrentMax, REF_RAMP_STEP);
}

/**
 * @brief PWM 同步管按输出电流开通（原 SHRTIMERdrive 的 delaySr）
 *        >4A 连续 1000 拍开通；已开通后 <2A 关闭
 */
static void PwmSyncDrvUpdate(void)
{
	static uint16_t delaySr = 0;

	if(DriverPwm.SynDrv == 0)
	{
		if(ADSample_Info.iOut_Bat_adc > 4.0f)
		{
			delaySr++;
			if(delaySr >= 1000)
			{
				delaySr = 1000;
				DriverPwm.SynDrv = 1;
			}
		}else delaySr = 0;
	}else
	{
		delaySr = 0;
		if(ADSample_Info.iOut_Bat_adc < 2.0f)DriverPwm.SynDrv = 0;
	}
}

/** 空闲关波。故障/PFC 未就绪时放电与继电器已由 HandleFast 配好 */
void ConIdleHandle(void)
{
	if((DataFlowFace.FaultSta.all == 0) && (DataFlowFace.PFC_ok != 0))
	{
		DischargeOff();
		RelayOff();
	}
	PowerIniPidVar();
	Ctrl_interFace.preOK = 0;
	DriveSet(0, 0, 0, POW_MAX_PLV, 0.02f);
	Ctrl_interFace.time = 0;
}

/**
 * @brief PWM→PFM 前保持：phase1 降占空比等电流；phase2 切 Transition
 */
void ConPwmHoldHandle(void)
{
	if(Ctrl_interFace.holdPhase == 2)
	{
		Ctrl_interFace.time = 0;
		Ctrl_interFace.Curr_REF = 0;
		RequestMode(Transition);
		Ctrl_interFace.flvOut = POW_MAX_PLV;
		piStructInter.integral = PID_MAX_PLV;
		piStructInter.fir = 0;
		DriveSet(0, 1, 0, Ctrl_interFace.flvOut, 0.05f);
	}else
	{
		DriveSet(0, 1, 0, 80000.0f, 0.04f);
		if(ADSample_Info.iOut_Bat_FIR < 1.0f)
		{
			Ctrl_interFace.time++;
			if(Ctrl_interFace.time >= 40)
			{
				Ctrl_interFace.holdPhase = 2;
				Ctrl_interFace.time = 0;
			}
		}
	}
	PwmSyncDrvUpdate();
}

/**
 * @brief PWM→PFM 过渡：有电流请求则 Duty 斜坡 400 拍后进 PFM
 */
void ConTransHandle(void)
{
	UpdateCurrentMaxFromCan();
	if(Ctrl_interFace.CurrentMax > 0.5f)
	{
		Ctrl_interFace.time++;
		if(Ctrl_interFace.time >= 400)
		{
			Ctrl_interFace.flvOut = POW_MAX_PLV;
			piStructInter.integral = PID_MAX_PLV;
			piStructInter.fir = 0;
			Ctrl_interFace.time = 0;
			RequestMode(OvLoadPFM);
		}
		Ctrl_interFace.Curr_REF = 0;
		DriveSet(1, 1, 0, POW_MAX_PLV, Ctrl_interFace.time * 0.001f + 0.05f);
	}else
	{
		Ctrl_interFace.flvOut = POW_MAX_PLV;
		piStructInter.integral = PID_MAX_PLV;
		piStructInter.fir = 0;
		Ctrl_interFace.Curr_REF = 0;
		DriveSet(0, 0, 0, POW_MAX_PLV, DriverPwm.Duty);
	}
}

/**
 * @brief PFM 电流环
 */
void ConPfmHandle(void)
{
	float err,out = 0;

	UpdateCurrentMaxFromCan();
	if((gSys_State.xp_HandShake == 0)||(ADSample_Info.vOut_Bat_adc < 39.0f))
	{
		Ctrl_interFace.flvOut = POW_MAX_PLV;
		piStructInter.integral = PID_MAX_PLV;
		piStructInter.fir = 0;
		Ctrl_interFace.Curr_REF = 0;
		DriveSet(0, 0, 0, POW_MAX_PLV, DriverPwm.Duty);
	}else
	{
		if(Ctrl_interFace.CurrentMax > 0.5f)
		{
			RefRampPfmCurr();
			mComputer();
			err = ADSample_Info.iOut_Bat_LPF - Ctrl_interFace.Curr_REF;
			out = Compensator_Pid(&piStructInter, err);
			Ctrl_interFace.flvOut = out * 50000.0f;
			if(Ctrl_interFace.flvOut > POW_MAX_PLV)
			{
				Ctrl_interFace.flvOut = POW_MAX_PLV;
				piStructInter.integral = PID_MAX_PLV;
			}
			if(Ctrl_interFace.flvOut < POW_MIN_PLV)
			{
				Ctrl_interFace.flvOut = POW_MIN_PLV;
				piStructInter.integral = PID_MIN_PLV;
			}
			if(mathTimeHandle() == 0)
			{
				DriverPwm.SynDrv = 0;
			}
			DriveSet(1, 1, DriverPwm.SynDrv, Ctrl_interFace.flvOut, 0.5f);
			if(ADSample_Info.iOut_Bat_adc < 4.0f)
			{
				DriverPwm.Sr_Dtime = 0;
				DriverPwm.SynDrv = 0;
			}
		}else
		{
			Ctrl_interFace.time = 0;
			RequestMode(Transition);
			Ctrl_interFace.flvOut = POW_MAX_PLV;
			piStructInter.integral = PID_MAX_PLV;
			piStructInter.fir = 0;
			Ctrl_interFace.Curr_REF = 0;
			DriveSet(0, 0, 0, POW_MAX_PLV, DriverPwm.Duty);
		}
	}
}

/**
 * @brief 快环状态机：只调 Handle，再采纳 reqMode
 */
void PowerCtrHandle(void)
{
	Ctrl_interFace.reqMode = Ctrl_interFace.CtrMode;

	switch(Ctrl_interFace.CtrMode)
	{
		case  NoSelect :
			ConIdleHandle();
		break;
		case  SoftStar :
			SoftStart();
		break;
		case BmsStar :
			SoftCurStart();
		break;
		case SoftCurSt:
			ConVoltCurHandle();
			Ctrl_interFace.time = 0;
		break;
		case ConVolt :
			ConVoltHandle();
		break;
		case  ConCurPWM :
			ConCurrHandle();
		break;
		case  PwmHold :
			ConPwmHoldHandle();
		break;
		case  Transition:
			ConTransHandle();
		break;
		case  OvLoadPFM :
			ConPfmHandle();
		break;
		default:
		break;
	}

	if(Ctrl_interFace.reqMode != Ctrl_interFace.CtrMode)
	{
		if(Ctrl_interFace.reqMode <= PwmHold)
		{
			Ctrl_interFace.CtrMode = Ctrl_interFace.reqMode;
		}
	}
}

/** 复位 PFM PI 与进环滤波器状态 */
void PowerIniPidVar(void)
{
	memset((uint8_t*)&piStructInter, 0, sizeof(pi_str));
	memcpy((uint8_t*)&kFilterTwo,(uint8_t*)&FilterTwo,sizeof(filter_str));
	memcpy((uint8_t*)&kFilterOne,(uint8_t*)&FilterOne,sizeof(filter_str));
	memcpy((uint8_t*)&kFilterRly,(uint8_t*)&FilterOne,sizeof(filter_str));
	memcpy((uint8_t*)&kFilterBatV,(uint8_t*)&FilterOne,sizeof(filter_str));
	memcpy((uint8_t*)&kFilterBatI,(uint8_t*)&FilterOne,sizeof(filter_str));
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

uint16_t starTime = 0;

/** 无握手唤醒：开环抬继电器电压到 30V 后切恒压 */
void SoftStart(void)
{
	static uint16_t preNum = 0;
	float vRly;

	vRly = ADSample_Info.vOut_Rly_LPF;

	Ctrl_interFace.Volt_Ref = 30.0f;
	if(DataFlowFace.RelaySta == 0)
	{
		if(vRly < Ctrl_interFace.Volt_Ref)
		{
			RelayOff();
			DischargeOff();
			Ctrl_interFace.preOK = 0;
			DriverPwm.Plv = 120000.0f;
			DriverPwm.DrvH = 1;
			DriverPwm.DrvL = 1;
			DriverPwm.SynDrv = 0;
			DriverPwm.Duty = 0.035f;
			if(vRly > 25.0f)DriverPwm.Duty = 0.05f;
			else if(vRly > 20.0f)DriverPwm.Duty = 0.047f;
			else if(vRly > 15.0f)DriverPwm.Duty = 0.044f;
			else if(vRly > 10.0f) DriverPwm.Duty = 0.04f;
		}else
		{
			RelayOn();
			preNum = 0;
		}
	}else
	{
		preNum++;
		if(preNum > 1000)
		{
			RequestMode(ConVolt);
			preNum = 0;
			starTime = 0;
		}
		Ctrl_interFace.preOK = 1;
		DriverPwm.Plv = POW_MAX_PLV;
		DriverPwm.DrvH = 0;
		DriverPwm.DrvL = 0;
		DriverPwm.SynDrv = 0;
		DriverPwm.Duty = 0.01f;
		vParamPid.oldout = 0.01f;
	}
}

/** PWM 恒压 30V，电流限制 5.4A，双环取小后累加占空比 */
void ConVoltHandle(void)
{
	float temp, vRly, outV, outI;

	Ctrl_interFace.Volt_Ref = 30.0f;
	vRly = ADSample_Info.vOut_Rly_LPF;
	if(vRly < Ctrl_interFace.Volt_Ref)temp = vRly + 0.01f;
	else temp = Ctrl_interFace.Volt_Ref;

	outV = PiPwmCompute(&vParamPid, temp - vRly, PWM_V_KP, PWM_V_KI);
	outI = PiPwmCompute(&iParamPid, 5.4f - ADSample_Info.iOut_Bat_LPF, PWM_I_LIM_KP, PWM_I_LIM_KI);
	if(outV > outI)outV = outI;
	outV += vParamPid.oldout;
	if(outV > 0.4f)outV = 0.4f;
	vParamPid.oldout = outV;
	vParamPid.output = outV;

	DischargeOn();
	DriveSet((outV < 0.02f) ? 0 : 1, 1, 0, 120000.0f, outV);
}

/** PWM 恒流；电池电压≥40V 请求进入 PwmHold */
void ConCurrHandle(void)
{
	if(ADSample_Info.vOut_Bat_adc >= 40.0f)
	{
		Ctrl_interFace.holdPhase = 1;
		Ctrl_interFace.time = 0;
		Ctrl_interFace.Curr_REF = 0;
		RequestMode(PwmHold);
		DriveSet(0, 1, 0, 80000.0f, 0.02f);
		PwmSyncDrvUpdate();
		return;
	}

	if(gSys_State.xp_HandShake == 0)
	{
		DriverPwm.DrvH = 0;
		DriverPwm.DrvL = 0;
		DriverPwm.SynDrv = 0;
	}else
	{
		UpdateCurrentMaxFromCan();
		if(Ctrl_interFace.CurrentMax > 0.1f)
		{
			RefRampPwmCurr();

			iParamPid.err = Ctrl_interFace.Curr_REF - ADSample_Info.iOut_Bat_LPF;
			if(iParamPid.err > 0.2f)iParamPid.err = 0.2f;
			else if(iParamPid.err < -0.2f)iParamPid.err = -0.2f;
			iParamPid.output = PiPwmCompute(&iParamPid, iParamPid.err, PWM_CUR_KP, PWM_CUR_KI);
			testDuty = iParamPid.oldout;
			iParamPid.output = iParamPid.oldout + iParamPid.output;

			if(iParamPid.output > 0.4f)iParamPid.output = 0.4f;
			if(iParamPid.output < 0.02f)
			{
				iParamPid.output = 0.02f;
				iParamPid.oldout = iParamPid.output;
				DriveSet(0, 1, 0, 80000.0f, iParamPid.output);
			}
			else
			{
				iParamPid.oldout = iParamPid.output;
				DriveSet(1, 1, 0, 80000.0f, iParamPid.output);
			}
		}else
		{
			DriverPwm.DrvH = 0;
			DriverPwm.DrvL = 0;
			DriverPwm.SynDrv = 0;
			iParamPid.output = 0;
			iParamPid.oldout = 0;
			DriverPwm.Duty = 0;
			DriverPwm.Sr_Dtime = 0;
			Ctrl_interFace.Curr_REF = 0;
		}
	}
	PwmSyncDrvUpdate();
}

/** 有握手预充：抬压吸合继电器后，按电池电压进 PWM 或直接 Transition */
void SoftCurStart(void)
{
	static uint16_t preChargeTime = 0,preChargeTime2 = 0;
	float vRly,vBat = 0;
	vBat = ADSample_Info.vOut_Bat_adc;
	vRly = ADSample_Info.vOut_Rly_LPF;
	if(Ctrl_interFace.preOK == 0)
	{
		if(Ctrl_interFace.litate == 0)
		{
			DischargeOff();
			RelayOff();
			Ctrl_interFace.preOK = 0;
			preChargeTime = 0;
			DriverPwm.Plv = 40000.0f;
			DriverPwm.DrvH = 1;
			DriverPwm.DrvL = 1;
			DriverPwm.SynDrv = 0;

			if(vRly < 10.0f)DriverPwm.Duty = 0.01f;
			else if(vRly < 20.0f)DriverPwm.Duty = 0.02f;
			else if(vRly < 30.0f)DriverPwm.Duty = 0.03f;
			else if(vRly < 40.0f)DriverPwm.Duty = 0.04f;
			else if(vRly < 50.0f)DriverPwm.Duty = 0.06f;
			else DriverPwm.Duty = 0.07f;
			if(vRly > vBat)DriverPwm.Duty = 0.05f;
			if(vRly >= vBat + 2.0f)
			{
				Ctrl_interFace.litate = 1;
				preChargeTime = 0;
			}
		}else if(Ctrl_interFace.litate == 1)
		{
			DischargeOn();
			RelayOff();
			if(vRly < vBat + 0.5f)
			{
				Ctrl_interFace.litate = 0;
				preChargeTime = 0;
			}else
			{
				preChargeTime++;
				if(preChargeTime >= 20)
				{
					Ctrl_interFace.preOK = 1;
					preChargeTime = 0;
				}
			}
			DriverPwm.Plv = POW_MAX_PLV;
			DriverPwm.DrvH = 0;
			DriverPwm.DrvL = 0;
			DriverPwm.SynDrv = 0;
			DriverPwm.Duty = 0.02f;
		}
	}else if(Ctrl_interFace.preOK == 1)
	{
		if(vRly < vBat + 0.2f)
		{
			preChargeTime++;
			if(preChargeTime >= 20)
			{
				Ctrl_interFace.preOK = 2;
				preChargeTime = 0;
				preChargeTime2 = 0;
				RelayOn();
			}
			DischargeOff();
			DriverPwm.Plv = POW_MAX_PLV;
			DriverPwm.DrvH = 0;
			DriverPwm.DrvL = 0;
			DriverPwm.SynDrv = 0;
			DriverPwm.Duty = 0.02f;
		}
	}
	else if(DataFlowFace.RelaySta == 1)
	{
		if(vBat <  40.0f)
		{
			preChargeTime++;
			if(preChargeTime >= 20000)
			{
				iParamPid.integral = 0;
				iParamPid.oldout = 0;
				preChargeTime = 0;
				iParamPid.oldout = 0;
				Ctrl_interFace.Curr_REF = 0;
				RequestMode(ConCurPWM);
			}
		}else if(vBat > 40.5f) preChargeTime = 0;
		if(vBat >=  40.0f)
		{
			preChargeTime2++;
			if(preChargeTime2 >= 20000)
			{
				RequestMode(Transition);
				Ctrl_interFace.Curr_REF = 0;
				preChargeTime2 = 0;
			}
		}else if(vBat <  39.5f) preChargeTime2 = 0;

		DischargeOff();
		DriverPwm.Plv = POW_MAX_PLV;
		DriverPwm.DrvH = 0;
		DriverPwm.DrvL = 0;
		DriverPwm.SynDrv = 0;
		DriverPwm.Duty = 0.02f;
	}
}

/** CV 恒压（CAN 30~60V），占空比按电压/电流限幅 */
void ConVoltHandleTwo(void)
{
	float temp,vRly,limit = 0;
	static float limRef = 0.05f;
	vRly = ADSample_Info.vOut_Rly_LPF;
	temp = gCAN_DATA.sPMS_ChrgMsg0_Data.PMS_ChrgReqVolt * 0.1f;

	if((temp >= 30.0f)&&(temp <= 60.0f))
	{
		if(gSys_State.xp_CVmode == 1)
		{
			DriverPwm.DrvH = 0;
			DriverPwm.DrvL = 0;
			DriverPwm.SynDrv = 0;
		}else
		{
			DischargeOn();
			vParamPid.err = temp  - vRly;
			if(vParamPid.err > 0.01f)vParamPid.err = 0.01f;
			else if(vParamPid.err < -0.9f)vParamPid.err = -0.9f;
			vParamPid.output = PiPwmCompute(&vParamPid, vParamPid.err, PWM_CV2_KP, PWM_CV2_KI);
			vParamPid.output += vParamPid.oldout;

			if(ADSample_Info.iOut_Bat_FIR > 2.2f)
			{
				if(vRly >= 30.0f)limit = (vRly - 30.0f)*0.003f + 0.05f;
				else limit = vRly * 0.0015f + 0.02f;
			}else
			{
				limit = (temp - 30.0f)*0.004f + 0.06f;
			}
			limRef = 0.95f * limRef + limit * 0.05f;
			if(vParamPid.output > limRef)
			{
				vParamPid.output = limRef;
			}
			vParamPid.oldout = vParamPid.output;
			DriveSet((vParamPid.output < 0.007f) ? 0 : 1, 1, 0, 40000.0f, vParamPid.output);
		}
	}else
	{
			DriverPwm.DrvH = 0;
			DriverPwm.DrvL = 0;
			DriverPwm.SynDrv = 0;
	}
}

/** SoftCurSt：预充抬压后转 ConVoltHandleTwo */
void ConVoltCurHandle(void)
{
	static uint16_t delay = 0;
	float temp,vRly;
	vRly = ADSample_Info.vOut_Rly_LPF;

	if(gSys_State.xp_HandShake == 1)temp = gCAN_DATA.sBMS2_ChrgMsg0_Data.BMS2_ChrgReqVolt * 0.1f;
	else temp = gCAN_DATA.sPMS_ChrgMsg0_Data.PMS_ChrgReqVolt * 0.1f;
	Ctrl_interFace.Volt_Ref = temp;
	if(Ctrl_interFace.preOK == 0)
	{
		if(Ctrl_interFace.litate == 0)
		{
			DischargeOff();
			RelayOff();
			Ctrl_interFace.preOK = 0;
			DriverPwm.Plv = 40000.0f;
			DriverPwm.DrvH = 1;
			DriverPwm.DrvL = 1;
			DriverPwm.SynDrv = 0;
			DriverPwm.Duty = 0.01f;
			if(vRly < 10.0f)DriverPwm.Duty = 0.01f;
			else if(vRly < 20.0f)DriverPwm.Duty = 0.02f;
			else if(vRly < 30.0f)DriverPwm.Duty = 0.03f;
			else if(vRly < 40.0f)DriverPwm.Duty = 0.04f;
			else DriverPwm.Duty = 0.05f;
			delay = 0;
			if(vRly > Ctrl_interFace.Volt_Ref)Ctrl_interFace.litate = 1;
		}else
		{
			RelayOn();
			delay++;
			if(delay > 1000)
			{
				Ctrl_interFace.preOK = 1;
				delay = 0;
			}
			DriverPwm.Plv = 40000.0f;
			DriverPwm.DrvH = 0;
			DriverPwm.DrvL = 1;
			DriverPwm.SynDrv = 0;
			DriverPwm.Duty = 0.02f;
			vParamPid.oldout = 0.01f;
		}
	}
	else
	{
		ConVoltHandleTwo();
	}
}
