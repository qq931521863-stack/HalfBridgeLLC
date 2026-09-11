#ifndef CAN_CONTROL_2800W_H
#define CAN_CONTROL_2800W_H

#include <string.h>
//#include "HwConfig.h"
#include "main.h"
//#include "fdcan.h"
#include "stm32g4xx_hal.h"
//变量类型定义
//typedef signed char             int8_t;
//typedef short int               int16_t;
//typedef int                     int32_t;
//typedef unsigned char           uint8_t;
//typedef unsigned short int      uint16_t;
//typedef unsigned int            uint32_t;

//CAN ID
#define ID_BMS_ChrgInfo            0x289
#define ID_BMS2_ChrgMsg0           0x2F7
#define ID_Chrgr_ChrgMsg0          0x2D5
#define ID_PMS_ChrgMsg0            0x2DA
#define ID_TESTER_CHRG_DiagReq     0x704
#define ID_TESTER_FunctionDiagReq  0x7DF
#define ID_Chrgr_Debug_1           0x65B
#define ID_CHRG_TESTER_DiagResp    0x784

//ID_BMS_ChrgInfo_RX
typedef struct
{
    /*Cn:BMS充电准备就绪状态
        0x0:Default
        0x1:Ready
        0x2:NotReady
        0x3:Reserve
    */
    /*En:BMS charging ready status*/
    uint8_t BMS_RdyForChrg;
    /*Cn:BMS充电状态
        0x0:NoCharge
        0x1:Charging
        0x2:ChargeFull
        0x3:BMSErrorStop
        0x4:PMSStop
        0x5-0xF:Reserve
    */
    /*En:BMS charging status*/
    uint8_t BMS_ChrgSt;
    /*Cn:BMS充电请求电流 (0.1A)*/
    /*En:BMS charging request current (0.1A)*/
    uint16_t BMS_ChrgReqCur;
    /*Cn:BMS充电请求电压 (0.1V)*/
    /*En:BMS charging request voltage (0.1V)*/
    uint16_t BMS_ChrgReqVolt;
    /*Cn:计数器*/
    /*En:counter*/
    uint8_t BMS_ChrgInfo_ctRoll;
    /*Cn:Checksum校验（XOR）*/
    /*En:Checksum verification (XOR)*/
    uint8_t BMS_ChrgInfo_Checksum;
}BMS_ChrgInfo_Data;

//ID_BMS2_ChrgMsg0_RX
typedef struct
{
    /*Cn:背包电池充电握手状态
        0x0:Default
        0x1:Ready
        0x2:NotReady
        0x3:Reserve
    */
    /*En:Backpack battery charging handshake status*/
    uint8_t BMS2_HandShake;
    /*Cn:背包电池充电使能
        0x0:Init
        0x1:Start
        0x2:Stop
        0x3:Reserve
    */
    /*En:Backpack battery charging enablement*/
    uint8_t BMS2_ChrgEna;
    /*Cn:背包电池充电请求电流 (0.1A)*/
    /*En:Backpack battery charging request current (0.1A)*/
    uint16_t BMS2_ChrgReqCur;
    /*Cn:背包电池充电请求电压 (0.1V)*/
    /*En:Backpack battery charging request voltage (0.1V)*/
    uint16_t BMS2_ChrgReqVolt;
    /*Cn:背包电池充电状态
        0x0:NoCharge
        0x1:Charging
        0x2:ChargeFull
        0x3:BMSErrorStop
        0x4:PMSStop
        0x5:ManualStop
        0x6:ChargerError
        0x7-0xF:Reserve
    */
    /*En:Backpack battery charging status*/
    uint8_t BMS2_ChrgSt;
    /*Cn:计数器*/
    /*En:counter*/
    uint8_t BMS2_ChrgMsg_ctRoll;
    /*Cn:Checksum校验（XOR）*/
    /*En:Checksum verification (XOR)*/
    uint8_t BMS2_ChrgMsg_Checksum;
}BMS2_ChrgMsg0_Data;

