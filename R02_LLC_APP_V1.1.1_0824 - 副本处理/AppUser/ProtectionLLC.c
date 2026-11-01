
#include "ProtectionLLC.h"
#include "HwConfig.h"

TSysProData SysProtectData[SYS_PROTECT_NUM];
HwProData  HwprotectData;
HwOtProData HwOtpStr;
const TSysProPara  SysProPara[SYS_PROTECT_NUM] = {
	//ID 使能 比较 阀值A   阀值B   时间A  次数 重复次数  重启间隔  清除时间  清除使能   比较 阀值A   阀值B  时间A                          

	{ 0,   1,  1,   2.0f,   0,     200,   20,    1,     40000,    6000,     1 ,      0,  -2.0f, 30.0f, 20000 }, //1.输出过压62
	{ 1,   1,  0,  26.0f,   0,     200,   20,    1,     40000,    6000,     0 ,      1,  28.0f,  0.0f,   200 }, //2.输出欠压
	{ 2,   1,  1,  42.0f,   0,     200,   20,    1,     40000,    6000,     0 ,      0,   0.0f,  0.0f,  8000 }, //3.输出过流	
	{ 3,   1,  2,  12.8f, 11.2f,   200,   20,    1,     1000 ,    6000,     0 ,      0,  64.0f,  0.0f,   200 }, //4.LLC辅源故障 5ms
	{ 4,   1,  0,  26.0f, 10.0f,   200,   20,    1,     1000 ,    6000,     0 ,      0,  64.0f,  0.0f,   200 }, //5.恒压模式欠压
	{ 5,   1,  2,   2.0f, -2.0f,   200,   20,    1,     1000 ,    6000,     0 ,      0,  64.0f,  0.0f,   200 }, //6.输出继电器故障
	{ 6,   1,  2,   2.0f, -2.0f,  4000,   20,    1,     1000 ,    6000,     0 ,      0,  64.0f,  0.0f,   200 }, //7.谐振电流过流故障
	{ 7,   1,  1,   65.0f, 0.0f,  4000,   20,    1,     1000 ,    6000,     0 ,      0,  64.0f,  0.0f,  8000 }, //8.继电器前端电压过压故障65
};

const float OTP_SysProParaUp[4][8] = {                   

	{ 85.0f,   105.0f,  105.0f,   85.0f,   85.0f,    70.0f,   65.0f,    0, }, 
	{ 95.0f,   115.0f,  115.0f,   95.0f,   95.0f,    80.0f,   75.0f,    0, }, 
	{ 105.0f,  125.0f,  125.0f,   105.0f,  105.0f,   90.0f,   85.0f,    0, }, 
	{ 60.0f,   80.0f,   80.0f,    60.0f,   60.0f,    60.0f,   45.0f,    0, },	
};

