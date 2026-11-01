
#include "HwConfig.h"

ADSample_Var_t  ADSample_Info;

uint16_t ADC0_Buffer[8],ADC2_Buffer[8];

const uint16_t ZM3973NTC[166]=
{
  3968,3959,3951,3941,3932,3922,3911,3900,3888,3876,3863,3850,3836,3821,3805,3789,
  3772,3755,3736,3717,3697,3676,3654,3631,3607,3583,3558,3532,3505,3477,3448,3419,
  3388,3357,3325,3293,3259,3225,3190,3154,3117,3080,3042,3003,2964,2924,2883,2842,
  2801,2758,2716,2673,2629,2585,2541,2497,2452,2407,2362,2317,2272,2227,2182,2137,
  2092,2048,2003,1959,1915,1872,1829,1786,1744,1703,1662,1621,1581,1542,1503,1465,
  1428,1391,1355,1319,1284,1250,1217,1184,1153,1121,1091,1061,1032,1003, 976, 949,
  922,896,871,847,823,800,777,755,734,713,693,673,654,636,617,600,
  583,566,550,535,519,505,490,476,463,450,437,425,413,401,390,379,
  369,358,348,339,329,320,312,303,295,287,279,271,264,257,250,243,
  237,231,224,218,213,207,202,196,191,186,182,177,172,168,164,159,
  155,152,148,144,140,137
};
const uint16_t HNE104[166]=
{
  3979,3971,3963,3954,3945,3935,3925,3914,3903,3891,3878,3865,3851,3837,3821,3805,
  3789,3771,3753,3734,3714,3694,3672,3650,3627,3603,3578,3552,3526,3498,3470,3440,
  3410,3379,3347,3314,3280,3245,3210,3173,3136,3098,3059,3020,2980,2939,2898,2856,
  2813,2770,2726,2682,2638,2593,2548,2489,2458,2412,2366,2321,2275,2229,2183,2138,
  2093,2048,2003,1958,1914,1870,1827,1784,1741,1699,1658,1617,1576,1537,1498,1459,
  1421,1384,1348,1312,1277,1243,1209,1176,1144,1112,1081,1051,1022,993,965,938,
  911,885,860,836,812,788,766,743,722,701,681,661,642,623,605,588,
  571,554,538,522,507,493,478,465,451,438,426,413,401,390,379,368,
  357,347,337,328,319,310,301,292,284,276,269,261,254,247,240,233,
  227,221,215,209,203,198,192,187,182,177,173,168,164,159,155,151,
  147,143,140,136,132,129
};
static uint8_t tsp_BinaryTableSearch( uint16_t adc_val )
{
	uint16_t start = 0U, end = 165, mid = 0U, retout = 0;
	if( adc_val <= ZM3973NTC[165])retout = 165;
	else if( adc_val >= ZM3973NTC[0])retout = 0;
	else
	{
		while ( start <= end )
		{
			mid = (start + end) >> 1; 
			if( adc_val ==  ZM3973NTC[mid] )
			{
				break;
			}
			if( ( adc_val < ZM3973NTC[mid] ) && ( adc_val > ZM3973NTC[mid+1U] ) )
			{
				break;
			}
			if( adc_val < ZM3973NTC[mid] )
			{
				start = mid + 1U; 
			}
			else if( adc_val > ZM3973NTC[mid] )
			{
				end = mid - 1U;
			}
		}
		start = ZM3973NTC[mid] - adc_val;
		end = (ZM3973NTC[mid]-ZM3973NTC[mid+1])>>1;
		if(start >= end)retout = mid+1;
		else retout = mid;
	}
	return retout;
}