// typedef struct{
//     uint32_t FAULT_IN_OV :1; 
//     uint32_t FAULT_IN_UV :1; 
//     uint32_t FAULT_IN_OC :1;  
//     uint32_t FAULT_AC_OVER_FREQ :1;   
//     uint32_t FAULT_AC_UNDER_FREQ :1; 
//     uint32_t FAULT_BUS_OV :1; 
//     uint32_t FAULT_BUS_UV :1; 
//     uint32_t FAULT_OUT_OV :1;  
//     uint32_t FAULT_OUT_UV :1;  
//     uint32_t FAULT_OUT_OC :1;  
//     uint32_t FAULT_PFC_HW_ERR :1;  
//     uint32_t FAULT_LLC_HW_ERR :1;  
//     uint32_t FAULT_OUT_SHORT :1;  
//     uint32_t FAULT_PFC_OTP :1; 
//     uint32_t FAULT_LLC_OTP :1; 
//     uint32_t FAULT_BATT_REVS :1; 
//     uint32_t FAULT_IN_RLY_ERR :1; 
//     uint32_t FAULT_OUT_RLY_ERR :1; 
//     uint32_t FAULT_FAN_ERR :1; 
//     uint32_t FAULT_CAN_CMM_ERR :1;  
//     uint32_t FAULT_INSULATION_RES_Err :1; 
//     uint32_t FAULT_SCI_CMM_ERR :1; 
//     uint32_t FAULT_RES_ERR1 :1; 
//     uint32_t FAULT_RES_ERR2 :1; 
//     uint32_t FAULT_RES_ERR3 :1; 
//     uint32_t FAULT_RES_ERR4 :1; 
//     uint32_t FAULT_RES_ERR5 :1; 
//     uint32_t FAULT_RES_ERR6 :1; 
//     uint32_t FAULT_RES_ERR7 :1; 
//     uint32_t FAULT_RES_ERR8 :1; 
//     uint32_t res :2; 
    
// }Err_Data;

// typedef union {
// 	uint32_t Err;
// 	Err_Data sErr_Data;
// }Chrgr_FalutInfo;

