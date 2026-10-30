#include "operateStatus.h"
#include <string.h>
#include "CAN_Control_2800W.h"
#include "HwConfig.h"
#include "UpgradeFeatureConfig.h"
#if OTA_UPGRADE_ENABLE
#include "OtaGw.h"
#endif
Sys_State gSys_State;

uint16_t StaDelayTime = 0;

// 函数声明
static void Initial_state_Function(void);
static void Initial_state_Switch(void);
static void ConstantVoltWackup_state_Function(void);
static void ConstantVoltWackup_state_Switch(void);
static void ChargeStandby_state_Function(void);
static void ChargeStandby_state_Switch(void);
static void Charging_state_Function(void);
static void Charging_state_Switch(void);
static void FullCharged_state_Function(void);
static void FullCharged_state_Switch(void);
static void Fault_state_Function(void);
static void Fault_state_Switch(void);

void get_CAN_States(void){
	
	if(gCAN_DATA.Xp_HandShake.HandShake == 3){
		gCAN_DATA.Xp_HandShake.HandShake = 2;
	}
	
    gSys_State.xp_HandShake = gCAN_DATA.Xp_HandShake.HandShake; //握手状态
	
	if(gCAN_DATA.sPMS_ChrgMsg0_Data.PMS_ChrgEna == 0x01 || gCAN_DATA.sBMS2_ChrgMsg0_Data.BMS2_ChrgEna == 0x01){
		gSys_State.xp_ChrgEna = 0x01;   //充电使能状态
	}else{
		gSys_State.xp_ChrgEna = 0x00;   //充电使能状态
	}
	
	if(gCAN_DATA.Xp_HandShake.online == 3){
		//双信号在线，保持充电使能为0
		gSys_State.xp_ChrgEna = 0x00;   //充电使能状态
	}
    
    gSys_State.xp_ChrgSt = gCAN_DATA.sBMS2_ChrgMsg0_Data.BMS2_ChrgSt; //电池状态
//    gSys_State.xp_CVmode = 0x00; //CV模式开关
	  gSys_State.xpRx_CVmode = gCAN_DATA.sPMS_ChrgMsg0_Data.PMS_CVModeReq; //接收到的CV模式请求
	
	//错误标志
	if(DataFlowFace.FaultSta.bit.ACin_oc == 1 
		|| DataFlowFace.FaultSta.bit.Vbus_ov == 1
		|| DataFlowFace.FaultSta.bit.Out_uv == 1
		|| DataFlowFace.FaultSta.bit.Out_oc == 1
		|| DataFlowFace.FaultSta.bit.PFC_Err == 1
		|| DataFlowFace.FaultSta.bit.LLC_Err == 1
		|| DataFlowFace.FaultSta.bit.Out_short == 1
		|| DataFlowFace.FaultSta.bit.outRLY_Err == 1
		|| DataFlowFace.FaultSta.bit.inRLY_Err == 1
		|| DataFlowFace.FaultSta.bit.CAN_Err == 1
		|| DataFlowFace.FaultSta.bit.SCI_Err == 1
		|| DataFlowFace.FaultSta.bit.ResonOc == 1
		|| DataFlowFace.FaultSta.bit.LLC_otp == 1
		|| DataFlowFace.FaultSta.bit.PFC_otp == 1
		|| DataFlowFace.FaultSta.bit.InterOv == 1){
			gSys_State.Chrgr_FaultInfo = 1;
		}else{
			gSys_State.Chrgr_FaultInfo = 0;
		}
	  
    gSys_State.xp_BatVolt = ADSample_Info.vOut_Bat_FIR;   //电池电压
}

/*******************************************************************************
 * @fn        :Sys_States_API SysStates[]
 * @brief     :充电机状态逻辑API注册表，根据状态执行对应状态逻辑函数和状态切换判断函数
 * @param     :null
 * @return    :null
 * @example   :
 * @date      :2026.4.13
 ******************************************************************************/
const Sys_States_API SysStates[] = {
	{Initial_state,Initial_state_Function,Initial_state_Switch},
	{ConstantVoltWackup_state,ConstantVoltWackup_state_Function,ConstantVoltWackup_state_Switch},
	{ChargeStandby_state,ChargeStandby_state_Function,ChargeStandby_state_Switch},
	{Charging_state,Charging_state_Function,Charging_state_Switch},
	{FullCharged_state,FullCharged_state_Function,FullCharged_state_Switch},
	{Fault_state,Fault_state_Function,Fault_state_Switch},
};

#define Sys_States_Len  (sizeof(SysStates) / sizeof(SysStates[0]))

/*******************************************************************************
 * @fn        :void SysStatesInit(void)
 * @brief     :充电状态初始化
 * @param     :null
 * @return    :null
 * @example   :
 * @date      :2026.4.13
 ******************************************************************************/
void SysStatesInit(void) {
	memset(&gSys_State, 0, sizeof(Sys_State));
}