static uint8_t Air_BinaryTableSearch( uint16_t adc_val )
{
	uint16_t start = 0U, end = 165, mid = 0U, retout = 0;
	if( adc_val <= HNE104[165])retout = 165;
	else if( adc_val >= HNE104[0])retout = 0;
	else
	{
		while ( start <= end )
		{
			mid = (start + end) >> 1; 
			if( adc_val ==  HNE104[mid] )
			{
				break;
			}
			if( ( adc_val < HNE104[mid] ) && ( adc_val > HNE104[mid+1U] ) )
			{
				break;
			}
			if( adc_val < HNE104[mid] )
			{
				start = mid + 1U; 
			}
			else if( adc_val > HNE104[mid] )
			{
				end = mid - 1U;
			}
		}
		start = HNE104[mid] - adc_val;
		end = (HNE104[mid]-HNE104[mid+1])>>1;
		if(start >= end)retout = mid+1;
		else retout = mid;
	}
	return retout;
}
void ADC0_Sample(void)	
{
  float adc_temp;
#ifndef PLECS_DLL
	adc_temp = (float)ADC0_Buffer[0];
	ADSample_Info.vOut_Rly_adc = (float)adc_temp * COM_VOUT_BASE;	
	adc_temp = (float)ADC0_Buffer[1];
	ADSample_Info.vOut_Bat_adc = (float)adc_temp * COM_VOUT_BASE;	
	adc_temp = (float)ADC0_Buffer[2];
	ADSample_Info.iOut_Bat_adc = (float)adc_temp * COM_IOUT_BASE;	
#else
	(void)adc_temp;
#endif
	ADSample_Info.iOut_Bat_FIR = ADSample_Info.iOut_Bat_FIR * 0.999f + ADSample_Info.iOut_Bat_adc * 0.001f;
	ADSample_Info.vOut_Bat_FIR = ADSample_Info.vOut_Bat_FIR * 0.999f + ADSample_Info.vOut_Bat_adc * 0.001f;
}	
void ADC2_Sample(void)	
{
#ifndef PLECS_DLL
  float adc_temp;
	uint16_t temp;
	
	//HAL_GPIO_TogglePin(LED_OUT3_GPIO_Port, LED_OUT3_Pin);
	temp = ADC2_Buffer[0]; 

	ADSample_Info.Temp0_adc = tsp_BinaryTableSearch(temp); //���ɢ�����¶�
  temp = ADC2_Buffer[1]; 

	ADSample_Info.Temp1_adc = Air_BinaryTableSearch(temp); //�ڲ������¶�

	temp = ADC2_Buffer[2]; 
		
	ADSample_Info.Temp2_adc = tsp_BinaryTableSearch(temp); //��������¶�
	temp = ADC0_Buffer[4]; 
		
	ADSample_Info.Temp3_adc = tsp_BinaryTableSearch(temp);  //��ѹ���¶�
	
	ADSample_Info.NTC0_AD_FIR = ADSample_Info.NTC0_AD_FIR * 0.99f + ADSample_Info.Temp0_adc * 0.01f;
	ADSample_Info.NTC1_AD_FIR = ADSample_Info.NTC1_AD_FIR * 0.99f + ADSample_Info.Temp1_adc * 0.01f;
	ADSample_Info.NTC2_AD_FIR = ADSample_Info.NTC2_AD_FIR * 0.99f + ADSample_Info.Temp2_adc * 0.01f;
	ADSample_Info.NTC3_AD_FIR = ADSample_Info.NTC3_AD_FIR * 0.99f + ADSample_Info.Temp3_adc * 0.01f;
	
	ADSample_Info.Fan0_cur = ADSample_Info.Fan0_cur * 0.99f + (float)ADC2_Buffer[3] * COM_FCUR_BASE * 0.01f; 
	ADSample_Info.Fan1_cur = ADSample_Info.Fan1_cur * 0.99f + (float)ADC2_Buffer[4] * COM_FCUR_BASE * 0.01f;
	ADSample_Info.Fan2_cur = ADSample_Info.Fan2_cur * 0.99f + (float)ADC2_Buffer[5] * COM_FCUR_BASE * 0.01f;
	ADSample_Info.Fan3_cur = ADSample_Info.Fan3_cur * 0.99f + (float)ADC2_Buffer[6] * COM_FCUR_BASE * 0.01f;
	
	ADSample_Info.vRef_1v5_adc = (float)ADC2_Buffer[7] * COM_IOUT_BASE;	
	
	ADSample_Info.vRef_1v5_FIR = ADSample_Info.vRef_1v5_FIR * 0.9372f + ADSample_Info.vRef_1v5_adc *0.0628f;

  // 12V��Դ
	adc_temp = (float)ADC0_Buffer[3] * COM_AUX_BASE;
	ADSample_Info.AuxVolt= ADSample_Info.AuxVolt * 0.9372f + adc_temp *0.0628f;	
#endif
}	

void HAL_COMP_SR(void)
{
#ifndef PLECS_DLL
	if(LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_22) != 0UL)
	{
		LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_22);
	}
	if(LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_21) != 0UL)
	{
		LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_21);
	}
 	 if(LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_29) != 0UL)
	{
	  LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_29);
	}
#endif
}

