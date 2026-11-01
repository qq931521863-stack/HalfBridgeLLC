
#include "mathR02.h"

const float M_highPlv[15][8] = 
{
	50.0f,50.0f,50.0f,50.0f,50.0f,78.0f,113.0f,166.0f,
	50.0f,50.0f,50.0f,50.0f,50.0f,78.0f,113.0f,166.0f,
	50.0f,50.0f,50.0f,50.0f,87.0f,140.0f,193.0f,252.0f,
	50.0f,50.0f,50.0f,60.0f,128.0f,175.0f,275.0f,450.0f,
	50.0f,50.0f,50.0f,70.0f,152.0f,234.0f,311.0f,50.0f,
	50.0f,50.0f,50.0f,87.0f,181.0f,269.0f,370.0f,50.0f,
	50.0f,50.0f,50.0f,130.0f,219.0f,325.0f,431.0f,500.0f,
	50.0f,50.0f,50.0f,122.0f,246.0f,346.0f,500.0f,700.0f,
	50.0f,50.0f,50.0f,142.0f,236.0f,354.0f,520.0f,700.0f,
	50.0f,50.0f,79.0f,178.0f,284.0f,400.0f,600.0f,800.0f,
	50.0f,50.0f,83.0f,195.0f,319.0f,500.0f,700.0f,900.0f,
	50.0f,66.0f,83.0f,207.0f,331.0f,600.0f,800.0f,900.0f,
	50.0f,48.0f,95.0f,236.0f,384.0f,550.0f,700.0f,900.0f,
	50.0f,80.0f,120.0f,230.0f,400.0f,550.0f,700.0f,900.0f,
	50.0f,100.0f,250.0f,500.0f,700.0f,800.0f,900.0f,1000.0f
};
const float M_lowPlv[9][8] = 
{
	2200.0f,2230.0f,2619.0f,2819.0f,2937.0f,2937.0f,2937.0f,3200.0f,
	1358.0f,1700.0f,1971.0f,2065.0f,2277.0f,2324.0f,2500.0f,2800.0f,
	1228.0f,1322.0f,1582.0f,1629.0f,1664.0f,1723.0f,1800.0f,1900.0f,
	1098.0f,1193.0f,1193.0f,1200.0f,1200.0f,1200.0f,1200.0f,1300.0f,
	815.0f,815.0f,815.0f,815.0f,900.0f,900.0f,900.0f,900.0f,
	886.0f,803.0f,662.0f,662.0f,674.0f,674.0f,700.0f,700.0f,
	792.0f,615.0f,285.0f,320.0f,308.0f,367.0f,226.0f,230.0f,
	311.0f,200.0f,157.0f,98.0f,87.0f,80.0f,70.0f,70.0f,
	355.0f,249.0f,190.0f,96.0f,90.0f,90.0f,90.0f,90.0f
};

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
float Sr_compute(void)
{	
	uint8_t k,i1,i2,i3;
	float iout,out = 0;	
	float inx,inx2,inb;
	iout = ADSample_Info.iOut_Bat_adc;
	out = ADSample_Info.iOut_Bat_FIR;
	
	if(ADSample_Info.iOut_Bat_FIR >= 6.0f)DriverPwm.SynDrv = 1;
	else if(ADSample_Info.iOut_Bat_adc < 4.0f)DriverPwm.SynDrv = 0;	
	
	inb = Ctrl_interFace.flvOut;
  if(inb < 102000.0f)
	{
		for(k = 0;k < 9;k++)
		{
			if(inb >= M_Arloplv[k])
			{
				if(k == 8)
				{
					i1 = 8;
					break;
				}
			}else
			{
				if(k){i1 = k;break;}
				else {i1 = 0;break;}			
			}
		}
			
		inb = out;
		for(k = 0;k < 8;k++)
		{
			if(inb >= M_ArLow[k])
			{
				if(k == 7)
				{
					i2 = 7;
					break;
				}
			}else
			{
				if(k){i2 = k;break;}
				else {i2 = 0;break;}			
			}		
		}	
		
		inb = iout;
		
		for(k = 0;k < 8;k++)
		{
			if(inb >= M_ArLow[k])
			{
				if(k == 7)
				{
					i3 = 7;
					break;
				}
			}else
			{
				if(k){i3 = k;break;}
				else {i3 = 0;break;}			
			}		
		}
		
		inx = M_lowPlv[i1][i2];
		inx2 = M_lowPlv[i1][i3];
		
		if(inx < 10.0f)return 0;
		if(inx2 < 10.0f)return 0;
		
		if(i2 > 0)
		{
			inb = M_lowPlv[i1][i2 - 1];
			if(inb > inx)inx = inb;
		}
		if(i1 > 0)
		{
			inb = M_lowPlv[i1 - 1][i2];
			if(inb > inx)inx = inb;			
		}
		if(i1 < 7)
		{
			inb = M_lowPlv[i1 + 1][i2];
			if(inb > inx)inx = inb;				
		}
		
    			
		if(i3 > 0)
		{
			inb = M_lowPlv[i1][i3 - 1];
			if(inb > inx2)inx2 = inb;
		}
		if(i1 > 0)
		{
			inb = M_lowPlv[i1 - 1][i3];
			if(inb > inx2)inx2 = inb;			
		}
		if(i1 < 7)
		{
			inb = M_lowPlv[i1 + 1][i3];
			if(inb > inx2)inx2 = inb;				
		}		
		
		if(inx2 > inx)inx = inx2;
		
	}else
	{		
		for(k = 0;k < 15;k++)
		{
			if(inb >= M_ArHigh[k])
			{
				if(k == 14)
				{
					i1 = 14;
					break;
				}
			}else
			{
				if(k){i1 = k;break;}
				else {i1 = 0;break;}			
			}
		}
			
		if(iout > out)inb = iout;
		else inb = out;
		inb += 3.0f;
		if(i1 < 7)i1++;
		for(k = 0;k < 8;k++)
		{
			if(inb >= M_ArLow[k])
			{
				if(k == 7)
				{
					i2 = 7;
					break;
				}
			}else
			{
				if(k){i2 = k;break;}
				else {i2 = 0;break;}			
			}		
		}		
		inx = M_highPlv[i1][i2];
	}
  return inx;	
}


uint8_t mathTimeHandle(void)
{
	uint8_t res = 1;
	float temp,inb;

	if(DriverPwm.SynDrv)
	{
		if((ADSample_Info.iOut_Bat_FIR < 4.0f)||(ADSample_Info.iOut_Bat_adc < 4.0f))
		{
			DriverPwm.SynDrv = 0;
		}
	}else
	{
		DriverPwm.Sr_Dtime = 0;
		if(ADSample_Info.iOut_Bat_FIR > 6.0f)DriverPwm.SynDrv = 1;
	}
	if(DriverPwm.SynDrv)
	{
		temp = Sr_compute();
		if(temp > 0)
		{
			inb = Ctrl_interFace.flvOut;
			if(inb >= 102000.0f)
			{
				DriverPwm.Sr_Atime = (temp + 200) * SR_DELAY_TIME;
				DriverPwm.Sr_Btime = 200 * SR_DELAY_TIME;  
			}else
			{	
				DriverPwm.Sr_Atime = 200 * SR_DELAY_TIME;
				DriverPwm.Sr_Btime = (temp + 200) * SR_DELAY_TIME;		
			}
	  }else res = 0;
  }
	return res;
}