/*******************************************************************************
 * @fn        :void runMainStateMachine(void) 
 * @brief     :充电状态机，充电状态的切换判断和状态逻辑执行
 * @param     :null
 * @return    :null
 * @example   :
 * @date      :2026.4.13
 ******************************************************************************/
void runMainStateMachine(void) {
    get_CAN_States();

#if OTA_UPGRADE_ENABLE
	if (s_eHandoffState > 0) {
		gSys_State.SysSta = Ota_state;
		return;
	}
#endif

	if (SysStates[gSys_State.SysSta].Function != NULL) {
		SysStates[gSys_State.SysSta].Function();      // 执行当前状态的功能
	}
	if (SysStates[gSys_State.SysSta].SwitchState != NULL) {
		SysStates[gSys_State.SysSta].SwitchState();   // 执行状态切换判断
	}
	if(Delay1ms.delay_ini >= 100)
	{
		if(DataFlowFace.RunState != gSys_State.SysSta)DataFlowFace.OldState = Init;
		DataFlowFace.RunState = gSys_State.SysSta;
	}else
	{
		DataFlowFace.RunState = Init;
	}	
}

/*******************************************************************************
 * @fn        :static void Initial_state_Function(void)
 * @brief     :初始状态功能执行函数
 * @param     :
 * @return    :
 * @example   :
 * @date      :
 ******************************************************************************/
static void Initial_state_Function(void) {
//	StaDelayTime++;
}

/*******************************************************************************
 * @fn        :static void Initial_state_Switch(void)
 * @brief     :初始状态切换模式判断
 * @param     :
 * @return    :
 * @example   :
 * @date      :
 ******************************************************************************/
static void Initial_state_Switch(void) {
	//&& StaDelayTime >= 3 * 200
	if (gSys_State.Chrgr_FaultInfo != 0) {
		//进入错误状态
		gSys_State.SysSta = Fault_state;
	}
	else if (gSys_State.xp_HandShake == 0 ) {
		//进入激活状态
		gSys_State.SysSta = ConstantVoltWackup_state;
	}
	else if (gSys_State.xp_HandShake != 0) {
		//进入预充状态
		gSys_State.SysSta = ChargeStandby_state;
	}
}

/*******************************************************************************
 * @fn        :static void ConstantVoltWackup_state_Function(void)
 * @brief     :唤醒状态功能执行函数
 * @param     :
 * @return    :
 * @example   :
 * @date      :
 ******************************************************************************/
static void ConstantVoltWackup_state_Function(void) {
	
}

/*******************************************************************************
 * @fn        :static void ConstantVoltWackup_state_Switch(void)
 * @brief     :唤醒状态切换模式判断
 * @param     :
 * @return    :
 * @example   :
 * @date      :
 ******************************************************************************/
static void ConstantVoltWackup_state_Switch(void) {
	if (gSys_State.Chrgr_FaultInfo != 0) {
		//进入错误状态
		gSys_State.SysSta = Fault_state;
	}
	else if (gSys_State.xp_HandShake != 0) {
		//进入预充状态
		gSys_State.SysSta = ChargeStandby_state;
	}
	
	if(gCAN_DATA.Xp_HandShake.online == 3){
		//进入预充状态
		gSys_State.SysSta = ChargeStandby_state;
	}
}

/*******************************************************************************
 * @fn        :static void ChargeStandby_state_Function(void)
 * @brief     :充电待机功能执行函数
 * @param     :
 * @return    :
 * @example   :
 * @date      :
 ******************************************************************************/
static void ChargeStandby_state_Function(void) {
	
}

/*******************************************************************************
 * @fn        :static void ChargeStandby_state_Switch(void)
 * @brief     :充电待机状态切换模式判断
 * @param     :
 * @return    :
 * @example   :
 * @date      :
 ******************************************************************************/
static void ChargeStandby_state_Switch(void) {
	if (gSys_State.Chrgr_FaultInfo != 0) {
		//进入错误状态
		gSys_State.SysSta = Fault_state;
	}
	else if(gSys_State.xp_HandShake == 0x02 && gSys_State.xp_ChrgEna == 0x01 ){		
		//PMS模式下进入充电状态
		gSys_State.SysSta = Charging_state;
	}else if(gSys_State.xp_HandShake == 0x01 && gSys_State.xp_ChrgEna == 0x01 ){
		//BMS模式下进入充电状态
		gSys_State.SysSta = Charging_state;
	}else if (gSys_State.xp_HandShake == 0) {
		//进入激活状态
		gSys_State.SysSta = ConstantVoltWackup_state;
	}
	
	if(gCAN_DATA.Xp_HandShake.online == 3){
		//进入预充状态
		gSys_State.SysSta = ChargeStandby_state;
	}
	//有握手无使能也不切换状态
}

/*******************************************************************************
 * @fn        :static void Charging_state_Function(void)
 * @brief     :充电状态功能执行函数
 * @param     :
 * @return    :
 * @example   :
 * @date      :
 ******************************************************************************/
