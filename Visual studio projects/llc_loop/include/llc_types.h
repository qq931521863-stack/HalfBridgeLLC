#ifndef LLC_TYPES_H
#define LLC_TYPES_H
#include <stdint.h>

#define CON_CURR_OUT            16.0f
#define POW_INT_PLV             40000.0f
#define OUT_MAX_CURT            12.5f
#define PWM_MIN_DUTY            0.01f
#define PWM_MAX_DUTY            0.15f
#define POW_MAX_CURR            12.8f
#define POW_OUT_VOLT            24.0f
#define POW_MAX_PLV             250000.0f
#define POW_MIN_PLV             60000.0f
#define PID_MAX_PLV             (POW_MAX_PLV * 0.00002f)
#define PID_MIN_PLV             (POW_MIN_PLV * 0.00002f)
#define SLO_OUT_VOLT            (POW_OUT_VOLT / POW_INT_PLV)
#define SR_DELAY_TIME           0.68f
#define SR_TIQIA_TIME           68

#define PWM_V_KP                0.04f
#define PWM_V_KI                0.001f
#define PWM_I_LIM_KP            0.02f
#define PWM_I_LIM_KI            0.004f
#define PWM_CUR_KP              0.0005f
#define PWM_CUR_KI              0.001f
#define PWM_CV2_KP              0.04f
#define PWM_CV2_KI              0.001f

#define PFM_VBAT_DIV_MIN        0.01f
#define REF_RAMP_STEP           0.0002f
#define PWM_CURR_MAX_CLAMP      10.0f
#define PWM_CURR_TRACK_BAND     1.0f

typedef enum
{
	NoSelect,
	SoftStar,
	BmsStar,
	SoftCurSt,
	ConVolt,
	ConCurPWM,
	Transition,
	OvLoadPFM,
	PwmHold
} CtrMode_st;

typedef struct
{
	CtrMode_st CtrMode;
	CtrMode_st reqMode;
	uint8_t preOK;
	uint8_t state;
	uint8_t holdPhase;
	uint16_t time;
	uint8_t litate;
	float Resistor;
	float outDuty;
	float flvOut;
	float Volt_Ref;
	float Curr_REF;
	float CurrentMax;
	float ovp;
	float ocp;
} Power_interFace_t;

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
} pi_str;

typedef struct
{
	float coeff_B0;
	float coeff_B1;
	float coeff_B2;
	float coeff_A1;
	float coeff_A2;
	float filter_W1;
	float filter_W2;
} filter_str;

#endif
