#ifndef LLC_TYPES_H
#define LLC_TYPES_H
#include <stdint.h>
#define CON_CURR_OUT 16.0f
#define  POW_INT_PLV	          40000.0f   /* 额定周期计数值 */
#define  OUT_MAX_CURT	          12.5f      /* 输出电流上限（保护侧） */

#define  PWM_MIN_DUTY	          0.01f      /* 历史宏，环路未套用 */
#define  PWM_MAX_DUTY	          0.15f      /* 历史宏，环路占空比上限仍为 0.4 */
#define  POW_MAX_CURR	          12.8f
#define  POW_OUT_VOLT	          24.0f

#define  POW_MAX_PLV	          250000.0f  /* PFM 周期上限（最低频） */
#define  POW_MIN_PLV	          60000.0f   /* PFM 周期下限（最高频） */

#define  PID_MAX_PLV	          (POW_MAX_PLV * 0.00002f)  /* PFM 积分上限 = Plv/50000 */
#define  PID_MIN_PLV	          (POW_MIN_PLV * 0.00002f)

#define  SLO_OUT_VOLT	          (POW_OUT_VOLT / POW_INT_PLV)

#define  SR_DELAY_TIME	        0.68f
#define  SR_TIQIA_TIME	        68

/* PWM 增量式 PI 增益（后期台架整定）。PFM 仍用 Compensator_Pid，不共用。 */
#define  PWM_V_KP               0.04f
#define  PWM_V_KI               0.001f
#define  PWM_I_LIM_KP           0.02f
#define  PWM_I_LIM_KI           0.004f
#define  PWM_CUR_KP             0.0005f
#define  PWM_CUR_KI             0.001f
#define  PWM_CV2_KP             0.04f
#define  PWM_CV2_KI             0.001f

/* 功率限幅除法下限，仅防止 v≈0 时 Inf，正常电池电压远大于此值 */
#define  PFM_VBAT_DIV_MIN       0.01f

/* 电流缓变步进（PWM/PFM 共用内核数值，策略分家） */
#define  REF_RAMP_STEP          0.0002f
#define  PWM_CURR_MAX_CLAMP     10.0f    /* PWM 恒流 CurrentMax 钳位 */
#define  PWM_CURR_TRACK_BAND    1.0f     /* PWM 仅在实测电流 ±1A 窗内爬坡 */

/**
 * @brief 快环控制模式（PowerCtrHandle 状态机）
 * @note  PWM->PFM: ConCurPWM -> PwmHold -> Transition -> OvLoadPFM
 */
typedef enum
{
	NoSelect,       /* 空闲/关波 */
	SoftStar,       /* 无握手唤醒预充 */
	BmsStar,        /* 有握手预充 */
	SoftCurSt,      /* CV 预充后恒压 */
	ConVolt,        /* PWM 恒压 30V */
	ConCurPWM,      /* PWM 恒流 */
	Transition,     /* PWM->PFM 占空比过渡 */
	OvLoadPFM,      /* PFM 变频电流环 */
	PwmHold,        /* PWM 切 PFM 前降占空比等电流跌落 */
}CtrMode_st;

/**
 * @brief 快环对外接口
 */
typedef struct
{
	CtrMode_st CtrMode;   /* 当前模式，仅 PowerCtrHandle 或慢状态机改写 */
	CtrMode_st reqMode;   /* Handle 请求的下一模式，由 PowerCtrHandle 采纳 */
	uint8_t preOK;        /* 预充阶段：0 抬压 / 1 等继电器 / 2 完成 */
	uint8_t state;
	uint8_t holdPhase;    /* PwmHold：1=等电流  2=切 Transition */
	uint16_t time;        /* 快环通用拍计数（Hold/Transition） */
	uint8_t litate;       /* 预充子阶段 */
	float  Resistor;
	float  outDuty;       /* 预留占空比 */
	float  flvOut;        /* PFM 频率指令 */
	float  Volt_Ref;
	float  Curr_REF;      /* 缓变后的电流指令 */
	float  CurrentMax;    /* 限幅后的电流上限 */
	float  ovp;
	float  ocp;
}Power_interFace_t;

/** PI 状态：PWM 用 integral 存上一拍误差；PFM 用 integral 做积分项 */
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


/** 一阶/二阶 IIR 系数与状态 */
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

#endif
