#include "DllHeader.h"
#include "HwConfig.h"

#define IN_VOUT_RLY     aState->inputs[0]
#define IN_VOUT_BAT     aState->inputs[1]
#define IN_IOUT_BAT     aState->inputs[2]
#define IN_CTRMODE_OBS  aState->inputs[3]
#define IN_PREOK        aState->inputs[4]
#define IN_CHRG_ENA     aState->inputs[5]
#define IN_HANDSHAKE    aState->inputs[6]
#define IN_VREQ         aState->inputs[7]

#define OUT_CTRMODE     aState->outputs[0]
#define OUT_VOLT_REF    aState->outputs[1]
#define OUT_CURRENT_MAX aState->outputs[2]
#define OUT_HANDSHAKE   aState->outputs[3]
#define OUT_RELAY       aState->outputs[4]
#define OUT_DISCHARGE   aState->outputs[5]

static double s_last_time = -1.0;
static int s_last_mode_cmd = -1;
static CtrMode_st s_mode_before = NoSelect;

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
	aSizes->numOutputs = 6;
	aSizes->numStates = 0;
	aSizes->numParameters = 0;
}

DLLEXPORT void plecsStart(struct SimulationState* aState)
{
	(void)aState;
	s_last_time = -1.0;
	s_last_mode_cmd = -1;
	s_mode_before = NoSelect;

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

	PowerIniPidVar();
	pfc_DataFlowFace.ACinVolRmsFir = 220.0f;
	Delay1ms.delay_ini = 100;
	DataFlowFace.PFC_ok = 1;
}

DLLEXPORT void plecsOutput(struct SimulationState* aState)
{
	int mode_cmd;

	(void)IN_CTRMODE_OBS;

	if (aState->time == 0.0)
	{
		OUT_CTRMODE = -1.0;
		OUT_VOLT_REF = Ctrl_interFace.Volt_Ref;
		OUT_CURRENT_MAX = Ctrl_interFace.CurrentMax;
		OUT_HANDSHAKE = gSys_State.xp_HandShake;
		OUT_RELAY = DataFlowFace.RelaySta;
		OUT_DISCHARGE = DataFlowFace.DisCharge;
		return;
	}

	ADSample_Info.vOut_Rly_adc = (float)IN_VOUT_RLY;
	ADSample_Info.vOut_Bat_adc = (float)IN_VOUT_BAT;
	ADSample_Info.iOut_Bat_adc = (float)IN_IOUT_BAT;
	ADSample_Info.vOut_Bat_FIR = ADSample_Info.vOut_Bat_adc;
	ADSample_Info.iOut_Bat_FIR = ADSample_Info.iOut_Bat_adc;
	Ctrl_interFace.preOK = (uint8_t)IN_PREOK;
	gSys_State.xp_ChrgEna = (uint8_t)IN_CHRG_ENA;
	gSys_State.xp_HandShake = (uint8_t)IN_HANDSHAKE;
	WriteCanFromPhysical((float)IN_VREQ, 4.0f);

	if (gSys_State.xp_HandShake == 1)
	{
		gCAN_DATA.sBMS2_ChrgMsg0_Data.BMS2_ChrgEna = gSys_State.xp_ChrgEna;
		gSys_State.xp_CVmode = 1;
	}
	else if (gSys_State.xp_HandShake == 2)
	{
		gCAN_DATA.sPMS_ChrgMsg0_Data.PMS_ChrgEna = gSys_State.xp_ChrgEna;
		gCAN_DATA.sPMS_ChrgMsg0_Data.PMS_CVModeReq = 1;
		gSys_State.xpRx_CVmode = 1;
		gSys_State.xp_CVmode = 2;
	}

	/* PLECS only: map CAN bits to slow-loop states. Keil uses operateStatus. */
	if (gSys_State.xp_ChrgEna == 1)
	{
		DataFlowFace.RunState = Charging;
	}
	else if (gSys_State.xp_HandShake == 0)
	{
		DataFlowFace.RunState = Wakeup;
	}
	else
	{
		DataFlowFace.RunState = Stadby;
	}

	if (aState->time != s_last_time)
	{
		s_last_time = aState->time;
		s_mode_before = Ctrl_interFace.CtrMode;
		StateM();
	}

	if ((int)Ctrl_interFace.CtrMode != (int)s_mode_before)
	{
		mode_cmd = (int)Ctrl_interFace.CtrMode;
		s_last_mode_cmd = mode_cmd;
	}
	else
	{
		mode_cmd = -1;
	}

	OUT_CTRMODE = (double)mode_cmd;
	OUT_VOLT_REF = Ctrl_interFace.Volt_Ref;
	OUT_CURRENT_MAX = Ctrl_interFace.CurrentMax;
	OUT_HANDSHAKE = gSys_State.xp_HandShake;
	OUT_RELAY = DataFlowFace.RelaySta;
	OUT_DISCHARGE = DataFlowFace.DisCharge;
}
