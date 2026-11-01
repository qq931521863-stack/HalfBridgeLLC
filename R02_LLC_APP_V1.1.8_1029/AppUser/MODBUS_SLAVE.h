#ifndef MODBUS_SLAVE_H
#define MODBUS_SLAVE_H
#include "HwConfig.h"
#include <string.h>

#define pfc_id 0x01
#define llc_id 0xA1

/* LLC USART3 ↔ PFC USART1 业务串口（与从机 MODBUS_SLAVE.h 保持一致） */
#define PFC_LLC_UART_BAUD         115200U
#define PFC_LLC_RX_DMA_LEN        86U

#define BUFFER_SIZE 100

typedef struct
{
	uint8_t Stuts;                //0:空闲 1：恒压 2：恒流
	float outVolt;              //0.25V	
	float outCurr;              //故障等级
}STATUS_LLC;


//状态机枚举量
typedef enum
{
  init,  //初始化
  Wait,  //空闲等待
  Rise,  //软启
	Derate,//限流状态
  Run,   //正常运行 
  Err    //故障
}pfc_STATE_M;

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
	 uint32_t outRLY_Err: 1 ;	
   uint32_t FAN_Err   : 1 ;
   uint32_t CAN_Err   : 1 ;
   uint32_t INS_Err   : 1 ;
   uint32_t SCI_Err   : 1 ;
	 uint32_t ResonOc   : 1 ;
	 uint32_t InterOv   : 1 ;	 	 
	 
   uint32_t res       : 8 ;		
}pfc_FAULT_BIT_T;

typedef union {
	uint32_t     all;
	pfc_FAULT_BIT_T  bit;
}pfc_FAULT_STA_T;

typedef struct 
{      
	pfc_STATE_M      RunState;    //运行状态机
	pfc_FAULT_STA_T  FaultSta;	  //故障标志
	uint8_t      PfcState;    //PFC状态
	uint8_t      RelaySta;	  //继电器状态
	uint8_t      PwmSta;      //驱动允许信号
	uint8_t      ssFinsh;     //软启动
	uint8_t      Zero;        //过零
	uint8_t      VinDcFlag;   //直流标志
	uint8_t      Mrise;       //启动信号
	uint8_t      PFC_ok;      //pfc工作正常
	uint8_t      LLC_en;      //LLC允许信号
	uint8_t      Negative;    //负半波
	float        OutVolt;     //LLC输出电压
	float        VbusSetVol;  //母线设置电压
	float        Vbus_ref;    //母线参考电压
	float        VbusVolt;    //母线实际电压
	float        ACinFlv;     //交流频率
	float        ACinFreFir;  //滤波后的频率	
	float        ACinVolRMS;  //交流电压有效值
	float        ACinVolRmsFir;  //交流电压有效值
	float        ACinCurRMS;  //交流电流有效值
	float        ACinCurRmsFir;  //交流电流有效值
	float        ACinPower;   //交流输入功率	
	float        ACinMaxPow;  //最大交流输入功率 用于过温降额
	float		     pfcAux;
	float 		   pfcInDcComp; 
	
	float        pfcTransformerTemp;     //PFC 变压器温度
	float		     pfcHeatSinkTemp;        //散热器温度
	float		     pfcInductanceTemp;      //电感温度

} pfc_DataFlow_t;
extern pfc_DataFlow_t  pfc_DataFlowFace;

extern uint8_t Rxbuffer_usat3[BUFFER_SIZE];
extern STATUS_LLC DataLLC;
extern void Modbus_Slave_Rx(uint8_t *RxData ,uint16_t Len);
extern void Send_to_Pfc_info(void);
extern void overTime_pfc(void);
extern void InitModbusData(void);
#endif

