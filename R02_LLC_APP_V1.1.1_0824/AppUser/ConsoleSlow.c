
#include "HwConfig.h"
#ifndef PLECS_DLL
#include "UpgradeFeatureConfig.h"
#if OTA_UPGRADE_ENABLE
#include "OtaGw.h"
#endif
#include "CanFdTxQueue.h"
#endif

DelayMS_t   Delay1ms;
DataFlow_t  DataFlowFace;
Driver_t    DriverPwm;
void HAL_IncTick(void)
{
#ifndef PLECS_DLL
	static uint16_t uartTime = 0;

	uwTick += uwTickFreq;
	Delay1ms.delay_Slow++;	
	Delay1ms.powerOn++;	
	if(Delay1ms.powerOn >= 2000)
	{
		Delay1ms.powerOn = 2000;
	}else if(Delay1ms.powerOn == 1900)
	{
	  HAL_COMP_Start(&hcomp1); 
	  HAL_COMP_Start(&hcomp2);			
	  HAL_COMP_Start(&hcomp3);			
	}
	if(Delay1ms.delay_Slow >= 5)
	{
		Delay1ms.delay_Slow = 0;
		runMainStateMachine();
		StateM();
	}else if(Delay1ms.delay_Slow == 1)
	{
    OTP_Protection(); 
	}else if(Delay1ms.delay_Slow == 2)
	{
#if OTA_UPGRADE_ENABLE
		OtaGw_HandoffTask();
#endif
	}else if(Delay1ms.delay_Slow == 3)
	{
		uartTime++;
		if(uartTime >= 20)
		{
			UartCom();
			uartTime = 0;
			SysReset();
		}
	}else if(Delay1ms.delay_Slow == 4)
	{
    LEDShow(); 
	}	
  CanFd_SysTickCanService();
	RelayProtect();	
	AuxProtect();
#endif
}