//ID_Chrgr_ChrgMsg0_TX
typedef struct
{
    /*Cn:充电器握手状态
        0x0:Default
        0x1:Ready
        0x2:NotReady
        0x3:Reserve
    */
    /*En:Charger handshake status*/
    uint8_t Chrgr_HandShake;
    /*Cn:充电器工作状态
        0x0: Init
        0x1: Wakeup
        0x2: Stadby
        0x3: Charging
        0x4: FullCharged
        0x5: Fault
        0x6-0x7:Reserve
    */
    /*En:Charger working status*/
    uint8_t Chrgr_WorkSt;
    /*Cn:充电器直流侧输出电流(0.1A)*/
    /*En:Charger DC output current (0.1A)*/
    uint16_t Chrgr_DCOutpCur;
    /*Cn:充电器直流侧输出电压(0.1V)*/
    /*En:Charger DC output voltage (0.1V)*/
    uint16_t Chrgr_DCOutpVolt;
    /*Cn:充电器交流侧输入电流(0.1A)*/
    /*En:Charger AC input current (0.1A)*/
    uint16_t Chrgr_ACInputCur;
    /*Cn:充电器交流侧输入电压(V)*/
    /*En:Charger AC input voltage (V)*/
    uint16_t Chrgr_ACInputVolt;
    /*Cn:充电器内部温度(0.1degC)*/
    /*En:Charger internal temperature (0.1degC)*/
    uint8_t Chrgr_Temp;
    /*Cn:充电器故障信息*/
    /*En:Charger fault information*/
    uint8_t FAULT_IN_OV ; 
    uint8_t FAULT_IN_UV ; 
    uint8_t FAULT_IN_OC ;  
    uint8_t FAULT_AC_OVER_FREQ ;   
    uint8_t FAULT_AC_UNDER_FREQ ; 
    uint8_t FAULT_BUS_OV ; 
    uint8_t FAULT_BUS_UV ; 
    uint8_t FAULT_OUT_OV ;  
    uint8_t FAULT_OUT_UV ;  
    uint8_t FAULT_OUT_OC ;  
    uint8_t FAULT_PFC_HW_ERR ;  
    uint8_t FAULT_LLC_HW_ERR ;  
    uint8_t FAULT_OUT_SHORT ;  
    uint8_t FAULT_PFC_OTP ; 
    uint8_t FAULT_LLC_OTP ; 
    uint8_t FAULT_BATT_REVS ; 
    uint8_t FAULT_IN_RLY_ERR ; 
    uint8_t FAULT_OUT_RLY_ERR ; 
    uint8_t FAULT_FAN_ERR ; 
    uint8_t FAULT_CAN_CMM_ERR ;  
    uint8_t FAULT_INSULATION_RES_Err ; 
    uint8_t FAULT_SCI_CMM_ERR ; 
    uint8_t FAULT_RES_ERR1 ; 
    uint8_t FAULT_RES_ERR2 ; 
    uint8_t FAULT_RES_ERR3 ; 
    uint8_t FAULT_RES_ERR4 ; 
    uint8_t FAULT_RES_ERR5 ; 
    uint8_t FAULT_RES_ERR6 ; 
    uint8_t FAULT_RES_ERR7 ; 
    uint8_t FAULT_RES_ERR8 ; 
    /*Cn:充电器故障等级
        上报所有故障中的最高等级；
        0x0:Nofault
        0x1:Level1fault
        0x2:Level2fault
        0x3:Level3fault
        0x4:Level4fault
        0x5:Level5fault
        0x6-0xF:Reserved
    */
    /*En:Charger fault level*/
    uint8_t Chrgr_FalutLevel ;
    /*Cn:充电器降额信息
        0x0:No derate;
        0x1:Derate_high_outvoltage;
        0x2:Derate_low_outvoltage;
        0x3:Derate_board_high_temperature;
        0x4:Derate_board_low_temperature;
        0x5:Derate_air_high_temperature;
        0x6:Derate_PFC_undervoltage;
        0x7:Derate_inputvoltage;
        0x8-0xF:Reserved
    */
    /*En:Charger derate information*/
    uint8_t Chrgr_DerateInfo;
    /*Cn:bootLoader版本*/
    /*En:Charger bootLoader version*/
    char Chrgr_BootLoadRev[3];
    /*Cn:充电器app软件版本*/
    /*En:Charger appsoftware version*/
    char Chrgr_AppRev[3];
    /*Cn:充电器硬件版本*/
    /*En:Charger hardware version*/
    char Chrgr_HardWareRev[3];
    /*Cn:充电器零件号*/
    /*En:Charger part number*/
    char Chrgr_PartNumberRev1[8];
    /*Cn:充电器零件号版本号*/
    /*En:Charger part number version*/
    char Chrgr_PartNumberRev2[6];
    /*Cn:销售地区
        0x0:China
        0x1:Europe
        0x2:RegionA
        0x3:RegionB
        0x4:RegionC
        0x5:RegionD
        0x6:RegionE
        0x7:RegionF
    */
    /*En:Sales region*/
    uint8_t Chrgr_SalesRegion;
    /*Cn:计数器*/
    /*En:Charger counter*/
 volatile   uint8_t Chrgr_ChrgMsg_ctRoll;
    // /*Cn:Checksum校验（XOR）*/
    /*En:Charger Checksum verification (XOR)*/
    uint8_t Chrgr_ChrgMsg_Checksum;
}Chrgr_ChrgMsg0_Data;
extern Chrgr_ChrgMsg0_Data sChrgMsg0;
//ID_PMS_ChrgMsg0_TX
typedef struct
{
    /*Cn:PMS充电握手状态
        0x0: Default
        0x1: Ready
        0x2: NotReady
        0x3: Reserve
    */
    /*En:PMS charging handshake status*/
    uint8_t PMS_HandShake;
    /*Cn:PMS充电使能
        0x0:Init
        0x1:Start
        0x2:Stop
        0x3:Reserve
    */
    /*En:PMS charging enable*/
    uint8_t PMS_ChrgEna;
    /*Cn:PMS请求是否恒压模式
        0x0:No Request
        0x1:CV Mode Request
    */
    /*En:PMS charging request CV mode request*/
    uint8_t PMS_CVModeReq;
    /*Cn:PMS充电请求电流(0.1A)*/
    /*En:PMS charging request current (0.1A)*/
    uint16_t PMS_ChrgReqCur;
    /*Cn:PMS充电请求电压(0.1V)*/
    /*En:PMS charging request voltage (0.1V)*/
    uint16_t PMS_ChrgReqVolt;
    /*Cn:计数器*/
    /*En:PMS charging counter*/
    uint8_t PMS_ChrgMsg_ctRoll;
    /*Cn:Checksum校验（XOR）*/
    /*En:PMS charging Checksum verification (XOR)*/
    uint8_t PMS_ChrgMsg_Checksum;
}PMS_ChrgMsg0_Data;

