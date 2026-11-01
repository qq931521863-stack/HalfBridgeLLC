#ifndef OPERATE_STATUS_H
#define OPERATE_STATUS_H

#include <string.h>
#include "main.h"
#include "stm32g4xx_hal.h"


//变量类型定义
//typedef signed char             int8_t;
//typedef short int               int16_t;
//typedef int                     int32_t;
//typedef unsigned char           uint8_t;
//typedef unsigned short int      uint16_t;
//typedef unsigned int            uint32_t;

typedef struct {
	uint8_t SysSta;			//系统状态
	uint8_t xp_HandShake;	//握手标志
	uint8_t xp_ChrgEna;		//充电标志
	uint8_t xp_ChrgSt;		//电池状态
	uint8_t xp_CVmode;		//本体CV模式
	uint8_t xpRx_CVmode;		//接受的CV模式
	uint8_t Chrgr_FaultInfo;	//错误状态
	uint32_t Fault_Blocked;		//错误掩码
	float xp_BatVolt;	//电池电压
}Sys_State;

enum {
	Initial_state = 0,
	ConstantVoltWackup_state,
	ChargeStandby_state,
	Charging_state,
	FullCharged_state,
//	ChargingCV_state,
	Fault_state,
	Ota_state,
};

typedef struct {
	 uint8_t SysStateNum;			//状态序号
	void (*Function)(void);    // 状态功能函数
	void (*SwitchState)(void); // 状态切换判断函数
} Sys_States_API;

extern Sys_State gSys_State;
extern void runMainStateMachine(void);
extern void SysStatesInit(void) ;
#endif
