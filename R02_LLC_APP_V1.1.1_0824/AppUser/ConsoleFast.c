
#include "HwConfig.h"

float testDuty = 0;
void HandleFast(void)
{
	static float vtemp = 0;
	float adc_temp,err = 0,temp = 0;	
	if(HAL_GPIO_ReadPin(PFC_OK_GPIO_Port, PFC_OK_Pin))DataFlowFace.PFC_ok = 1;
	else DataFlowFace.PFC_ok = 0;
	SampleLpfHandle();
  	SwOCP();  
//	HwProtect();
	if(HwprotectData.OvFault == 0)
	{		
		if((Ctrl_interFace.CtrMode == OvLoadPFM)||(Ctrl_interFace.CtrMode == ConCurPWM)||(Ctrl_interFace.CtrMode == PwmHold))
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
	if(ADSample_Info.vOut_Bat_adc >= 62.0f)
	{
		DataFlowFace.FaultSta.bit.Out_ov = 1;	
	}
	if(DataFlowFace.FaultSta.all)
	{
		DischargeOn();
		LLC_Disable();
		RelayOff();	
		Ctrl_interFace.Curr_REF = 0;
		Ctrl_interFace.CtrMode = NoSelect;
		PowerCtrHandle();
	}else if(DataFlowFace.PFC_ok == 0)
	{	
		DischargeOn();
		LLC_Enable();
		Ctrl_interFace.CtrMode = NoSelect;
		DataFlowFace.OldState = Init;
		Ctrl_interFace.Curr_REF = 0;
		PowerCtrHandle();
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
**     Description :  ???????????????��?????????????
**     Parameters  :??
**     Returns     :??
** ===================================================================
*/
void SwOCP(void)
{	
	static uint16_t ErrNum1 = 0;
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