//ID_TESTER_CHRG_DiagReq_TX
typedef struct
{
    /*Cn:诊断请求信号占位*/
    /*En:Diagnostic request signal placeholder*/
    uint8_t TESTER_CHRG_DiagReq_Sig;
}TESTER_CHRG_DiagReq_Data;

//ID_TESTER_FunctionDiagReq_TX
typedef struct
{
    /*Cn:诊断请求信号占位*/
    /*En:Diagnostic request signal placeholder*/
    uint8_t TETSER_FunctionDiagReq_Sig;
}TESTER_FunctionDiagReq_Data;

//ID_Chrgr_Debug_1_TX
typedef struct
{
    /*Cn:时间戳*/
    /*En:Charger debug signal*/
    uint32_t Chrgr_TimeStamp;
    /*Cn:时间毫秒刻度*/
    /*En:Time millisecond scale*/
    uint16_t Chrgr_Ms;
    /*Cn:充电机状态
        0x0:Initial_state
        0x1:ConstantVoltWackup_state
        0x2:ChargeStandby_state
        0x3:Charging_state
        0x4:FullCharged_state
        0x5:Fault_state
    */
    /*En:Charger state*/
    uint8_t Chrgr_State ;
    /*Cn:PFC状态
        0x0:Initial_state
        0x1:SoftStart_state
        0x2:RestrictionCurrent_state
        0x3:Run_state
        0x4:Fault_state
    */
    /*En:PFC state*/
    uint8_t Chrgr_PfcState ;
    /*Cn:LLC状态
        0x0:Initial_state
        0x1:SoftStart_state
        0x2:ConstantVoltWackup_state
        0x3:ConstantCurrent_state
        0x4:ResrictiontCurrent_state
        0x5:Fault_state
    */
    /*En:LLC state*/
    uint8_t Chrgr_LlcState ;
    /*Cn:输入PFC继电器使能*/
    /*En:Input PFC relay enable*/
    uint8_t Chrgr_InPfcRelayEnable ;
    /*Cn:输入PFC使能*/
    /*En:Input PFC enable*/
    uint8_t Chrgr_PfcEn ;
    /*Cn:PFC间歇模式使能*/
    /*En:PFC burst mode enable*/
    uint8_t Chrgr_PfcBurstEn ;
    /*Cn:输出继电器使能*/
    /*En:Output relay enable*/
    uint8_t Chrgr_OutLlcRelay ;
    /*Cn:PFC正常运行信号*/
    /*En:PFC normal operation signal*/
    uint8_t Chrgr_PfcOk ;
    /*Cn:输出电容放电MOS使能*/
    /*En:Output capacitor discharge MOS enable*/
    uint8_t Chrgr_DisChrgEn ;
    /*Cn:LLC间歇模式使能*/
    /*En:LLC burst mode enable*/
    uint8_t Chrgr_LlcBurstEn ;
    /*Cn:PFC辅源电压值*/
    /*En:PFC auxiliary voltage value*/
    uint8_t Chrgr_PfcAux ;
    /*Cn:输入频率*/
    /*En:Input frequency value*/
    uint8_t Chrgr_InAcFre ;
    /*Cn:输入交流电压直流分量*/
    /*En:Input AC voltage DC component value*/
    uint8_t Chrgr_InDcComp ;
    /*Cn:母线电压平均值*/
    /*En:Bus voltage average value*/
    uint16_t Chrgr_BusAveVolt ;
    /*Cn:LLC辅源电压值*/
    /*En:LLC auxiliary voltage value*/
    uint8_t Chrgr_LlcAux ;
    /*Cn:输出继电器前端电压值*/
    /*En:Output relay front voltage value*/
    uint16_t Chrgr_OutRelayVolt ;
    /*Cn:次级LLC工作频率*/
    /*En:LLC working frequency value*/
    uint16_t Chrgr_LlcFreq ;
    /*Cn:风扇驱动占空比*/
    /*En:Fan duty cycle value*/
    uint8_t Chrgr_FanDuty ;
    /*Cn:风扇1的电流值*/
    /*En:Fan 1 current value*/
    uint8_t Chrgr_Fan1Cur ;
    /*Cn:风扇2的电流值*/
    /*En:Fan 2 current value*/
    uint8_t Chrgr_Fan2Cur ;
    /*Cn:风扇3的电流值*/
    /*En:Fan 3 current value*/
    uint8_t Chrgr_Fan3Cur ;
    /*Cn:风扇4的电流值*/
    /*En:Fan 4 current value*/
    uint8_t Chrgr_Fan4Cur ;
    /*Cn:PFC散热片温度*/
    /*En:PFC heat sink temperature value*/
    uint8_t Chrgr_PfcHeatSinkTemp ;
    /*Cn:PFC电感温度*/
    /*En:PFC inductance temperature value*/
    uint8_t Chrgr_PfcInductanceTemp ;
    /*Cn:变压器温度*/
    /*En:Transformer temperature value*/
    uint8_t Chrgr_TransformerTemp ;
    /*Cn:LLC散热片A组温度*/
    /*En:LLC heat sink A group temperature value*/
    uint8_t Chrgr_LlcHeatSinkTempA ;
    /*Cn:LLC散热片B组温度*/
    /*En:LLC heat sink B group temperature value*/
    uint8_t Chrgr_LlcHeatSinkTempB ;
    /*Cn:DC接口温度*/
    /*En:DC interface temperature value*/
    uint8_t Chrgr_DcInterfaceTemp ;
    /*Cn:空气温度*/
    /*En:Air temperature value*/
    uint8_t Chrgr_AirTemp ;
    /*Cn:计数器*/
    /*En:Counter*/
    uint8_t PMS_ChrgMsg_ctRoll ;
    /*Cn:Checksum校验(XOR)*/
    /*En:Checksum verification (XOR)*/
    uint8_t PMS_ChrgMsg_Checksum ;
}Chrgr_Debug_1_Data;

