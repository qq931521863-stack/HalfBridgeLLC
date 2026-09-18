#include "DllHeader.h"
#include "HwConfig.h"

#define IN_VOUT_RLY     aState->inputs[0]
#define IN_VOUT_BAT     aState->inputs[1]
#define IN_IOUT_BAT     aState->inputs[2]
#define IN_PFC_OK       aState->inputs[3]
#define IN_CTRMODE      aState->inputs[4]
#define IN_VOLT_REF     aState->inputs[5]
#define IN_CURRENT_MAX  aState->inputs[6]
#define IN_HANDSHAKE    aState->inputs[7]

#define OUT_DUTY        aState->outputs[0]
#define OUT_SYNDRV      aState->outputs[1]
#define OUT_DRVH        aState->outputs[2]
#define OUT_DRVL        aState->outputs[3]
#define OUT_PLV         aState->outputs[4]
#define OUT_CTRMODE     aState->outputs[5]
#define OUT_SRON1       aState->outputs[6]
#define OUT_SROFF1      aState->outputs[7]
#define OUT_SRON2       aState->outputs[8]
#define OUT_SROFF2      aState->outputs[9]
#define OUT_SRA         aState->outputs[10]
#define OUT_SRB         aState->outputs[11]
#define OUT_SRD         aState->outputs[12]
#define OUT_PWMON1      aState->outputs[13]
#define OUT_PWMOFF1     aState->outputs[14]
#define OUT_PWMON2      aState->outputs[15]
#define OUT_PWMOFF2     aState->outputs[16]

static double s_last_time = -1.0;

static void WriteCanFromPhysical(float volt_v, float curr_a)
{
	uint16_t v10;
	uint16_t i10;

	if (volt_v < 0.0f) volt_v = 0.0f;
	if (curr_a < 0.0f) curr_a = 0.0f;
	v10 = (uint16_t)(volt_v * 10.0f + 0.5f);
	i10 = (uint16_t)(curr_a * 10.0f + 0.5f);

	gCAN_DATA.sBMS2_ChrgMsg0_Data.BMS2_ChrgReqVolt = v10;
	gCAN_DATA.sBMS2_ChrgMsg0_Data.BMS2_ChrgReqCur = i10;
	gCAN_DATA.sPMS_ChrgMsg0_Data.PMS_ChrgReqVolt = v10;
	gCAN_DATA.sPMS_ChrgMsg0_Data.PMS_ChrgReqCur = i10;
}

DLLEXPORT void plecsSetSizes(struct SimulationSizes* aSizes)
{
	aSizes->numInputs = 8;
	aSizes->numOutputs = 17;
	aSizes->numStates = 0;
	aSizes->numParameters = 0;
}

DLLEXPORT void plecsStart(struct SimulationState* aState)
{
	(void)aState;
	s_last_time = -1.0;

	memset(&ADSample_Info, 0, sizeof(ADSample_Info));
	memset(&DataFlowFace, 0, sizeof(DataFlowFace));
	memset(&DriverPwm, 0, sizeof(DriverPwm));
	memset(&Ctrl_interFace, 0, sizeof(Ctrl_interFace));
	memset(&gCAN_DATA, 0, sizeof(gCAN_DATA));
	memset(&gSys_State, 0, sizeof(gSys_State));
	memset(&HwprotectData, 0, sizeof(HwprotectData));
	memset(&HwOtpStr, 0, sizeof(HwOtpStr));
	memset(&pfc_DataFlowFace, 0, sizeof(pfc_DataFlowFace));
	memset(&Delay1ms, 0, sizeof(Delay1ms));
	memset(&PlecsSrWin, 0, sizeof(PlecsSrWin));
	memset(&PlecsPwmWin, 0, sizeof(PlecsPwmWin));

	PowerIniPidVar();
	pfc_DataFlowFace.ACinVolRmsFir = 220.0f;
	DriverPwm.Plv = POW_INT_PLV;
}

DLLEXPORT void plecsOutput(struct SimulationState* aState)
{
	uint16_t req_c;

	if (aState->time == 0.0)
	{
		OUT_DUTY = 0.0;
		OUT_SYNDRV = 0.0;
		OUT_DRVH = 0.0;
		OUT_DRVL = 0.0;
		OUT_PLV = DriverPwm.Plv;
		OUT_CTRMODE = (double)Ctrl_interFace.CtrMode;
		OUT_SRON1 = 0.0;
		OUT_SROFF1 = 0.0;
		OUT_SRON2 = 0.0;
		OUT_SROFF2 = 0.0;
		OUT_SRA = 0.0;
		OUT_SRB = 0.0;
		OUT_SRD = 0.0;
		OUT_PWMON1 = 0.0;
		OUT_PWMOFF1 = 0.0;
		OUT_PWMON2 = 0.0;
		OUT_PWMOFF2 = 0.0;
		return;
	}

	ADSample_Info.vOut_Rly_adc = (float)IN_VOUT_RLY;
	ADSample_Info.vOut_Bat_adc = (float)IN_VOUT_BAT;
	ADSample_Info.iOut_Bat_adc = (float)IN_IOUT_BAT;
	DataFlowFace.PFC_ok = (uint8_t)IN_PFC_OK;
	if (IN_CTRMODE >= 0.0)
	{
		Ctrl_interFace.CtrMode = (CtrMode_st)(int)IN_CTRMODE;
	}
	Ctrl_interFace.Volt_Ref = (float)IN_VOLT_REF;
	Ctrl_interFace.CurrentMax = (float)IN_CURRENT_MAX;
	gSys_State.xp_HandShake = (uint8_t)IN_HANDSHAKE;

	WriteCanFromPhysical((float)IN_VOLT_REF, (float)IN_CURRENT_MAX);
	req_c = gCAN_DATA.sBMS2_ChrgMsg0_Data.BMS2_ChrgReqCur;
	if (gSys_State.xp_HandShake == 1)
	{
		gCAN_DATA.sBMS2_ChrgMsg0_Data.BMS2_ChrgEna = 1;
		gCAN_DATA.sBMS2_ChrgMsg0_Data.BMS2_ChrgReqCur = req_c;
	}
	else if (gSys_State.xp_HandShake == 2)
	{
		gCAN_DATA.sPMS_ChrgMsg0_Data.PMS_ChrgEna = 1;
	}

	if (aState->time != s_last_time)
	{
		s_last_time = aState->time;
		ADC0_Sample();
		HandleFast();
	}

	OUT_DUTY = DriverPwm.Duty;
	OUT_SYNDRV = DriverPwm.SynDrv;
	OUT_DRVH = DriverPwm.DrvH;
	OUT_DRVL = DriverPwm.DrvL;
	OUT_PLV = DriverPwm.Plv;
	OUT_CTRMODE = (double)Ctrl_interFace.CtrMode;
	OUT_SRON1 = PlecsSrWin.SrOn1;
	OUT_SROFF1 = PlecsSrWin.SrOff1;
	OUT_SRON2 = PlecsSrWin.SrOn2;
	OUT_SROFF2 = PlecsSrWin.SrOff2;
	OUT_SRA = PlecsSrWin.SrA_s;
	OUT_SRB = PlecsSrWin.SrB_s;
	OUT_SRD = PlecsSrWin.SrD_s;
	OUT_PWMON1 = PlecsPwmWin.PwmOn1;
	OUT_PWMOFF1 = PlecsPwmWin.PwmOff1;
	OUT_PWMON2 = PlecsPwmWin.PwmOn2;
	OUT_PWMOFF2 = PlecsPwmWin.PwmOff2;
}
