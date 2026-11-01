
#include "mathR02.h"

Power_interFace_t Ctrl_interFace;
pi_str piStructInter,vParamPid,iParamPid;
filter_str kFilterOne,kFilterTwo;
void PowerCtrHandle(void)
{	
	static uint16_t time = 0;
	float err,out = 0;
	//状态机
	switch(Ctrl_interFace.CtrMode)
	{
		case  NoSelect :
		{
//			HAL_COMP_Stop(&hcomp2);	
			DischargeOff();
			RelayOff();
			PowerIniPidVar();
			Ctrl_interFace.preOK = 0;
			DriverPwm.Plv = POW_MAX_PLV;
			DriverPwm.DrvH = 0;
			DriverPwm.DrvL = 0;	
			DriverPwm.SynDrv = 0;
			DriverPwm.Duty = 0.02f;				
			time = 0;
		}
		break;
		case  SoftStar :
		{      
			SoftStart();
		}
		break;		
		case BmsStar :
		{
			SoftCurStart();
		}
		break;
		case SoftCurSt:
		{      
			ConVoltCurHandle();
			time = 0;
		}
		break;		
		case ConVolt :  
		{      
			ConVoltHandle();
		}
		break;
		case  ConCurPWM :
		{			
			if(Ctrl_interFace.psmSwitch == 2)
			{				
				time = 0;								
				DriverPwm.DrvH = 0;
				DriverPwm.DrvL = 1;					
				DriverPwm.Duty = 0.05f;
				DriverPwm.SynDrv = 0;       
        Ctrl_interFace.Curr_REF = 0;				
				Ctrl_interFace.CtrMode = Transition;	
				
				Ctrl_interFace.flvOut = POW_MAX_PLV;
				DriverPwm.Plv = Ctrl_interFace.flvOut;
				piStructInter.integral = POW_MAX_PLV * 0.00002f;
				piStructInter.fir = 0;						
			

			}else	if(Ctrl_interFace.psmSwitch == 1)
			{		
				DriverPwm.Plv = 80000.0f;					
				DriverPwm.DrvH = 0;
				DriverPwm.DrvL = 1;					
				DriverPwm.Duty = 0.04f;
				DriverPwm.SynDrv = 0;	
				if(ADSample_Info.iOut_Bat_FIR < 1.0f)
				{
					time++;
					if(time >= 40)
					{
						Ctrl_interFace.psmSwitch = 2;
	          time = 0;
					}
				}					
			}else
			{
				if(ADSample_Info.vOut_Bat_adc >= 40.0f)
				{
					Ctrl_interFace.psmSwitch = 1;
					time = 0;
					DriverPwm.Plv = 80000.0f;					
					DriverPwm.DrvH = 0;
					DriverPwm.DrvL = 1;					
					DriverPwm.Duty = 0.02f;
					DriverPwm.SynDrv = 0;	
          Ctrl_interFace.Curr_REF = 0;						
				}
				else 
				{
					ConCurrHandle();					
				}
			}			
		}
		break;
		case  Transition:
			if(gSys_State.xp_HandShake == 1)Ctrl_interFace.CurrentMax = gCAN_DATA.sBMS2_ChrgMsg0_Data.BMS2_ChrgReqCur * 0.1f;
			else if(gSys_State.xp_HandShake == 2)Ctrl_interFace.CurrentMax = gCAN_DATA.sPMS_ChrgMsg0_Data.PMS_ChrgReqCur * 0.1f;				
			if(Ctrl_interFace.CurrentMax > 0.5f)
			{
				time++;
				if(time >= 400)
				{
					Ctrl_interFace.flvOut = POW_MAX_PLV;
					piStructInter.integral = POW_MAX_PLV * 0.00002f;
					piStructInter.fir = 0;
					time = 0;				
					Ctrl_interFace.CtrMode = OvLoadPFM;	
				}
				DriverPwm.Plv = POW_MAX_PLV;					
				DriverPwm.DrvH = 1;
				DriverPwm.DrvL = 1;		
        DriverPwm.SynDrv = 0;					
				DriverPwm.Duty = time * 0.001f + 0.05f;
				Ctrl_interFace.Curr_REF = 0;	
			}else
			{
				DriverPwm.DrvH = 0;
				DriverPwm.DrvL = 0;		
				DriverPwm.SynDrv = 0;		
				Ctrl_interFace.flvOut = POW_MAX_PLV;
				piStructInter.integral = POW_MAX_PLV * 0.00002f;
				piStructInter.fir = 0;						
				Ctrl_interFace.Curr_REF = 0;	
			}			
		break;
		case  OvLoadPFM :
		{	
			if(gSys_State.xp_HandShake == 1)Ctrl_interFace.CurrentMax = gCAN_DATA.sBMS2_ChrgMsg0_Data.BMS2_ChrgReqCur * 0.1f;
			else if(gSys_State.xp_HandShake == 2)Ctrl_interFace.CurrentMax = gCAN_DATA.sPMS_ChrgMsg0_Data.PMS_ChrgReqCur * 0.1f;			
			if((gSys_State.xp_HandShake == 0)||(ADSample_Info.vOut_Bat_adc < 39.0f))
			{
				DriverPwm.DrvH = 0;
				DriverPwm.DrvL = 0;		
				DriverPwm.SynDrv = 0;		
				Ctrl_interFace.flvOut = POW_MAX_PLV;
				piStructInter.integral = POW_MAX_PLV * 0.00002f;
				piStructInter.fir = 0;						
				Ctrl_interFace.Curr_REF = 0;						
			}else
			{
				if(Ctrl_interFace.CurrentMax > 0.5f)
				{
					err =  pfc_DataFlowFace.ACinVolRmsFir * 7.2f;
					if(err > 1600.0f)err = 1600.0f;
					out = err / ADSample_Info.vOut_Bat_FIR;
					if(Ctrl_interFace.CurrentMax > out)Ctrl_interFace.CurrentMax = out;
					if(Ctrl_interFace.CurrentMax > CON_CURR_OUT)Ctrl_interFace.CurrentMax = CON_CURR_OUT;	
					//降额处理
					if(HwOtpStr.ErrStaErr == 2)
					{
						Ctrl_interFace.CurrentMax *= 0.5f;
					}else if(HwOtpStr.ErrStaErr == 1)
					{
						Ctrl_interFace.CurrentMax *= 0.75f;
					}
						
					if( Ctrl_interFace.Curr_REF < Ctrl_interFace.CurrentMax - 0.0002f)Ctrl_interFace.Curr_REF += 0.0002f;
					else if( Ctrl_interFace.Curr_REF > Ctrl_interFace.CurrentMax + 0.0002f)Ctrl_interFace.Curr_REF -= 0.0002f;
					else Ctrl_interFace.Curr_REF = Ctrl_interFace.CurrentMax;		
					mComputer();
					err = ADSample_Info.iOut_Bat_adc - Ctrl_interFace.Curr_REF;
					piStructInter.firOut = OneOrderForm(err, &kFilterOne);						
					out = Compensator_Pid(&piStructInter, piStructInter.firOut);				
					Ctrl_interFace.flvOut = out * 50000.0f;
					if(Ctrl_interFace.flvOut > POW_MAX_PLV)
					{
						Ctrl_interFace.flvOut = POW_MAX_PLV;
						piStructInter.integral = POW_MAX_PLV * 0.00002f;
					}							
					if(Ctrl_interFace.flvOut < POW_MIN_PLV)
					{					
						Ctrl_interFace.flvOut = POW_MIN_PLV;
						piStructInter.integral = POW_MIN_PLV * 0.00002f;
					}
					DriverPwm.Plv = Ctrl_interFace.flvOut;
					if(mathTimeHandle() == 0)
					{					
//						DataFlowFace.FaultSta.bit.res |= 1;
						DriverPwm.SynDrv = 0;
					}	
					DriverPwm.DrvH = 1;
					DriverPwm.DrvL = 1;	
					DriverPwm.Duty = 0.5f;
				}else
				{
					time = 0;
					Ctrl_interFace.CtrMode = Transition;
					DriverPwm.DrvH = 0;
					DriverPwm.DrvL = 0;		
					DriverPwm.SynDrv = 0;		
					Ctrl_interFace.flvOut = POW_MAX_PLV;
					piStructInter.integral = POW_MAX_PLV * 0.00002f;
					piStructInter.fir = 0;						
					Ctrl_interFace.Curr_REF = 0;							
				}
			}
		}

		break;
		default:
		break;
	}

}