//ID_CHRG_TESTER_DiagResp_TX
typedef struct
{
    /*Cn:诊断请求信号占位*/
    /*En:Diagnostic response signal placeholder*/
    uint8_t CHRG_TESTER_DiagResp_Sig;
}CHRG_TESTER_DiagResp_Data;

typedef struct{
    uint8_t HandShake;   //bit0 : bms bit1 : PMS
	uint8_t online;		 //bit0 : bms bit1 : PMS
    uint32_t ctRoll;
    uint8_t tick;       //send tick
}Xp_HandShake_Status;

//ID_CAN_RX_DATA
typedef struct
{
    //BMS_ChrgInfo_Data sBMS_ChrgInfo_Data;
    BMS2_ChrgMsg0_Data sBMS2_ChrgMsg0_Data;
    PMS_ChrgMsg0_Data  sPMS_ChrgMsg0_Data;
    Xp_HandShake_Status Xp_HandShake;
}CAN_DATA;

extern CAN_DATA gCAN_DATA;

/*******************************************************************************
 * @fn        : void initCANFDData(void)
 * @brief     : Initialize CANFD data
 * @param     : void
 * @return    : void
 * @example   : initCANFDData();
 * @date      : 2026/02/06
 ******************************************************************************/
extern void initCANFDData(void);

/*******************************************************************************
 * @fn        : uint8_t parse_ID_BMS2_ChrgMsg0_Data(uint8_t Len,uint8_t *RxData)
 * @brief     : Parse ID_BMS2_ChrgMsg0_Data
 * @param     : void
 * @return    : void
 * @example   : parse_ID_BMS2_ChrgMsg0_Data(Len,RxData);
 * @date      : 2026/02/06
 ******************************************************************************/
extern uint8_t parse_ID_BMS2_ChrgMsg0_Data(uint8_t Len,uint8_t *RxData);

