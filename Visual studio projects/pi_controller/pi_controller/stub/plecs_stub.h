#ifndef PLECS_STUB_H
#define PLECS_STUB_H

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	uint8_t BMS2_HandShake;
	uint8_t BMS2_ChrgEna;
	uint16_t BMS2_ChrgReqCur;
	uint16_t BMS2_ChrgReqVolt;
	uint8_t BMS2_ChrgSt;
	uint8_t BMS2_ChrgMsg_ctRoll;
	uint8_t BMS2_ChrgMsg_Checksum;
} BMS2_ChrgMsg0_Data;

typedef struct {
	uint8_t PMS_HandShake;
	uint8_t PMS_ChrgEna;
	uint8_t PMS_CVModeReq;
	uint16_t PMS_ChrgReqCur;
	uint16_t PMS_ChrgReqVolt;
	uint8_t PMS_ChrgMsg_ctRoll;
	uint8_t PMS_ChrgMsg_Checksum;
} PMS_ChrgMsg0_Data;

typedef struct {
	uint8_t HandShake;
	uint8_t online;
	uint32_t ctRoll;
	uint8_t tick;
} Xp_HandShake_Status;

typedef struct {
	BMS2_ChrgMsg0_Data sBMS2_ChrgMsg0_Data;
	PMS_ChrgMsg0_Data sPMS_ChrgMsg0_Data;
	Xp_HandShake_Status Xp_HandShake;
} CAN_DATA;

extern CAN_DATA gCAN_DATA;

typedef struct {
	uint8_t SysSta;
	uint8_t xp_HandShake;
	uint8_t xp_ChrgEna;
	uint8_t xp_ChrgSt;
	uint8_t xp_CVmode;
	uint8_t xpRx_CVmode;
	uint8_t Chrgr_FaultInfo;
	uint32_t Fault_Blocked;
	float xp_BatVolt;
} Sys_State;

extern Sys_State gSys_State;

typedef struct {
	uint8_t OvFault;
	uint8_t iniOk;
	uint16_t LowVoltTime;
	uint16_t NorVoltTime;
	uint32_t OvTime;
} HwProData;

typedef struct {
	float tempIn[8];
	int16_t Delayu;
	int16_t Delayd;
	uint8_t ErrStaErr;
} HwOtProData;

extern HwProData HwprotectData;
extern HwOtProData HwOtpStr;

typedef struct {
	float ACinVolRmsFir;
} pfc_DataFlow_t;

extern pfc_DataFlow_t pfc_DataFlowFace;

typedef struct {
	float SrOn1;
	float SrOff1;
	float SrOn2;
	float SrOff2;
	float SrA_s;
	float SrB_s;
	float SrD_s;
} PlecsSrWin_t;

typedef struct {
	float PwmOn1;
	float PwmOff1;
	float PwmOn2;
	float PwmOff2;
} PlecsPwmWin_t;

extern PlecsSrWin_t PlecsSrWin;
extern PlecsPwmWin_t PlecsPwmWin;

#ifdef __cplusplus
}
#endif

#endif