const float OTP_SysProParaDw[4][8] = {                   

	{ 75.0f,    95.0f,   95.0f,   75.0f,   75.0f,    60.0f,   55.0f,    0, }, 
	{ 80.0f,   100.0f,  100.0f,   80.0f,   80.0f,    65.0f,   60.0f,    0, }, 
	{ 60.0f,   80.0f,   80.0f,    60.0f,   60.0f,    60.0f,   45.0f,    0, }, 
	{ 60.0f,   80.0f,   80.0f,    60.0f,   60.0f,    60.0f,   45.0f,    0, },	
};
uint8_t  Protect_comm(uint8_t fault, uint8_t faultID,float input)
{
	uint8_t i,sult = 0,faultbitA = 0;
	uint32_t sum=0;
	faultbitA = fault;
	if(Delay1ms.powerOn >= 2000)
	{		
		SysProtectData[faultID].TimeCount ++;
		if(SysProtectData[faultID].TimeCount > 20000)SysProtectData[faultID].TimeCount = 20000;
		if(faultbitA)
		{
			if(SysProPara[faultID].ClearEnable)
			{
				sult = 0;
				if(SysProPara[faultID].DirectionConfig == 0)
				{
					if(input < SysProPara[faultID].Threshold_C)sult = 1;
				}else if(SysProPara[faultID].DirectionConfig == 1)
				{
					if(input > SysProPara[faultID].Threshold_C)sult = 1;
				}else if(SysProPara[faultID].DirectionConfig == 2)
				{
					if(input > SysProPara[faultID].Threshold_C)sult = 1;
					else if(input < SysProPara[faultID].Threshold_D)sult = 1;
				}else
				{
					if(input < SysProPara[faultID].Threshold_C)
					{
					  if(input > SysProPara[faultID].Threshold_D)sult = 1;
					}						
				}
				if(sult)
				{
 				  if(SysProtectData[faultID].TimeCount >= SysProPara[faultID].DelayTimeC)
					{
						//清除故障
						faultbitA = 0;	
            SysProtectData[faultID].TimeCount = 0;						
					}
				}else
				{
					SysProtectData[faultID].TimeCount = 0;
				}				
			}
		}else if((DataFlowFace.RelayOld)||(faultID == 3))
		{
			sult = 0;
			if(SysProPara[faultID].byDirectionConfig == 0)
			{
				if(input < SysProPara[faultID].Threshold_A)sult = 1;
			}else if(SysProPara[faultID].byDirectionConfig == 1)
			{
				if(input > SysProPara[faultID].Threshold_A)sult = 1;
			}else if(SysProPara[faultID].byDirectionConfig == 2)
			{
				if(input > SysProPara[faultID].Threshold_A)sult = 1;
				else if(input < SysProPara[faultID].Threshold_B)sult = 1;
			}else
			{
				if(input < SysProPara[faultID].Threshold_A)
				{
					if(input > SysProPara[faultID].Threshold_B)sult = 1;
				}						
			}
			if(sult)
			{
				SysProtectData[faultID].Delay[SysProtectData[faultID].DelayNum] = SysProtectData[faultID].TimeCount;
				SysProtectData[faultID].TimeCount = 0;
				SysProtectData[faultID].DelayNum ++;
				if(SysProtectData[faultID].DelayNum >= SysProPara[faultID].ErrNum)
				{
					SysProtectData[faultID].DelayNum = 0;
					SysProtectData[faultID].Full = 1;
				}

				if(SysProtectData[faultID].Full)
				{
					if(SysProPara[faultID].ErrNum == 1)
					{
						for(i = 0;i < SysProPara[faultID].ErrNum;i++)SysProtectData[faultID].Delay[i] = 0;
						SysProtectData[faultID].DelayNum = 0;
						SysProtectData[faultID].Full = 0;
						faultbitA = 1;					
					}else
					{
						sum = 0;						
						for(i = 0;i < SysProPara[faultID].ErrNum;i++)sum += SysProtectData[faultID].Delay[i];
						if(sum < SysProPara[faultID].DelayTimeA)
						{							
							//故障发生
							for(i = 0;i < SysProPara[faultID].ErrNum;i++)SysProtectData[faultID].Delay[i] = 0;
							SysProtectData[faultID].DelayNum = 0;
							SysProtectData[faultID].Full = 0;
							faultbitA = 1;
						}
				  }
				}
			}
		}	
	}

	return faultbitA;
}

void OutOvProtect(void)
{
	uint8_t fault = 0,sult = 0;
  float input = ADSample_Info.vOut_Bat_adc;
	if(DataFlowFace.RunState == Wakeup){input -= 30.0f;sult = 1;}
	else if(DataFlowFace.RunState == Charging){input -= 60.0f;sult = 1;}
	if(sult)
	{
		if(DataFlowFace.FaultSta.bit.Out_ov)fault = 1;
		else fault = 0;
		sult = Protect_comm(fault,0,input);
		if(sult)DataFlowFace.FaultSta.bit.Out_ov = 1;
		else DataFlowFace.FaultSta.bit.Out_ov = 0;
	}
}

void OutUvProtect(void)
{
	uint8_t fault = 0,sult = 0;
	float input = ADSample_Info.vOut_Bat_adc;
  if(DataFlowFace.FaultSta.bit.Out_uv)fault = 1;
	else fault = 0;
  sult = Protect_comm(fault,1,input);
	if(sult)DataFlowFace.FaultSta.bit.Out_uv = 1;
	else DataFlowFace.FaultSta.bit.Out_uv = 0;	
}
void OutUvConProtect(void)
{
	uint8_t fault = 0,sult = 0;
	float input = ADSample_Info.vOut_Bat_adc;
  if(DataFlowFace.FaultSta.bit.Out_uv)fault = 1;
	else fault = 0;
  sult = Protect_comm(fault,4,input);
	if(sult)DataFlowFace.FaultSta.bit.Out_uv = 1;
	else DataFlowFace.FaultSta.bit.Out_uv = 0;	
}
void OutOcProtect(void)
{
	uint8_t fault = 0,sult = 0;
	float input = ADSample_Info.iOut_Bat_adc;
  if(DataFlowFace.FaultSta.bit.Out_oc)fault = 1;
	else fault = 0;
  sult = Protect_comm(fault,2,input);
	if(sult)DataFlowFace.FaultSta.bit.Out_oc = 1;
	else DataFlowFace.FaultSta.bit.Out_oc = 0;	
}