void PowerIniPidVar(void)
{	
	memset((uint8_t*)&piStructInter, 0, sizeof(pi_str));
	memcpy((uint8_t*)&kFilterTwo,(uint8_t*)&FilterTwo,sizeof(filter_str));
	memcpy((uint8_t*)&kFilterOne,(uint8_t*)&FilterOne,sizeof(filter_str));
}

float piExter_Compute(float input,pi_str* p)
{
	p->output = p->Kp * (input - p->integral) + p->Ki * input;
	p->integral = input;
  return p->output;    
}
float Compensator_Pid(pi_str* p, float err)
{
	p->err = err;                                              //计算误差
  p->integral += p->Ki*p->err;
	p->output = p->integral + p->Kp * p->err;
	return p->output;
}
float oneOrderForm(float input, filter_str *p)
{
	 float vr;
	 vr = (input * p->coeff_B0) + p->filter_W1;
	 p->filter_W1 = (input * p->coeff_B1) - (vr * p->coeff_A1);
	 return vr;
}

float OneOrderForm(float input, filter_str *p)
{
	 float vr;
	 vr = (input * p->coeff_B0) + p->filter_W1;
	 p->filter_W1 = (input * p->coeff_B1) - (vr * p->coeff_A1);
	 return vr;
}

float SecondOrderForm(float input, filter_str *p)
{
	 float vr;
	 vr = (input * p->coeff_B0) + p->filter_W1;
	 p->filter_W1 = (input * p->coeff_B1) + p->filter_W2 - (vr * p->coeff_A1);
	 p->filter_W2 = (input * p->coeff_B2) - (vr * p->coeff_A2);
	 return vr;
}
const filter_str FilterOne = {0.2391f,0.2391f,0,-0.5219f,0,0,0};
const filter_str FilterTwo = {7.4499f,-9.2174f,2.8510f,0.1758f,-0.0922f,0,0};