void LEDShow(void)
{
#ifndef PLECS_DLL
	static uint16_t fast,delay = 0;
	delay++;
	if(delay >= 320)delay = 0;
	fast++;
	if(fast >= 120)fast = 0;
	static uint8_t last_led = 0;
	
	if(last_led != gSys_State.SysSta){
		last_led = gSys_State.SysSta;
		HAL_GPIO_WritePin(LED_OUT2_GPIO_Port,LED_OUT2_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(LED_OUT1_GPIO_Port,LED_OUT1_Pin, GPIO_PIN_RESET);
   		HAL_GPIO_WritePin(LED_OUT3_GPIO_Port,LED_OUT3_Pin, GPIO_PIN_RESET);
		return;
	}
	
	switch(gSys_State.SysSta)
	{
		case  Init :
		{			
			if(delay >= 160)
			{
				HAL_GPIO_WritePin(LED_OUT2_GPIO_Port,LED_OUT2_Pin, GPIO_PIN_SET); 
			}else 
			{
				HAL_GPIO_WritePin(LED_OUT2_GPIO_Port,LED_OUT2_Pin, GPIO_PIN_RESET); 
			}
		  HAL_GPIO_WritePin(LED_OUT1_GPIO_Port,LED_OUT1_Pin, GPIO_PIN_RESET);
   		HAL_GPIO_WritePin(LED_OUT3_GPIO_Port,LED_OUT3_Pin, GPIO_PIN_RESET);

		}
		break;
		case  Wakeup :
		{
//			if(delay >= 160)
//			{
//				HAL_GPIO_WritePin(LED_OUT3_GPIO_Port,LED_OUT3_Pin, GPIO_PIN_SET);	
//			}else{
//				HAL_GPIO_WritePin(LED_OUT3_GPIO_Port,LED_OUT3_Pin, GPIO_PIN_RESET); 
//			}
//		HAL_GPIO_WritePin(LED_OUT1_GPIO_Port,LED_OUT1_Pin, GPIO_PIN_RESET);
//   		HAL_GPIO_WritePin(LED_OUT2_GPIO_Port,LED_OUT2_Pin, GPIO_PIN_RESET); 	

			if(delay >= 160)
			{
				HAL_GPIO_WritePin(LED_OUT2_GPIO_Port,LED_OUT2_Pin, GPIO_PIN_SET); 
				 HAL_GPIO_WritePin(LED_OUT1_GPIO_Port,LED_OUT1_Pin, GPIO_PIN_SET);
			}else{
				HAL_GPIO_WritePin(LED_OUT2_GPIO_Port,LED_OUT2_Pin, GPIO_PIN_RESET); 
				 HAL_GPIO_WritePin(LED_OUT1_GPIO_Port,LED_OUT1_Pin, GPIO_PIN_RESET);
			}
     
   		HAL_GPIO_WritePin(LED_OUT3_GPIO_Port,LED_OUT3_Pin, GPIO_PIN_RESET);				
		}
		break;
		case  Stadby:
		{
			if(fast >= 60)
			{
				HAL_GPIO_WritePin(LED_OUT2_GPIO_Port,LED_OUT2_Pin, GPIO_PIN_SET); 
			}else HAL_GPIO_WritePin(LED_OUT2_GPIO_Port,LED_OUT2_Pin, GPIO_PIN_RESET); 
			
		  HAL_GPIO_WritePin(LED_OUT1_GPIO_Port,LED_OUT1_Pin, GPIO_PIN_RESET);
   		HAL_GPIO_WritePin(LED_OUT3_GPIO_Port,LED_OUT3_Pin, GPIO_PIN_RESET);
	
		}
		break;
		case  Charging :
		{
//			HAL_GPIO_WritePin(LED_OUT2_GPIO_Port,LED_OUT2_Pin, GPIO_PIN_RESET);
//		HAL_GPIO_WritePin(LED_OUT1_GPIO_Port,LED_OUT1_Pin, GPIO_PIN_RESET);
//   		HAL_GPIO_WritePin(LED_OUT3_GPIO_Port,LED_OUT3_Pin, GPIO_PIN_SET);		
			HAL_GPIO_WritePin(LED_OUT2_GPIO_Port,LED_OUT2_Pin, GPIO_PIN_SET);
		  HAL_GPIO_WritePin(LED_OUT1_GPIO_Port,LED_OUT1_Pin, GPIO_PIN_SET);
   		HAL_GPIO_WritePin(LED_OUT3_GPIO_Port,LED_OUT3_Pin, GPIO_PIN_RESET);				
		}
		break;
		case FullCharged:
		{
			HAL_GPIO_WritePin(LED_OUT2_GPIO_Port,LED_OUT2_Pin, GPIO_PIN_SET); 
 
		  HAL_GPIO_WritePin(LED_OUT1_GPIO_Port,LED_OUT1_Pin, GPIO_PIN_RESET);
   		HAL_GPIO_WritePin(LED_OUT3_GPIO_Port,LED_OUT3_Pin, GPIO_PIN_RESET);			
		}
		break;
		case  Fault :
		{
			HAL_GPIO_WritePin(LED_OUT2_GPIO_Port,LED_OUT2_Pin, GPIO_PIN_RESET); 
 
      HAL_GPIO_WritePin(LED_OUT1_GPIO_Port,LED_OUT1_Pin, GPIO_PIN_SET);	//red
   		HAL_GPIO_WritePin(LED_OUT3_GPIO_Port,LED_OUT3_Pin, GPIO_PIN_RESET);					
		}
		break;
		default:break;
	}
#endif
}

void UartCom(void)
{
#ifndef PLECS_DLL
	Send_to_Pfc_info();
	overTime_pfc();
#endif
}
void interrupt_ADC1(void)
{
	ADC0_Sample();
	HandleFast();		
}
void interrupt_ADC2(void)
{
	ADC2_Sample();
}
void StateM(void)
{
	switch(DataFlowFace.RunState)
	{
		case  Init :StateMInit();
		break;
		case  Wakeup : if(Delay1ms.delay_ini >= 100)ConVoWakeup();
		break;
		case  Stadby:if(Delay1ms.delay_ini >= 100)StandBy();
		break;
		case  Charging :if(Delay1ms.delay_ini >= 100)ChargeOn();
		break;
		case FullCharged:if(Delay1ms.delay_ini >= 100)ChargeFull();
		break;
		case  Fault :StateMErr();
		break;
		default:break;
	}
}

void StateMInit(void)
{
  starLowIni();
#ifndef PLECS_DLL
	if(HAL_GPIO_ReadPin(PFC_OK_GPIO_Port, PFC_OK_Pin))DataFlowFace.PFC_ok = 1;
	else DataFlowFace.PFC_ok = 0;	
#endif
	Ctrl_interFace.CtrMode = NoSelect;
	PowerIniPidVar();

	DataFlowFace.OldState  = DataFlowFace.RunState;
	DischargeOn();

	if(DataFlowFace.PFC_ok)
	{
	  Delay1ms.delay_ini++;	
		if(Delay1ms.delay_ini >= 100)
		{
      Delay1ms.delay_ini = 100;
		}
  }else Delay1ms.delay_ini = 0;	
}

/** ===================================================================
**     Funtion Name : void StateMWait(void)
**     Description :  1.Ê¹ÄÜllc_en 2.µÈ´ý pfcok 3.µÈ´ýµç³ØµçÑ¹Õý³£
**     Parameters  :
**     Returns     :
** ===================================================================*/
 
void ConVoWakeup(void)
{
	static uint16_t cont = 0;
	DataFlowFace.outSetVolt = CON_VOLT_OUT;
	if(DataFlowFace.OldState  != DataFlowFace.RunState)
	{
		if((ADSample_Info.vOut_Rly_adc < 28.f) && (ADSample_Info.vOut_Bat_adc < 5.0f) && (DataFlowFace.RelaySta == 0))
		{
			cont++;
			if(cont >= 3)
			{			
				DataFlowFace.OldState  = DataFlowFace.RunState;
				DischargeOff();
				RelayOff();		
				Ctrl_interFace.preOK = 0;
				DataFlowFace.outSetVolt = CON_VOLT_OUT;			
				memset((uint8_t*)&vParamPid, 0, sizeof(pi_str));
				Ctrl_interFace.CtrMode = SoftStar;	
				cont = 0;
			}
		}else
		{
			cont = 0;
			DischargeOn();
			RelayOff();		
		}	
	}else 
	{
		cont = 0;		
	}
}

/** ===================================================================
**     Funtion Name : void StateRelay(void)
**     Description :  1.PWMÆô¶¯ 2.Êä³öµçÑ¹==µç³ØµçÑ¹Ê±£¬ÎüºÏ¼ÌµçÆ÷  3.PWMÍ£Ö¹ µÈ´ýCAN³äµçÖ¸Áî
**     Parameters  :
**     Returns     :
** ===================================================================*/

void StandBy(void)
{
	if(DataFlowFace.OldState  != DataFlowFace.RunState)
	{
		DataFlowFace.OldState  = DataFlowFace.RunState;
		//²ÎÊý³õÊ¼»¯		
		DischargeOn();
		RelayOff();		
		DataFlowFace.outSetVolt = 30;	
		Ctrl_interFace.CtrMode = NoSelect;			
	}	
}

void ChargeOn(void)
{
  static uint16_t CvCc = 0,cont = 0;
	float temp = 0;
	if(gSys_State.xp_HandShake == 1)gSys_State.xp_CVmode = 1;
	if(HwprotectData.OvFault)DataFlowFace.OldState = Init;
	if(DataFlowFace.OldState  != DataFlowFace.RunState)temp = 1;
	else 
	{
		if((gSys_State.xp_CVmode != CvCc) && gSys_State.xp_CVmode) temp =1;
	}
	
	if(temp && (HwprotectData.OvFault == 0))
	{		
		Ctrl_interFace.CtrMode = NoSelect;
		if(ADSample_Info.iOut_Bat_FIR < 1.0f)
		{
			if(gSys_State.xp_CVmode == 1) //CC
			{
				if(gSys_State.xp_HandShake == 1)temp = gCAN_DATA.sBMS2_ChrgMsg0_Data.BMS2_ChrgReqVolt * 0.1f;
				else temp = gCAN_DATA.sPMS_ChrgMsg0_Data.PMS_ChrgReqVolt * 0.1f;
				if((ADSample_Info.vOut_Rly_adc < ADSample_Info.vOut_Bat_adc - 2.0f)&&(temp >= 30.0f))
				{			
					if((ADSample_Info.vOut_Bat_adc >= 29.0f)&&(ADSample_Info.vOut_Bat_adc < 60.0f))
					{			
						DischargeOff();
						RelayOff();		
						cont++;
						if(cont >= 40) 
						{							
							Ctrl_interFace.Curr_REF = 0;
							DataFlowFace.OldState  = DataFlowFace.RunState;
							Ctrl_interFace.preOK = 0;
							Ctrl_interFace.litate = 0;					
							Ctrl_interFace.Volt_Ref = ADSample_Info.vOut_Bat_adc + 2.0f;
							Ctrl_interFace.CtrMode = BmsStar;	
							CvCc = gSys_State.xp_CVmode;
							cont = 0;
							iParamPid.oldout = 0;
						}
					}else 
					{
						cont = 0;
						DischargeOn();
						RelayOff();		
					}
				}else
				{
					cont = 0;
					DischargeOn();
					RelayOff();						
				}			
			}else if(gSys_State.xp_CVmode == 2) //CV
			{
				if(gSys_State.xp_HandShake == 1)temp = gCAN_DATA.sBMS2_ChrgMsg0_Data.BMS2_ChrgReqVolt * 0.1f;
				else temp = gCAN_DATA.sPMS_ChrgMsg0_Data.PMS_ChrgReqVolt * 0.1f;
				if((ADSample_Info.vOut_Rly_adc < temp - 2.0f)&&(temp >= 30.0f))
				{			
					DischargeOff();
					RelayOff();		
					cont++;
					if(cont >= 30) 
					{
						HwprotectData.OvFault = 0;
						DataFlowFace.OldState  = DataFlowFace.RunState;
						Ctrl_interFace.preOK = 0;
						Ctrl_interFace.litate = 0;
						Ctrl_interFace.CtrMode = SoftCurSt;	
						CvCc = gSys_State.xp_CVmode;
						cont = 0;
					}
				}else 
				{
					cont = 0;
					DischargeOn();
					RelayOff();		
				}			
			}
	  }
	}

	
}

void ChargeFull(void)
{
	if(DataFlowFace.OldState  != DataFlowFace.RunState)
	{
		DataFlowFace.OldState  = DataFlowFace.RunState;
		DischargeOn();
		RelayOff();		
    Ctrl_interFace.CtrMode = NoSelect;
		DataFlowFace.outSetVolt = 30;				
	}		
	//¹Ø±Õ¼ÌµçÆ÷
	//¹Ø±ÕMOS
}
void StateMErr(void)
{
  static uint16_t MerrCnt = 0;
    DischargeOn();
	LLC_Disable();
	RelayOff();	
	
	Ctrl_interFace.CtrMode = NoSelect;	
	//ÈôËùÓÐ¹ÊÕÏÒÑ»Ö¸´£¬ÇÒµÈ´ý´óÓÚ1S	
	if(DataFlowFace.FaultSta.all == 0)
	{	
		MerrCnt++;
		if(MerrCnt >= 200)
		{
			//Ìø×ªÖÁ¿ÕÏÐµÈ´ý×´Ì¬,ÖØÐÂÆô¶¯
			DataFlowFace.RunState  = Init;
			MerrCnt = 0;
	  }
  }else
	{
		MerrCnt = 0;
	}
}


void starLowIni(void)
{
	memset((uint8_t*)&DriverPwm,0,sizeof(Driver_t));
	PwmClose();
}

void PwmClose(void)
{
	DriverPwm.DrvH = 0;
	DriverPwm.DrvL = 0;
	DriverPwm.SynDrv = 0;
}

//void SHRTIMERdrive(void)
//{
//	uint16_t preA,pre,half,temp,bemp,duty,cnt;	
//	float time1;
//	preA = (uint16_t)(SHRTIMER_PLV / DriverPwm.Plv);
//	pre = preA & 0xfffc;
//	half = pre >> 1;		
//	if(ADSample_Info.iOut_Bat_adc < 4.0f)DriverPwm.Sr_Dtime = 0;
//	time1 = half - (LLC_DEADTIME * 2.0f) -  DriverPwm.Sr_Atime - DriverPwm.Sr_Btime;
//	DriverPwm.Sr_Dtime = DriverPwm.Sr_Dtime * 0.999f + time1 * 0.001f;
//	if(DriverPwm.Sr_Dtime > time1)DriverPwm.Sr_Dtime = time1;	
//	DriverPwm.DrvDtime = (uint16_t)DriverPwm.Sr_Dtime;
//	temp = LLC_DEADTIME + DriverPwm.Sr_Atime;
//	bemp = temp + LLC_DEADTIME + DriverPwm.DrvDtime;	
//	cnt = hhrtim1.Instance->sMasterRegs.MCNTR;	
//	while( pre < cnt + 200)
//	{
//		cnt = hhrtim1.Instance->sMasterRegs.MCNTR;
//	}
//	if(DriverPwm.DrvH){DrvH_On();}
//	else {DrvH_Off();}
//	if(DriverPwm.DrvL){DrvL_On();}
//	else {DrvL_Off();}

//	if(DriverPwm.SynDrv){SarH_On();SarL_On();}
//	else {SarH_Off();SarL_Off();DriverPwm.Sr_Dtime = 0;}		
//	hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_C].PERxR = pre;
//	hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_D].PERxR = pre;
//	hhrtim1.Instance->sMasterRegs.MPER = pre;
//	hhrtim1.Instance->sMasterRegs.MCMP1R = pre;
//  if(Ctrl_interFace.CtrMode == OvLoadPFM)
//	{		
//		hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_C].CMP1xR = LLC_DEADTIME;
//		hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_C].CMP2xR = half - LLC_DEADTIME;	
//		hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_C].CMP3xR = half + LLC_DEADTIME;
//		hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_C].CMP4xR = pre - LLC_DEADTIME;		
//		hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_D].CMP1xR = half + temp;			
//		hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_D].CMP2xR = half + bemp;			
//		hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_D].CMP3xR = temp;			
//		hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_D].CMP4xR = bemp;	   		
//	}else 
//	{
//		temp = pre >> 2;
//		bemp = pre - temp;
//		duty = (uint16_t)(DriverPwm.Duty * half);			
//		hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_C].CMP1xR = temp - duty;
//		hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_C].CMP2xR = temp + duty;	
//		hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_C].CMP3xR = bemp - duty;
//		hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_C].CMP4xR = bemp + duty;		
//	}
//}
void SHRTIMERdrive(void)
{
	uint16_t preA,pre,half,temp,bemp,duty,cnt;	
	float time1;
	preA = (uint16_t)(SHRTIMER_PLV / DriverPwm.Plv);
	pre = preA & 0xfffc;
	half = pre >> 1;		

	if(Ctrl_interFace.CtrMode == OvLoadPFM)  //psm
	{
	  time1 = half - (LLC_DEADTIME * 2.0f) -  DriverPwm.Sr_Atime - DriverPwm.Sr_Btime;
	  DriverPwm.Sr_Dtime = DriverPwm.Sr_Dtime * 0.999f + time1 * 0.001f;
	  if(DriverPwm.Sr_Dtime > time1)DriverPwm.Sr_Dtime = time1;	
	  DriverPwm.DrvDtime = (uint16_t)DriverPwm.Sr_Dtime;
	  temp = LLC_DEADTIME + DriverPwm.Sr_Atime;
	  bemp = temp + DriverPwm.DrvDtime;			
	}else 
	{
		temp = pre >> 2;
		bemp = pre - temp;
		time1 = ((float)half) * DriverPwm.Duty;		
		duty = (uint16_t)(time1);	
    if((Ctrl_interFace.CtrMode == ConCurPWM)||(Ctrl_interFace.CtrMode == PwmHold))
		{
			if(DriverPwm.SynDrv && DriverPwm.DrvH)
			{
        DriverPwm.Sr_Atime = 200.0f;	//0.68ns * 200		
				time1 = time1 * 2.0f - DriverPwm.Sr_Atime;  //同步管开通时间
				if(time1 < 150.0f)
				{
					time1 = 0;
					DriverPwm.SynDrv = 0;
					DriverPwm.Sr_Dtime = 0;
				}				
			}else
			{
				time1 = 0;
				DriverPwm.SynDrv = 0;
				DriverPwm.Sr_Dtime = 0;
			}
		}else
		{
			time1 = 0;
			DriverPwm.SynDrv = 0;
			DriverPwm.Sr_Dtime = 0;	
		}		
		DriverPwm.Sr_Dtime = DriverPwm.Sr_Dtime * 0.999f + time1 * 0.001f;
		if(DriverPwm.Sr_Dtime > time1)DriverPwm.Sr_Dtime = time1;	
		DriverPwm.DrvDtime = (uint16_t)DriverPwm.Sr_Dtime;		
	}	
//	cnt = hhrtim1.Instance->sMasterRegs.MCNTR;	
//	while( pre < cnt + 200)
//	{
//		cnt = hhrtim1.Instance->sMasterRegs.MCNTR;
//	}
#ifndef PLECS_DLL
	hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_C].PERxR = pre;
	hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_D].PERxR = pre;
	hhrtim1.Instance->sMasterRegs.MPER = pre;
	hhrtim1.Instance->sMasterRegs.MCMP1R = pre;
  if(Ctrl_interFace.CtrMode == OvLoadPFM)
	{		
		hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_C].CMP1xR = LLC_DEADTIME;
		hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_C].CMP2xR = half - LLC_DEADTIME;	
		hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_C].CMP3xR = half + LLC_DEADTIME;
		hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_C].CMP4xR = pre - LLC_DEADTIME;	
		
		hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_D].CMP1xR = half + temp;			
		hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_D].CMP2xR = half + bemp;			
		hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_D].CMP3xR = temp;			
		hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_D].CMP4xR = bemp;	   	
		if(DriverPwm.SynDrv == 0)
		{
			hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_D].CMP1xR = half + LLC_DEADTIME + 200;			
			hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_D].CMP2xR = half + LLC_DEADTIME + 210;			
			hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_D].CMP3xR = LLC_DEADTIME + 200;			
			hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_D].CMP4xR = LLC_DEADTIME + 210;				
		}else
		{
			hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_D].CMP1xR = half + temp;			
			hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_D].CMP2xR = half + bemp;			
			hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_D].CMP3xR = temp;			
			hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_D].CMP4xR = bemp;	
		}
		
	}else 
	{			
		hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_C].CMP1xR = temp - duty;
		hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_C].CMP2xR = temp + duty;		
		
		hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_C].CMP3xR = bemp - duty;
		hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_C].CMP4xR = bemp + duty;		
		
		if(DriverPwm.SynDrv == 0)
		{
			hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_D].CMP1xR = bemp - 1;
			hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_D].CMP2xR = bemp + 1;			
			hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_D].CMP3xR = temp - 1;			
			hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_D].CMP4xR = temp + 1;				
		}else
		{
			time1 = bemp - duty + DriverPwm.Sr_Atime;
			hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_D].CMP1xR = time1;
			hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_D].CMP2xR = bemp + duty;			
			time1 = temp - duty + DriverPwm.Sr_Atime;
			hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_D].CMP3xR = time1;				
			hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_D].CMP4xR = time1 + DriverPwm.DrvDtime;	
		}	
	}	
	
	if(DriverPwm.DrvH){DrvH_On();}
	else {DrvH_Off();}
	if(DriverPwm.DrvL){DrvL_On();}
	else {DrvL_Off();}		
	if(DriverPwm.SynDrv){SarH_On();SarL_On();}
	else {SarH_Off();SarL_Off();DriverPwm.Sr_Dtime = 0;}
