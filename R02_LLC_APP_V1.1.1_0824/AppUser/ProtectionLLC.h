
#ifndef PROTECTIONLLC_H
#define PROTECTIONLLC_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32g4xx_hal.h"
#include "math.h"
#include <stdio.h>
#include "string.h"
#include "main.h"
#include "MODBUS_SLAVE.h"
#include "HwConfig.h"


/*************************
1.输出过压
2.输出欠压
3.输出过流
4.LLC辅源故障
5.输出短路
6.输出继电器故障
7.CAN通讯故障
8.内部UART故障
*************************/

#define   SYS_PROTECT_NUM             8
//状态机枚举量
typedef enum
{
  ErrTime,     //错误计时过程
  WaitReTime,  //重启等待计时过程
	ClearTime,   //故障清除计时过程
}STATE_PROTECT;

#pragma pack(1)
typedef struct {
	uint8_t  FaultID;
	uint8_t  byEnable;
	uint8_t  byDirectionConfig;    // 0:< 1:> 2: > || < 3:<&>
	float    Threshold_A;
	float    Threshold_B;	
	uint16_t DelayTimeA;          //错误持续时间
	uint8_t  ErrNum;
	
  uint8_t  RepeatNum;
	uint16_t DelayTimeB;          //重启时间间隔
	uint16_t ClearTime;           //故障清除时间间隔
	
	uint8_t  ClearEnable;
	uint8_t  DirectionConfig;     // 0:< 1:> 2: > || < 3:<&>
	float    Threshold_C;
	float    Threshold_D;
	uint16_t DelayTimeC;          //恢复持续时间
} TSysProPara;
#pragma pack()

#pragma pack(1)
typedef struct {
	uint8_t  ErrState;	//0 错误计时过程 1 重启等待计时过程 
	uint8_t  DelayNum;
	uint8_t  Full;
	uint16_t TimeCount;
	uint16_t RepeatNum;
	uint16_t Delay[20];
} TSysProData;

typedef struct {
	uint8_t  OvFault;
	uint8_t  iniOk;
	uint16_t LowVoltTime;
	uint16_t NorVoltTime;
	uint32_t OvTime;
} HwProData;
typedef struct {
	float    tempIn[8];
	int16_t  Delayu;
	int16_t  Delayd;
	uint8_t  ErrStaErr;  //降额1 2 
} HwOtProData;
#pragma pack()
extern const TSysProPara  SysProPara[SYS_PROTECT_NUM];

extern TSysProData SysProtectData[SYS_PROTECT_NUM];
extern HwProData  HwprotectData;
extern HwOtProData HwOtpStr;
extern void HwProtect(void);
extern void RelayProtect(void);
extern void AuxProtect(void);
extern void OutOcProtect(void);
extern void OutUvProtect(void);
extern void OutOvProtect(void);
extern void Protect_Short(void);
extern void HwProtectIni(void);
extern void SysReset(void);
#ifdef __cplusplus
}
#endif

#endif 
