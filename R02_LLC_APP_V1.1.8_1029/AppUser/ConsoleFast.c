
#include "HwConfig.h"

float testDuty = 0;
extern void PowerCtrHandle(void);
void HandleFast(void)
{
	static float vtemp = 0;
	float err = 0,temp = 0;	
	if(HAL_GPIO_ReadPin(PFC_OK_GPIO_Port, PFC_OK_Pin))DataFlowFace.PFC_ok = 1;
	else DataFlowFace.PFC_ok = 0;	
  SwOCP();  
	if(HwprotectData.OvFault == 0)
	{		
		if((Ctrl_interFace.CtrMode == OvLoadPFM)||(Ctrl_interFace.CtrMode == ConCurPWM))
		{
			err = ADSample_Info.iOut_Bat_adc;
			if(err > 1.0f)vtemp = ADSample_Info.vOut_Bat_adc;
			if(Ctrl_interFace.Curr_REF > 1.0f)
			{				
				temp = vtemp + 3.0f;
				if((ADSample_Info.vOut_Bat_adc >= temp)&&(vtemp > 30.0f))
				{
					HwprotectData.OvFault = 1;
					Ctrl_interFace.CtrMode = NoSelect;
					HwprotectData.OvTime = 0;
					DischargeOn();
					RelayOff();						
				}else if(Ctrl_interFace.Curr_REF > 5.0f)
				{
					if(ADSample_Info.iOut_Bat_adc < 1.0f)
					{
						HwprotectData.OvFault = 1;
						Ctrl_interFace.CtrMode = NoSelect;
						HwprotectData.OvTime = 0;
						DischargeOn();
						RelayOff();		
					}
				}				
			}else 
			{				
				vtemp = 0;				
			}
		}else vtemp = 0;
	}else
	{
		vtemp = 0;
		if(ADSample_Info.vOut_Bat_adc <= 58.0f)HwprotectData.OvTime++;
		else HwprotectData.OvTime = 0;
		if(HwprotectData.OvTime >= 140000)
		{
			HwprotectData.OvFault = 0;
			HwprotectData.OvTime = 0;
			Ctrl_interFace.Curr_REF = 0;
		}
	}
//	if(HwprotectData.OvFault == 0)
//	{		
//		if((Ctrl_interFace.CtrMode == OvLoadPFM)||(Ctrl_interFace.CtrMode == ConCurPWM))
//		{
//			err = ADSample_Info.iOut_Bat_adc;
//			if(err > 1.0f)vtemp = ADSample_Info.vOut_Bat_adc;
//			if(Ctrl_interFace.Curr_REF > 1.0f)
//			{				
//				temp = vtemp + 3.0f;
//				if((ADSample_Info.vOut_Bat_adc >= temp)&&(vtemp > 30.0f))
//				{
//					HwprotectData.OvFault = 1;
//					Ctrl_interFace.CtrMode = NoSelect;
//					HwprotectData.OvTime = 0;
//					DischargeOn();
//					RelayOff();						
//				}
//				if(Ctrl_interFace.Curr_REF > 6.0f)
//				{
//					if(ADSample_Info.iOut_Bat_adc < 2.0f)
//					{
//						HwprotectData.OvFault = 1;
//						Ctrl_interFace.CtrMode = NoSelect;
//						HwprotectData.OvTime = 0;
//						DischargeOn();
//						RelayOff();		
//					}
//				}				
//			}else 
//			{				
//				vtemp = 0;				
//			}
//		}else vtemp = 0;
//	}else
//	{
//		vtemp = 0;
//		if(ADSample_Info.vOut_Bat_adc <= 58.0f)HwprotectData.OvTime++;
//		else HwprotectData.OvTime = 0;
//		if(HwprotectData.OvTime >= 140000)
//		{
//			HwprotectData.OvFault = 0;
//			HwprotectData.OvTime = 0;
//			Ctrl_interFace.Curr_REF = 0;
//		}
//	}
	if(ADSample_Info.vOut_Bat_adc >= 62.0f)
	{
		DataFlowFace.FaultSta.bit.Out_ov = 1;	
	}
	if(DataFlowFace.FaultSta.all)
	{
		DischargeOn();
		//PFC关闭
		LLC_Disable();
		//PWM关闭
		PwmClose();
		//继电器关闭
		RelayOff();	
		//进入故障状态
  	DriverPwm.Plv = POW_MAX_PLV;
		Ctrl_interFace.Curr_REF = 0;
	}else if(DataFlowFace.PFC_ok == 0)
	{	
		DischargeOn();
		LLC_Enable();
		PwmClose();		
		Ctrl_interFace.CtrMode = NoSelect;
		DataFlowFace.OldState = Init;
		Ctrl_interFace.Curr_REF = 0;
	}else 
	{
		LLC_Enable();
		PowerCtrHandle();	
	}
  SHRTIMERdrive();
}