#else
	if(DriverPwm.SynDrv == 0) DriverPwm.Sr_Dtime = 0;
#endif
	testDuty = DriverPwm.SynDrv;
}

void LLC_Disable(void)
{
#ifndef PLECS_DLL
	HAL_GPIO_WritePin(LLC_OUT_GPIO_Port, LLC_OUT_Pin, GPIO_PIN_RESET);
#endif
	DataFlowFace.LLC_en = 0;
}
void LLC_Enable(void)
{
#ifndef PLECS_DLL
	HAL_GPIO_WritePin(LLC_OUT_GPIO_Port, LLC_OUT_Pin, GPIO_PIN_SET);
#endif
	DataFlowFace.LLC_en = 1;
}
void RelayOn(void)
{
#ifndef PLECS_DLL
	HAL_GPIO_WritePin(RELAY_CTAL_GPIO_Port, RELAY_CTAL_Pin, GPIO_PIN_SET);
#endif
	DataFlowFace.RelaySta = 1;
}
void RelayOff(void)
{
#ifndef PLECS_DLL
	HAL_GPIO_WritePin(RELAY_CTAL_GPIO_Port, RELAY_CTAL_Pin, GPIO_PIN_RESET);
#endif
	DataFlowFace.RelaySta = 0;
}

void DischargeOn(void)
{
#ifndef PLECS_DLL
	HAL_GPIO_WritePin(VOUT_FD_GPIO_Port, VOUT_FD_Pin, GPIO_PIN_SET);
#endif
	DataFlowFace.DisCharge = 1;
}
void DischargeOff(void)
{
#ifndef PLECS_DLL
	HAL_GPIO_WritePin(VOUT_FD_GPIO_Port, VOUT_FD_Pin, GPIO_PIN_RESET);
#endif
	DataFlowFace.DisCharge = 0;
}
const float OutVconst[34]=
{
	30.2941f,31.80875f,33.12615f,34.2729f,35.27265f,36.1463f,36.91175f,37.58435f,38.17725f,38.7016f,39.16675f,
	39.5808f,39.95045f,40.2816f,40.5791f,40.84715f,41.0894f,41.3089f,41.5083f,41.6899f,41.85575f,42.93645f,
	43.4659f,43.7618f,43.94315f,44.0621f,44.1442f,44.2032f,44.247f,44.28045f,44.3065f,44.3272f,44.3439f,44.3576f
};

float JudgeMode(float inx)
{
	uint8_t y = 0;
	float k = 0;
	if(inx < 3.0f)
	{
		y = (uint8_t)(inx * 10.0f + 0.5f);
		if(y < 10)y = 10;
		k = OutVconst[y - 10];		
	}else if(inx <= 16.0f)
	{
		inx += 0.5f;
		y = (uint8_t)inx;
		y += 17;
		k = OutVconst[y];
		
	}else
	{
		k = OutVconst[33];
	}
	return k;
}

