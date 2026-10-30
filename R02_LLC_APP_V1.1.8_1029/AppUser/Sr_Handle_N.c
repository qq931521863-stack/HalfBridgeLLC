
#include "mathR02.h"


const float M_ArHigh[15]   = {110000.0f,120000.0f,130000.0f,140000.0f,150000.0f,160000.0f,170000.0f,180000.0f,190000.0f,200000.0f,210000.0f,220000.0f,230000.0f,240000.0f,250000.0f};
const float M_Arloplv[9]   = {60000.0f,65000.0f,70000.0f,75000.0f,80000.0f,85000.0f,90000.0f,95000.0f,100000.0f};
	
const float M_ArLow[8]     = {5.0f,10.0f,15.0f,20.0f,25.0f,30.0f,35.0f,40.0f};

const float M_CoeffiFX[14] = {60000.0f,70000.0f,80000.0f,90000.0f,102000.0f,110000.0f,120000.0f,130000.0f,140000.0f,160000.0f,180000.0f,200000.0f,250000.0f,300000.0f};
const float M_CoeffiFp[14] = {0.00376927f,0.003588f,0.002578f,0.001888763f,0.0018168f,0.0024f,0.00566f,0.01033f,0.021f,0.058f,0.086f,0.112976f,0.16f,0.24f};
const float M_CoeffiFi[14] = {19.11f,30.9894f,35.355f,46.9245f,71.63f,88.231f,114.292f,156.458f,300.113f,710.1f,965.1f,1208.0f,1500.0f,2000.0f};

void mComputer(void)
{
	uint8_t k,i1,i2;
	float b,inx,inx2,iny1,iny2,inb;
	inb = DriverPwm.Plv;
	for(k = 0;k < 13;k++)
	{
		if(inb >= M_CoeffiFX[k])
		{
			if(k == 12)
			{
				i1 = 12;
				i2 = i1;
				break;
			}
		}else
		{
			if(k){i1 = k-1;i2=k;break;}
			else {i1 = 0;i2 = 0;break;}			
		}
	}

	if(i1 == i2)
	{		
		inx = M_CoeffiFp[i1];
		iny1 = M_CoeffiFi[i1];
	}
	else 
	{
		inx = M_CoeffiFp[i1];
		iny1 = M_CoeffiFi[i1];				

		inx2 = M_CoeffiFp[i2];
		iny2 = M_CoeffiFi[i2];				
		
		b = (inb - M_CoeffiFX[i1])/(M_CoeffiFX[i2] - M_CoeffiFX[i1]);
		inx = inx + (inx2 - inx) * b;
		iny1 = iny1 + (iny2 - iny1) * b;
	}		
	
	if(piStructInter.ss == 0)piStructInter.ss =1.0f;
	if(piStructInter.ss < 0.8f)
	{
		if(ADSample_Info.iOut_Bat_adc < 18.0f)piStructInter.ss = 1.0f;
	}else
	{
		if(ADSample_Info.iOut_Bat_adc > 22.0f)piStructInter.ss = 0.5f;
	}
	piStructInter.Kp = inx * piStructInter.ss;
	piStructInter.Ki = iny1 * piStructInter.ss * 0.000025f;	//0.0000125f
}

void mathTimeHandle(void)
{
	float inb,inx;
	inb = Ctrl_interFace.flvOut;	
	if(inb < 95000.0f)
	{
    if(inb < 65000.0f)inx = 3500.0f;
		else if(inb < 70000.0f)inx = 3000.0f;
		else if(inb < 75000.0f)inx = 2500.0f;
		else if(inb < 80000.0f)inx = 2000.0f;
		else if(inb < 85000.0f)inx = 1600.0f;
		else if(inb < 90000.0f)inx = 1200.0f;
		else inx = 900.0f;
		DriverPwm.Sr_Atime = 200.0f * SR_DELAY_TIME;
		DriverPwm.Sr_Btime = (inx + 200.0f) * SR_DELAY_TIME;		
		
	}else if(inb >= 115000.0f)
	{
    if(inb >= 140000.0f)inx = 500.0f;
		else if(inb >= 130000.0f)inx = 400.0f;
		else if(inb >= 120000.0f)inx = 300.0f;
		else inx = 200.0f;
		DriverPwm.Sr_Atime = (inx + 200.0f) * SR_DELAY_TIME;
		DriverPwm.Sr_Btime = 200.0f * SR_DELAY_TIME; 		
	}else
	{
		DriverPwm.Sr_Atime = 400.0f * SR_DELAY_TIME;
		DriverPwm.Sr_Btime = 800.0f * SR_DELAY_TIME; 
	}		
}