void AuxProtect(void)
{
	uint8_t fault = 0,sult = 0;
	float input = ADSample_Info.AuxVolt;
  if(DataFlowFace.FaultSta.bit.LLC_Err)fault = 1;
	else fault = 0;
  sult = Protect_comm(fault,3,input);
	if(sult)DataFlowFace.FaultSta.bit.LLC_Err = 1;
	else DataFlowFace.FaultSta.bit.LLC_Err = 0;	
}

void RelayProtect(void)
{
	uint8_t fault = 0,sult = 0;
	float input = ADSample_Info.vOut_Rly_adc - ADSample_Info.vOut_Bat_adc;
  if(DataFlowFace.FaultSta.bit.outRLY_Err)fault = 1;
	else fault = 0;
  sult = Protect_comm(fault,5,input);
	if(sult)DataFlowFace.FaultSta.bit.outRLY_Err = 1;
	else DataFlowFace.FaultSta.bit.outRLY_Err = 0; 
}

void HwProtect(void)
{	
  OutOvProtect();	
//	OutUvProtect();
//	if(gSys_State.xp_CVmode == 1)OutUvProtect();
//	else OutUvConProtect();
}

void HwProtectIni(void)
{
	uint8_t i = 0;
	for(i = 0; i < SYS_PROTECT_NUM; i++)memset(&SysProtectData[i], 0, sizeof(TSysProPara));
	memset(&HwprotectData, 0, sizeof(HwProData));	
	memset(&HwOtpStr, 0, sizeof(HwOtProData));	
}

void SysReset(void)
{
	if(pfc_DataFlowFace.ACinVolRMS < 30.0f)
	{
		HwprotectData.iniOk = 0;		
	}
	if(pfc_DataFlowFace.ACinVolRMS > 60.0f)
	{		
		if(HwprotectData.NorVoltTime < 4)
		{
			HwprotectData.NorVoltTime++;
		}else if(HwprotectData.iniOk == 0)
		{			
			HAL_HRTIM_WaveformOutputStart(&hhrtim1,HRTIM_OUTPUT_TC1 | HRTIM_OUTPUT_TC2 | HRTIM_OUTPUT_TD1 | HRTIM_OUTPUT_TD2);	
			HAL_HRTIM_WaveformCountStart(&hhrtim1, HRTIM_TIMERID_TIMER_C);
			HAL_HRTIM_WaveformCountStart(&hhrtim1, HRTIM_TIMERID_TIMER_D);	
      HwprotectData.iniOk = 1;			
		}
	}else 
	{
		HwprotectData.NorVoltTime = 0;
	}
	
	if(HwprotectData.iniOk == 0)
	{
    DataFlowFace.FaultSta.all = 0;		
		Delay1ms.delay_ini = 0;
		DataFlowFace.RunState = Init;
	}	
}