/*
** ===================================================================
**     Funtion Name :void FastProtection(void)
**     Description :  保护函数，输出短路保护和过流保护
**     Parameters  :无
**     Returns     :无
** ===================================================================
*/
void SwOCP(void)
{	
//	static uint16_t ErrNum1 = 0;
  if(DataFlowFace.RelayOld)
	{
    if(ADSample_Info.vOut_Bat_adc < OUTPUT_SHORT)
		{
			if(ADSample_Info.iOut_Bat_adc > 10.0f)
			{
				DataFlowFace.FaultSta.bit.Out_short = 1;		
			}			
		}			
		
		if(ADSample_Info.iOut_Bat_adc > 42.0f) 
		{
			DataFlowFace.FaultSta.bit.Out_oc = 1;
		}	
  }
}

void SHRTIMERdrive(void)
{
	uint16_t preA,pre,half,temp,bemp,duty;	
	float time1;
	static uint16_t delaySr = 0;
	preA = (uint16_t)(SHRTIMER_PLV / DriverPwm.Plv);
	pre = preA & 0xfffc;
	half = pre >> 1;		

	if(Ctrl_interFace.CtrMode == OvLoadPFM)  //psm
	{
		DriverPwm.SynDrv = 0;
	}else 
	{
		temp = pre >> 2;
		bemp = pre - temp;
		time1 = ((float)half) * DriverPwm.Duty;		
		duty = (uint16_t)(time1);	
	}	
	
	hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_C].PERxR = pre;
	hhrtim1.Instance->sMasterRegs.MPER = pre;
	hhrtim1.Instance->sMasterRegs.MCMP1R = pre;
  if(Ctrl_interFace.CtrMode == OvLoadPFM)
	{		
		hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_C].CMP1xR = LLC_DEADTIME;
		hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_C].CMP2xR = half - LLC_DEADTIME;	
		hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_C].CMP3xR = half + LLC_DEADTIME;
		hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_C].CMP4xR = pre - LLC_DEADTIME;		
	}else 
	{			
		hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_C].CMP1xR = temp - duty;
		hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_C].CMP2xR = temp + duty;		
		hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_C].CMP3xR = bemp - duty;
		hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_C].CMP4xR = bemp + duty;			
  }	
	
	if(DriverPwm.DrvH){DrvH_On();}
	else {DrvH_Off();}
	if(DriverPwm.DrvL){DrvL_On();}
	else {DrvL_Off();}		
}

void LLC_Disable(void)
{
	HAL_GPIO_WritePin(LLC_OUT_GPIO_Port, LLC_OUT_Pin, GPIO_PIN_RESET);
	DataFlowFace.LLC_en = 0;
}
void LLC_Enable(void)
{
	HAL_GPIO_WritePin(LLC_OUT_GPIO_Port, LLC_OUT_Pin, GPIO_PIN_SET);
	DataFlowFace.LLC_en = 1;
}
void RelayOn(void)
{
	HAL_GPIO_WritePin(RELAY_CTAL_GPIO_Port, RELAY_CTAL_Pin, GPIO_PIN_SET);
	DataFlowFace.RelaySta = 1;
}
void RelayOff(void)
{
	HAL_GPIO_WritePin(RELAY_CTAL_GPIO_Port, RELAY_CTAL_Pin, GPIO_PIN_RESET);
	DataFlowFace.RelaySta = 0;
}

void DischargeOn(void)
{
	HAL_GPIO_WritePin(VOUT_FD_GPIO_Port, VOUT_FD_Pin, GPIO_PIN_SET);
	DataFlowFace.DisCharge = 1;
}
void DischargeOff(void)
{
	HAL_GPIO_WritePin(VOUT_FD_GPIO_Port, VOUT_FD_Pin, GPIO_PIN_RESET);
	DataFlowFace.DisCharge = 0;
}

