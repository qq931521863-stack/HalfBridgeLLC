#ifndef __DSPPOWERCTR_H
#define __DSPPOWERCTR_H

#include "HwConfig.h"	

#define  POW_INT_PLV	          40000.0f        
   
#define  OUT_MAX_CURT	          12.5f

#define  PWM_MIN_DUTY	          0.01f
#define  PWM_MAX_DUTY	          0.15f
#define  POW_MAX_CURR	          12.8f
#define  POW_OUT_VOLT	          24.0f

#define  POW_MAX_PLV	          250000.0f
#define  POW_MIN_PLV	          60000.0f

#define  PID_MAX_PLV	          POW_MAX_PLV * 0.00002f
#define  PID_MIN_PLV	          POW_MIN_PLV * 0.00002f

#define  SLO_OUT_VOLT	          POW_OUT_VOLT / POW_INT_PLV

#define  POW_STEP_PLV	          100.0f 
#define  SR_DELAY_TIME	        0.68f
#define  SR_TIQIA_TIME	        68

//状态机枚举量
typedef enum
{
  NoSelect,       //初始化
	SoftStar,
	BmsStar,
	SoftCurSt,
	ConVolt,
	ConCurPWM,
	Transition,
  OvLoadPFM,  
}CtrMode_st;
/**********
外部定义接口
**********/
typedef struct
{	
	CtrMode_st CtrMode; 
  uint8_t preOK;	
	uint8_t state;
	uint8_t psmSwitch;
	uint16_t time;	
	uint8_t litate;
  float  Resistor;
	float  outDuty;        //输出占空比 0.1f - 0.5f
	float  flvOut;         //输出频率  50000.0f - 420000.0f
  float  Volt_Ref;	
	float  Curr_REF;
	float  CurrentMax;
	float  ovp;            //过压值
	float  ocp;       
}Power_interFace_t;
extern Power_interFace_t Ctrl_interFace;


typedef struct {
	float ss;
	float Kp;
	float Ki;
	float integral;
	float err;
	float fir;
	float firOut;
	float output;
	float oldout;
	
}pi_str;

extern pi_str piStructInter,vParamPid,iParamPid;

typedef struct
{
  float coeff_B0;
  float coeff_B1;
  float coeff_B2;
  float coeff_A1;
  float coeff_A2;	
	
  float filter_W1;
  float filter_W2;	
}filter_str;
extern const filter_str FilterOne,FilterTwo;
extern filter_str kFilterOne,kFilterTwo;
extern const float M_CoeffiFX[14];
extern const float M_CoeffiFp[14];
extern const float M_CoeffiFi[14];
extern const float M_CoeffiFR[14];
extern const float M_CoeffiFB[13];
extern const pi_str pwmExpi;
//extern const float M_CoeffiFp[4][4];
//extern const float M_CoeffiFi[4][4];
//extern void PowerCtrHandle(void);
extern void PowerIniPidVar(void);
extern void powerConver(void);
extern float SecondOrderForm(float input, filter_str *p);
extern float OneOrderForm(float input, filter_str *p);
extern float piExter_Compute(float input,pi_str* p);
extern void SoftStart(void);
extern void ConCurrHandle(void);
extern void ConVoltHandle(void);
extern float Compensator_Pid(pi_str* p, float err);
extern void mComputer(void);
extern uint32_t A_SR_Control(void);
extern void SoftCurStart(void);
extern void ConVoltCurHandle(void);
extern void mathTimeHandle(void);
extern void mComputer(void);
#endif