const pi_str pwmExpi = {0,0,0,0,0,0};

uint16_t starTime = 0;
void SoftStart(void)
{
	static uint16_t preNum = 0;
  float vRly;
	
	vRly = ADSample_Info.vOut_Rly_adc;
	
//	Ctrl_interFace.CtrMode = ConVolt; //测试用
	Ctrl_interFace.Volt_Ref = 30.0f;
	if(DataFlowFace.RelaySta == 0)
	{
		if(vRly < Ctrl_interFace.Volt_Ref)
		{
			RelayOff();
			DischargeOff();
			Ctrl_interFace.preOK = 0;
			DriverPwm.Plv = 120000.0f;     //更改
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
			Ctrl_interFace.CtrMode = ConVolt;
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

void ConVoltHandle(void)
{	
	static float limRef = 0,vFir = 0;
  float temp,limit = 0,vRly=0;
	Ctrl_interFace.Volt_Ref = 30.0f;
	
	vRly = ADSample_Info.vOut_Rly_adc;
	if(vRly < Ctrl_interFace.Volt_Ref)temp = vRly + 0.01f;
	else temp = Ctrl_interFace.Volt_Ref;
	vParamPid.err = temp - vRly;
	vParamPid.output = 0.04f * (vParamPid.err - vParamPid.integral) + 0.001f * vParamPid.err;  
	vParamPid.integral = vParamPid.err;

	iParamPid.err = 5.4f - ADSample_Info.iOut_Bat_adc;
	iParamPid.output = 0.02f * (iParamPid.err - iParamPid.integral) + 0.004f * iParamPid.err; 
	iParamPid.integral = iParamPid.err;
	
	if(vParamPid.output > iParamPid.output)vParamPid.output = iParamPid.output;
	
	vParamPid.output +=vParamPid.oldout;

	if(vParamPid.output > 0.4f)
	{
		vParamPid.output = 0.4f;			
	}	
	
	if(vParamPid.output < 0.02f)DriverPwm.DrvH = 0;
	else DriverPwm.DrvH = 1;
	
	vParamPid.oldout = vParamPid.output;
	DriverPwm.Plv = 120000.0f;
  DischargeOn();
  DriverPwm.DrvL = 1;	
	DriverPwm.SynDrv = 0;
	DriverPwm.Duty = vParamPid.output;	
}


void ConCurrHandle(void)
{	
	if(gSys_State.xp_HandShake == 0)
	{
		DriverPwm.DrvH = 0;
		DriverPwm.DrvL = 0;		
		DriverPwm.SynDrv = 0;		
	}else
	{
		if(gSys_State.xp_HandShake == 1)Ctrl_interFace.CurrentMax = gCAN_DATA.sBMS2_ChrgMsg0_Data.BMS2_ChrgReqCur * 0.1f;
		else if(gSys_State.xp_HandShake == 2)Ctrl_interFace.CurrentMax = gCAN_DATA.sPMS_ChrgMsg0_Data.PMS_ChrgReqCur * 0.1f;
		if(Ctrl_interFace.CurrentMax > 0.1f)
		{
			if(Ctrl_interFace.CurrentMax > 10.0f)Ctrl_interFace.CurrentMax = 10.0f;	         
						
			if(Ctrl_interFace.Curr_REF < Ctrl_interFace.CurrentMax)
			{
				if(Ctrl_interFace.Curr_REF > ADSample_Info.iOut_Bat_adc - 1.0f)
				{
					if( Ctrl_interFace.Curr_REF < Ctrl_interFace.CurrentMax - 0.0002f)Ctrl_interFace.Curr_REF += 0.0002f;
					else Ctrl_interFace.Curr_REF = Ctrl_interFace.CurrentMax;		
//					if(ADSample_Info.iOut_Bat_adc > 1.0f)
//					{
//						Ctrl_interFace.Volt_Ref = ADSample_Info.vOut_Bat_adc + 2.5f;
//						if(Ctrl_interFace.Volt_Ref > MAX_BAT_VOLT)Ctrl_interFace.Volt_Ref = MAX_BAT_VOLT;	
//					}					
				}				
			}else 
			{
				if(Ctrl_interFace.Curr_REF < ADSample_Info.iOut_Bat_adc + 1.0f)
				{
					if( Ctrl_interFace.Curr_REF > Ctrl_interFace.CurrentMax + 0.0002f)Ctrl_interFace.Curr_REF -= 0.0002f;
					else Ctrl_interFace.Curr_REF = Ctrl_interFace.CurrentMax;		
//					if(ADSample_Info.iOut_Bat_adc > 1.0f)
//					{
//						Ctrl_interFace.Volt_Ref = ADSample_Info.vOut_Bat_adc + 2.5f;
//						if(Ctrl_interFace.Volt_Ref > MAX_BAT_VOLT)Ctrl_interFace.Volt_Ref = MAX_BAT_VOLT;	
//					}	
				}	
		  }
			
			iParamPid.err = Ctrl_interFace.Curr_REF - ADSample_Info.iOut_Bat_adc;
			if(iParamPid.err > 0.2f)iParamPid.err = 0.2f;
			else if(iParamPid.err < -0.2f)iParamPid.err = -0.2f;
//			iParamPid.output = 0.001f * (iParamPid.err - iParamPid.integral) + 0.002f * iParamPid.err; 			
			iParamPid.output = 0.0005f * (iParamPid.err - iParamPid.integral) + 0.001f * iParamPid.err; 
			iParamPid.integral = iParamPid.err;
			
//			vParamPid.err = Ctrl_interFace.Volt_Ref - ADSample_Info.vOut_Rly_adc;
//			vParamPid.output = 0.04f * (vParamPid.err - vParamPid.integral) + 0.0001f * vParamPid.err;  
//			vParamPid.integral = vParamPid.err;
//			
//			if(vParamPid.output < iParamPid.output)iParamPid.output = vParamPid.output;
			testDuty = iParamPid.oldout;
			iParamPid.output = iParamPid.oldout + iParamPid.output;			
			
			if(iParamPid.output > 0.4f)iParamPid.output = 0.4f;
			
			if(iParamPid.output < 0.02f)
			{
				DriverPwm.DrvH = 0;
				iParamPid.output = 0.02f;
			}
			else 
			{
				DriverPwm.DrvH = 1;
			}
			iParamPid.oldout = iParamPid.output;
			DriverPwm.Plv = 80000.0f;
			DriverPwm.DrvL = 1;	
			DriverPwm.SynDrv = 0;
			DriverPwm.Duty = iParamPid.output;				
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
}

void SoftCurStart(void)
{
	static uint16_t preChargeTime = 0,preChargeTime2 = 0;
  float vRly,vBat = 0;
  vBat = ADSample_Info.vOut_Bat_adc;
	vRly = ADSample_Info.vOut_Rly_adc;
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
				Ctrl_interFace.psmSwitch = 0;
				preChargeTime = 0;
				iParamPid.oldout = 0;
				Ctrl_interFace.Curr_REF = 0;
				Ctrl_interFace.CtrMode = ConCurPWM;		
			}
		}else if(vBat > 40.5f) preChargeTime = 0;
		if(vBat >=  40.0f)
		{
			preChargeTime2++;
			if(preChargeTime2 >= 20000)
			{
				Ctrl_interFace.CtrMode = Transition;
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

void ConVoltHandleTwo(void)
{		
  float temp,vRly,limit = 0;
	static float limRef = 0.05f;
	vRly = ADSample_Info.vOut_Rly_adc;
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
			vParamPid.output = 0.04f * (vParamPid.err - vParamPid.integral) + 0.001f * vParamPid.err;  //0.003f
			vParamPid.integral = vParamPid.err;
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
			/*
			30 - 0.06 
			40 - 0.105
			50 - 0.15
			60 - 0.18
			*/
			DriverPwm.DrvL = 1;			
			if(vParamPid.output < 0.007f)DriverPwm.DrvH = 0;
			else DriverPwm.DrvH = 1;
			vParamPid.oldout = vParamPid.output;	
			DriverPwm.Duty = vParamPid.output;
			DriverPwm.Plv = 40000.0f;			
			DriverPwm.SynDrv = 0;		
		}
	}else
	{
			DriverPwm.DrvH = 0;
			DriverPwm.DrvL = 0;		
			DriverPwm.SynDrv = 0;	
	}
}
void ConVoltCurHandle(void)
{
	static uint16_t delay = 0;
  float temp,vRly;	
	vRly = ADSample_Info.vOut_Rly_adc;

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
			DriverPwm.Plv = 40000.0f;     //更改
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
//				HAL_COMP_Stop(&hcomp2);	
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