/*******************************************************************************
 * @fn        : void parse_ID_PMS_ChrgMsg0_Data(uint8_t *RxData)
 * @brief     : Parse ID_PMS_ChrgMsg0_Data
 * @param     : void
 * @return    : void
 * @example   : parse_ID_PMS_ChrgMsg0_Data(RxData);
 * @date      : 2026/02/06
 ******************************************************************************/
extern uint8_t parse_ID_PMS_ChrgMsg0_Data(uint8_t Len,uint8_t *RxData);

/*******************************************************************************
 * @fn        : void send_ID_Chrgr_ChrgMsg0_Data(Chrgr_ChrgMsg0_Data *chrgr_ChrgMsg0_Data)
 * @brief     : Send ID_Chrgr_ChrgMsg0_Data
 * @param     : Chrgr_ChrgMsg0_Data *chrgr_ChrgMsg0_Data
 * @return    : void
 * @example   : send_ID_Chrgr_ChrgMsg0_Data(&chrgr_ChrgMsg0_Data);
 * @date      : 2026/02/06
 ******************************************************************************/
extern void send_ID_Chrgr_ChrgMsg0_Data(Chrgr_ChrgMsg0_Data *chrgr_ChrgMsg0_Data);

/*******************************************************************************
 * @fn        : void send_ID_TESTER_CHRG_DiagReq_Data(TESTER_CHRG_DiagReq_Data *tESTER_CHRG_DiagReq_Data)
 * @brief     : Send ID_TESTER_CHRG_DiagReq_Data
 * @param     : TESTER_CHRG_DiagReq_Data *tESTER_CHRG_DiagReq_Data
 * @return    : void
 * @example   : send_ID_TESTER_CHRG_DiagReq_Data(&tESTER_CHRG_DiagReq_Data);
 * @date      : 2026/02/06
 ******************************************************************************/
extern void send_ID_TESTER_CHRG_DiagReq_Data(TESTER_CHRG_DiagReq_Data *tESTER_CHRG_DiagReq_Data);

/*******************************************************************************
 * @fn        : void send_ID_TESTER_FunctionDiagReq_Data(TESTER_FunctionDiagReq_Data *tESTER_FunctionDiagReq_Data)
 * @brief     : Send ID_TESTER_FunctionDiagReq_Data
 * @param     : TESTER_FunctionDiagReq_Data *tESTER_FunctionDiagReq_Data
 * @return    : void
 * @example   : send_ID_TESTER_FunctionDiagReq_Data(&tESTER_FunctionDiagReq_Data);
 * @date      : 2026/02/06
 ******************************************************************************/
extern void send_ID_TESTER_FunctionDiagReq_Data(TESTER_FunctionDiagReq_Data *tESTER_FunctionDiagReq_Data);

/*******************************************************************************
 * @fn        : void send_ID_Chrgr_Debug_1_Data(Chrgr_Debug_1_Data *chrgr_Debug_1_Data)
 * @brief     : Send ID_Chrgr_Debug_1_Data
 * @param     : Chrgr_Debug_1_Data *chrgr_Debug_1_Data
 * @return    : void
 * @example   : send_ID_Chrgr_Debug_1_Data(&chrgr_Debug_1_Data);
 * @date      : 2026/02/06
 ******************************************************************************/
extern void send_ID_Chrgr_Debug_1_Data(Chrgr_Debug_1_Data *chrgr_Debug_1_Data);

/*******************************************************************************
 * @fn        : void send_ID_CHRG_TESTER_DiagResp_Data(CHRG_TESTER_DiagResp_Data *chrgr_Tester_DiagResp_Data)
 * @brief     : Send ID_CHRG_TESTER_DiagResp_Data
 * @param     : CHRG_TESTER_DiagResp_Data *chrgr_Tester_DiagResp_Data
 * @return    : void
 * @example   : send_ID_CHRG_TESTER_DiagResp_Data(&chrgr_Tester_DiagResp_Data);
 * @date      : 2026/02/06
 ******************************************************************************/
extern void send_ID_CHRG_TESTER_DiagResp_Data(CHRG_TESTER_DiagResp_Data *chrgr_Tester_DiagResp_Data);

extern void CAN_SendData_Run(void);
#endif