static void Charging_state_Function(void) {
	//切换CV or CC  机器人模式
	if(gSys_State.xp_HandShake == 0x02){
		if(gSys_State.xpRx_CVmode == 0 && gCAN_DATA.sPMS_ChrgMsg0_Data.PMS_ChrgReqCur >= 1.0f){
			gSys_State.xp_CVmode = 1;	// CC;
		}else if(gSys_State.xpRx_CVmode == 1 && gCAN_DATA.sPMS_ChrgMsg0_Data.PMS_ChrgReqVolt >= 30.0f){
			gSys_State.xp_CVmode = 2;	// CV;
		}else{
			gSys_State.xp_CVmode = 0;	// 关闭输出;
		}
	}else{
		 gSys_State.xp_CVmode = 1;	// 关闭输出;
	}
}

/*******************************************************************************
 * @fn        :static void Charging_state_Switch(void)
 * @brief     :充电状态切换模式判断
 * @param     :
 * @return    :
 * @example   :
 * @date      :
 ******************************************************************************/
static void Charging_state_Switch(void) {
	if(gSys_State.Chrgr_FaultInfo != 0){
		//进入错误状态
		gSys_State.SysSta = Fault_state;
		
	}else if((gSys_State.xp_HandShake == 0x02 || gSys_State.xp_HandShake == 0x01) && gSys_State.xp_ChrgEna == 1){

	}else if(gSys_State.xp_HandShake == 0x02 && gSys_State.xp_ChrgEna == 0){
		//进入预充状态	机器人模式
		gSys_State.SysSta = ChargeStandby_state;
	}else if(gSys_State.xp_HandShake == 0x01 && gSys_State.xp_ChrgEna == 0 ){
		//进入预充状态  电池模式
		gSys_State.SysSta = ChargeStandby_state;
	}
	else if(gCAN_DATA.Xp_HandShake.online == 0x00){
		//断通讯 根据电池电压进入空闲 or 唤醒
		if(gSys_State.xp_BatVolt > 30.0f){
			//进入错误状态
			gSys_State.SysSta = Fault_state;
			DataFlowFace.FaultSta.bit.CAN_Err = 1;
		}else{
			//进入激活状态
			gSys_State.SysSta = ConstantVoltWackup_state;
		}
	}else if(gSys_State.xp_HandShake == 0x00){
		//进入激活状态
		gSys_State.SysSta = ConstantVoltWackup_state;
	}
	
	if(gCAN_DATA.Xp_HandShake.online == 3){
		//进入预充状态
		gSys_State.SysSta = ChargeStandby_state;
	}
	
	//只要不在充电状态都清空CV模式判断
	if(gSys_State.SysSta != Charging_state){
		gSys_State.xp_CVmode = 0;
	}
	
}

/*******************************************************************************
 * @fn        :static void FullCharged_state_Function(void)
 * @brief     :充满状态功能执行函数
 * @param     :
 * @return    :
 * @example   :
 * @date      :
 ******************************************************************************/
static void FullCharged_state_Function(void) {
	
}

/*******************************************************************************
 * @fn        :static void FullCharged_state_Switch(void)
 * @brief     :满电状态切换模式判断
 * @param     :
 * @return    :
 * @example   :
 * @date      :
 ******************************************************************************/
static void FullCharged_state_Switch(void) {
	if (gSys_State.Chrgr_FaultInfo != 0) {
		//进入错误状态
		gSys_State.SysSta = Fault_state;
	}
//	else if (gSys_State.xp_HandShake == 0x00 || gSys_State.xp_ChrgSt != 2) {
//		//进入预充状态
//		gSys_State.SysSta = ChargeStandby_state;
//	}
	else if (gSys_State.xp_HandShake == 0x00 ) {
		//进入预充状态
		gSys_State.SysSta = ChargeStandby_state;
	}else if(gCAN_DATA.Xp_HandShake.online == 3){
		//进入预充状态
		gSys_State.SysSta = ChargeStandby_state;
	}
	
}
/*******************************************************************************
 * @fn        :static void Fault_state_Function(void)
 * @brief     :故障状态功能执行函数
 * @param     :
 * @return    :
 * @example   :
 * @date      :
 ******************************************************************************/
static void Fault_state_Function(void) {
	
}

/*******************************************************************************
 * @fn        :static void Fault_state_Switch(void)
 * @brief     :故障状态切换模式判断
 * @param     :
 * @return    :
 * @example   :
 * @date      :
 ******************************************************************************/
static void Fault_state_Switch(void) {
	if (gSys_State.xp_HandShake != 0 && gSys_State.Chrgr_FaultInfo == 0) {
		//进入预充状态
		gSys_State.SysSta = ChargeStandby_state;
	}else if (gSys_State.Chrgr_FaultInfo == 0) {
		//进入激活状态
		gSys_State.SysSta = ConstantVoltWackup_state;
	}
}
