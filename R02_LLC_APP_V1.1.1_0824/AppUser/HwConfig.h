
#ifndef HWCONFIG_H
#define HWCONFIG_H

#ifdef __cplusplus
 extern "C" {
#endif


#ifdef PLECS_DLL
#include "plecs_stub.h"
#else
#include "stm32g4xx_hal.h"
#include "main.h"
#endif
#include <stdio.h>
#include "string.h"
#include "math.h"
#include "mathR02.h"
#ifndef PLECS_DLL
#include "MODBUS_SLAVE.h"
#include "CAN_Control_2800W.h"
#include "operateStatus.h"
#include "ProtectionLLC.h"
#endif

#define OUTPUT_POW_MAX          1650.0f
#define OUTPUT_CUR_MAX          30.0f

#define OUTPUT_CUR_OVER         36.0f


//**********���Բ���**************

#define CON_VOLT_OUT            30.0f
#define CON_CURR_OUT	          16.0f   
#define VOLT_Threshold          22.5f
#define CURR_Threshold	        3.0f   

#define CON_VOLT_INI            30.0f  
#define OUT_POWER_MAX           1500.0f  
//**********���Բ���**************

#define MAX_BAT_VOLT            58.0f
#define HW_VOLT_OVP             65.0f
#define HW_VOLT_UVP             10.0f
#define OUTPUT_SHORT            10.0f

#define HW_ACIN_OFP             65.0f
#define HW_ACIN_OFR             63.0f

#define HW_ACIN_UFP             45.0f
#define HW_ACIN_UFR             47.0f

#define SYSTEM_PLV              170000000
#define SHRTIMER_PLV            680000000
#define SHRTIMER_MAX_PREIOD     300000
#define SHRTIMER_MIN_PREIOD     4500 
#define SHRTIMER_MAX_DUTY       0.5f
#define SHRTIMER_MIN_DUTY       0.05f

#define LLC_DEADTIME            120 //300ns

#define HW_ADC_REF              3.3f
#define HW_VOUT_GAIN            21.0f
#define HW_IOUT_GAIN            20.0f
#define HW_VREF_GAIN            400.0f
#define HW_FANCUR_GAIN          3.0f 
#define HW_AUX_GAIN             4.9f

#define HW_MAX_OCP              50.0f
#define HW_MAX_OVP              65.0f //65

#define HW_DAC_OCP              (uint16_t)((HW_MAX_OCP * 4095.0f)/(HW_ADC_REF * HW_IOUT_GAIN)) 
#define HW_DAC_OVP              (uint16_t)((HW_MAX_OVP * 4095.0f)/(HW_ADC_REF * HW_VOUT_GAIN)) 
#define HW_DAC_ICP              2600 
#define HW_AUX_GAIN             4.9f

#define COM_VOUT_BASE           ((float)(HW_ADC_REF * HW_VOUT_GAIN))/4095.0f
#define COM_IOUT_BASE           ((float)(HW_ADC_REF * HW_IOUT_GAIN))/4095.0f
#define COM_VREF_BASE           ((float)(HW_ADC_REF * HW_VREF_GAIN))/4095.0f	
#define COM_FCUR_BASE           ((float)(HW_ADC_REF * HW_FANCUR_GAIN))/4095.0f	
#define COM_AUX_BASE            ((float)(HW_ADC_REF * HW_AUX_GAIN ))/4095.0f	
	
#ifdef PLECS_DLL
#define DrvH_On()
#define DrvH_Off()
#define DrvL_On()
#define DrvL_Off()
#define SarH_On()
#define SarH_Off()
#define SarL_On()
#define SarL_Off()
#else
#define DrvH_On()   {GPIOB->MODER &= 0XF3FFFFFF; GPIOB->MODER |= 0X08000000;}  //
#define DrvH_Off()  {GPIOB->MODER &= 0XF3FFFFFF; GPIOB->MODER |= 0X04000000;GPIOB->BRR = (uint32_t)GPIO_PIN_13;}
#define DrvL_On()   {GPIOB->MODER &= 0XFCFFFFFF; GPIOB->MODER |= 0X02000000;}  //
#define DrvL_Off()  {GPIOB->MODER &= 0XFCFFFFFF; GPIOB->MODER |= 0X01000000;GPIOB->BRR = (uint32_t)GPIO_PIN_12;}

#define SarH_On()   {GPIOB->MODER &= 0XCFFFFFFF; GPIOB->MODER |= 0X20000000;}  //
#define SarH_Off()  {GPIOB->MODER &= 0XCFFFFFFF; GPIOB->MODER |= 0X10000000;GPIOB->BRR = (uint32_t)GPIO_PIN_14;}
#define SarL_On()   {GPIOB->MODER &= 0X3FFFFFFF; GPIOB->MODER |= 0X80000000;}  //
#define SarL_Off()  {GPIOB->MODER &= 0X3FFFFFFF; GPIOB->MODER |= 0X40000000;GPIOB->BRR = (uint32_t)GPIO_PIN_15;}
#endif
#define PI_FLOAT    3.141592653f

typedef struct 
{      
	float  vOut_Rly_adc;
	float  vOut_Rly_LPF;
  float  vOut_Bat_adc;
  float  vOut_Bat_FIR;
	float  vOut_Bat_LPF;
	float  iOut_Bat_adc;
	float  iOut_Bat_FIR;
	float  iOut_Bat_LPF;
  float  vRef_1v5_adc;
	float  vRef_1v5_FIR;
	
	float  AuxVolt;
  float  Temp0_adc;
  float  Temp1_adc;
  float  Temp2_adc;
	float  Temp3_adc;
	
  float  Fan0_cur;
  float  Fan1_cur;
  float  Fan2_cur;
	float  Fan3_cur;
	
  float  NTC0_AD_FIR;
  float  NTC1_AD_FIR;
  float  NTC2_AD_FIR;
	float  NTC3_AD_FIR;
	
  uint16_t  NTC0_out;
  uint16_t  NTC1_out;
  uint16_t  NTC2_out;	
	
} ADSample_Var_t;
extern ADSample_Var_t ADSample_Info;

typedef struct 
{      
	uint32_t delay_LED;
	uint32_t delay_Slow;
	uint32_t delay_ini;	
	uint32_t delay_num;
	uint32_t powerOn;
} DelayMS_t;
extern DelayMS_t  Delay1ms;

typedef struct
{
	uint8_t  DrvH;          
	uint8_t  DrvL;          
	uint8_t  SynDrv;      
	uint16_t Sr_Atime;	
	uint16_t Sr_Btime;
  uint16_t DrvDtime;
  float    Sr_Dtime;	
	float    Plv;	
	float    Duty; 	
}Driver_t;
extern Driver_t DriverPwm;

//״̬��ö����
typedef enum
{
  Init,
  Wakeup,
  Stadby,
  Charging,
  FullCharged,
  Fault
}STATE_M;
typedef struct{
   uint32_t ACin_ov   : 1 ;
   uint32_t ACin_uv   : 1 ;
   uint32_t ACin_oc   : 1 ;
   uint32_t ACin_of   : 1 ;
   uint32_t ACin_uf   : 1 ;
   uint32_t Vbus_ov   : 1 ;
   uint32_t Vbus_uv   : 1 ;
   uint32_t Out_ov    : 1 ;
   uint32_t Out_uv    : 1 ;
   uint32_t Out_oc    : 1 ;
   uint32_t PFC_Err   : 1 ;
   uint32_t LLC_Err   : 1 ;
   uint32_t Out_short : 1 ;
   uint32_t PFC_otp   : 1 ;
   uint32_t LLC_otp   : 1 ;
   uint32_t BAT_revs  : 1 ;
   uint32_t inRLY_Err : 1 ;	
   uint32_t outRLY_Err : 1 ;	
   uint32_t FAN_Err   : 1 ;
   uint32_t CAN_Err   : 1 ;
   uint32_t INS_Err   : 1 ;
   uint32_t SCI_Err   : 1 ;
	 uint32_t ResonOc   : 1 ;
	 uint32_t InterOv   : 1 ;	 
   uint32_t res       : 8 ;				
}FAULT_BIT_T;

//С����ģʽ
typedef union {
	uint32_t     all;
	FAULT_BIT_T  bit;
}FAULT_STA_T;
typedef struct 
{      
	STATE_M      RunState;     
	STATE_M      OldState;      
	FAULT_STA_T  FaultSta;	    
	uint8_t      RelaySta;	   
	uint8_t      RelayOld;
	uint8_t      DisCharge;     
	uint8_t      PFC_ok;        
	uint8_t      LLC_en;        
	uint8_t      derating;
	uint8_t      Burst;
	uint8_t      derate;
	uint8_t      derateLevel;	
	float        outSetVolt;
	float        ChargeMaxVol;  
	float        ChargeMaxCur; 
	float		     Fanpwm;
} DataFlow_t;

extern float testDuty;
extern DataFlow_t  DataFlowFace;
extern void sys_intx_enable(void);
extern void sys_intx_disable(void);
extern void IncTick(void);
extern void gpio_config(void);
extern void fwdg_init(void);
extern void CrcIni(void);
extern uint32_t CrcCompute(uint32_t* data,uint32_t num);
extern uint16_t ADC0_Buffer[8],ADC2_Buffer[8];

extern void ADC0_Sample(void);
extern void ADC2_Sample(void);
extern void interrupt_ADC1(void);
extern void interrupt_ADC2(void);
extern void PwmCtrl(void);
extern void PwmClose(void);
extern void SHRTIMERdrive(void);
extern void LLC_Disable(void);
extern void LLC_Enable(void);
extern void RelayOn(void);
extern void RelayOff(void);
extern void starLowIni(void);
extern void notch_config(float inflv);
extern void VIacRmsCal(void);
extern void HandleFast(void);
extern void starLoopIni(void);
extern void runLoopIni(void);
extern void HwOcp(void);

extern void StateM(void);
extern void SlowP(void);
extern void CANVIrefGet(void);
extern void UartCom(void);
extern void LEDShow(void);

extern void StateMan(void);
extern void StateMInit(void);
extern void ConVoWakeup(void);
extern void StandBy(void);
extern void ChargeOn(void);
extern void ChargeFull(void);
extern void StateMErr(void);


extern void preChargeHandle(void);
extern void DischargeOn(void);
extern void DischargeOff(void);
extern void HAL_COMP_SR(void);
extern void SwOCP(void);
extern void OTP_Protection(void);
float JudgeMode(float inx);
#ifdef __cplusplus
}
#endif

#endif /* GD32E503V_EVAL_H */