uint16_t OTP_Compare(uint8_t level)
{
	uint8_t n,u = 0,d = 0,i = 0;
	float temp = 0;	
	n = level;
	for(i = 0;i < 7; i++)
	{
		temp = HwOtpStr.tempIn[i];		
		if(temp > OTP_SysProParaUp[n][i])
		{
			u |= 1 << i;
		}else if(temp < OTP_SysProParaDw[n][i])
		{
			d |= 1 << i;
		}
	}
	
	if(u)
	{
		n = 1;
	}else
	{
		n = 0;
		if(d == 0x7f)
		{
			n = 2;
		}
	}
	return n;
}
void OTP_Protection(void)
{	
	static uint16_t RelayNum = 0;
	uint8_t i = 0;
	if(DataFlowFace.RelaySta)
	{
		RelayNum++;
		if(RelayNum >= 400)
		{
			RelayNum = 400;
			DataFlowFace.RelayOld = 1;
		}		
	}else 
	{
		DataFlowFace.RelayOld = 0;
		RelayNum = 0;
	}	
	TIM1->CCR1 = 499;
	TIM4->CCR2 = 499;		
	TIM4->CCR3 = 499;		
	TIM4->CCR4 = 499;	
	
//	HwOtpStr.tempIn[0] = pfc_DataFlowFace.pfcHeatSinkTemp - 40.0f;
//	HwOtpStr.tempIn[1] = pfc_DataFlowFace.pfcInductanceTemp - 40.0f;
//  HwOtpStr.tempIn[2] = pfc_DataFlowFace.pfcTransformerTemp - 40.0f;
//	HwOtpStr.tempIn[3] = ADSample_Info.NTC0_AD_FIR - 40.0f;   //输出散热器
//	HwOtpStr.tempIn[4] = ADSample_Info.NTC3_AD_FIR - 40.0f;  //变压器温度
//	HwOtpStr.tempIn[5] = ADSample_Info.NTC1_AD_FIR - 40.0f;  //内部空气温度
//	HwOtpStr.tempIn[6] = ADSample_Info.NTC2_AD_FIR - 40.0f;  //输出端子温度	
//	if(HwOtpStr.ErrStaErr == 3)
//	{
//		i = OTP_Compare(3);
//		if(i==1)
//		{
//			HwOtpStr.Delayd = 0;
//		}else if(i == 2)
//		{
//			HwOtpStr.Delayu = 0;
//			HwOtpStr.Delayd++;
//			if(HwOtpStr.Delayd >= 1000)
//			{
//				//清除过温故障
//				HwOtpStr.ErrStaErr = 0;
//				HwOtpStr.Delayu = 0;
//				HwOtpStr.Delayd = 0;
//				DataFlowFace.FaultSta.bit.LLC_otp = 0;
//				DataFlowFace.FaultSta.bit.PFC_otp = 0;
//			}
//		}else 
//		{
//			HwOtpStr.Delayu = 0;
//			HwOtpStr.Delayd = 0;			
//		}
//	}else if(HwOtpStr.ErrStaErr == 2)
//	{
//		i = OTP_Compare(2);
//		if(i==1)
//		{
//			HwOtpStr.Delayd = 0;
//			HwOtpStr.Delayu++;
//			if(HwOtpStr.Delayu >= 1000)
//			{
//				HwOtpStr.ErrStaErr = 3;
//				HwOtpStr.Delayu = 0;
//				HwOtpStr.Delayd = 0;
//				DataFlowFace.FaultSta.bit.LLC_otp = 1;
//				DataFlowFace.FaultSta.bit.PFC_otp = 1;				
//			}
//		}else if(i == 2)
//		{
//			HwOtpStr.Delayu = 0;
//			HwOtpStr.Delayd++;
//			if(HwOtpStr.Delayd >= 1000)
//			{
//				HwOtpStr.ErrStaErr = 1;
//				HwOtpStr.Delayu = 0;
//				HwOtpStr.Delayd = 0;
//			}
//		}else 
//		{
//			HwOtpStr.Delayu = 0;
//			HwOtpStr.Delayd = 0;			
//		}		
//	}else if(HwOtpStr.ErrStaErr == 1)
//  {
//		i = OTP_Compare(1);
//		if(i==1)
//		{
//			HwOtpStr.Delayd = 0;
//			HwOtpStr.Delayu++;
//			if(HwOtpStr.Delayu >= 1000)
//			{
//				HwOtpStr.ErrStaErr = 2;
//				HwOtpStr.Delayu = 0;
//				HwOtpStr.Delayd = 0;
//			}
//		}else if(i == 2)
//		{
//			HwOtpStr.Delayu = 0;
//			HwOtpStr.Delayd++;
//			if(HwOtpStr.Delayd >= 1000)
//			{
//				HwOtpStr.ErrStaErr = 0;
//				HwOtpStr.Delayu = 0;
//				HwOtpStr.Delayd = 0;
//			}
//		}else 
//		{
//			HwOtpStr.Delayu = 0;
//			HwOtpStr.Delayd = 0;			
//		}
//	}else
//	{
//		i = OTP_Compare(0);
//		if(i==1)
//		{
//			HwOtpStr.Delayd = 0;
//			HwOtpStr.Delayu++;
//			if(HwOtpStr.Delayu >= 1000)
//			{
//				HwOtpStr.ErrStaErr = 1;
//				HwOtpStr.Delayu = 0;
//				HwOtpStr.Delayd = 0;
//			}
//		}else if(i == 2)
//		{
//			HwOtpStr.Delayu = 0;
//		}else 
//		{
//			HwOtpStr.Delayu = 0;
//			HwOtpStr.Delayd = 0;			
//		}	
//	}	
}



