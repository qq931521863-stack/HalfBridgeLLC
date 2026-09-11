/**
 * @file        MODBUS_SLAVE.c
 * @brief       Modbus Protocol Slave Framework Program
 * @details     
 * 
 * @author      KJW
 * @date        2026-02-09
 * @version     v1.0.0
 * 
 * @copyright   (c) 2024 Your Company
 * 
 * @note        Before using：
 *             
 * @warning     
 * 
 * @changlog:
 * - v1.0.0 2026-02-09
 *   - Initial version
 */
#include "MODBUS_SLAVE.h"
#include "UpgradeFeatureConfig.h"
#if OTA_UPGRADE_ENABLE
#include "LlcOta.h"
#include "OtaGw.h"
#endif


STATUS_LLC DataLLC;
pfc_DataFlow_t  pfc_DataFlowFace;

uint8_t Rxbuffer_usat3[BUFFER_SIZE] = {0};
uint8_t LLcData[12] = {0};

volatile uint8_t overU = 0;
volatile uint32_t lastRxPfctime = 0;

/*******************************************************************************
 * @fn        : void SendData_API(uint8_t *Data,uint16_t Len)
 * @brief     : Modbus Master Send Data API
 * @param     : Data - Pointer to the data to be sent
 *              Len - Length of the data to be sent
 * @return    : null
 * @example   : null
 * @author    : KJW
 * @date      : 2026-02-09
 ******************************************************************************/

void SendData_API(uint8_t *Data,uint16_t Len){
//	HAL_UART_Transmit_DMA(&huart3, Data, Len);
//	__HAL_DMA_DISABLE(&hdma_usart3_tx);
//	hdma_usart3_tx.Instance->CNDTR = Len;
//	hdma_usart3_tx.Instance->CPAR = (uint32_t)&huart3.Instance->TDR;
//	hdma_usart3_tx.Instance->CMAR = (uint32_t)&Data;
//	__HAL_DMA_ENABLE(&hdma_usart3_tx);
//	__HAL_UART_CLEAR_FLAG(&huart3, UART_CLEAR_TCF);
//	huart3.Instance->CR3 |= 0x80;		
//	__HAL_UART_ENABLE_IT(&huart3, UART_IT_IDLE);
//	huart3.Instance->CR3 &= 0xFFFFFF7F;	
//	HAL_DMA_Start(&hdma_usart3_tx, (uint32_t)&huart3.Instance->TDR,(uint32_t)&Data, Len);
//	huart3.Instance->CR3 |= 0x80;	
	
	huart3.Instance->CR3 &= 0xFFFFFF7F;	
	__HAL_DMA_DISABLE(&hdma_usart3_tx);
	hdma_usart3_tx.Instance->CNDTR = Len;
	hdma_usart3_tx.Instance->CPAR = (uint32_t)&huart3.Instance->TDR;
	hdma_usart3_tx.Instance->CMAR = (uint32_t)Data;
//	HAL_DMA_Start(&hdma_usart1_tx, (uint32_t)&Data,(uint32_t)&huart1.Instance->TDR, Len);
	__HAL_DMA_ENABLE(&hdma_usart3_tx);
	huart3.Instance->CR3 |= 0x80;			
}


/*******************************************************************************
 * @fn        : uint16_t Modbus_CRC16(uint8_t *data, uint16_t length)
 * @brief     : Modbus CRC16
 * @param     : data - Pointer to the Data
 *              length - Length of the Data
 * @return    : 16bit CRC
 * @example   : Modbus_CRC16(data,length);
 * @author    : KJW
 * @date      : 2026-02-09
 ******************************************************************************/
uint16_t Modbus_CRC16(uint8_t *data, uint16_t length) {
    uint16_t crc = 0xFFFF;  // 初始值
    uint16_t i, j;
    
    for (i = 0; i < length; i++) {
        crc ^= data[i];  // 异或数据字节
        
        for (j = 0; j < 8; j++) {
            if (crc & 0x0001) {  // 如果最低位为1
                crc >>= 1;       // 右移一位
                crc ^= 0xA001;   // 异或多项式(反向)
            } else {
                crc >>= 1;       // 只右移一位
            }
        }
    }
    
    return crc;  // 返回CRC值
}

/*******************************************************************************
 * @fn        : void InitModbusData(void)
 * @brief     : Init Modbus Data
 * @param     : null
 * @return    : null
 * @example   : InitModbusData();
 * @author    : KJW
 * @date      : 2026-02-09
 ******************************************************************************/
void InitModbusData(void){
  memset(&DataLLC,0,sizeof(STATUS_LLC));
	memset(&pfc_DataFlowFace,0,sizeof(pfc_DataFlow_t));
	huart3.Instance->CR3 &= 0xFFFFFFBF;	
	__HAL_UART_ENABLE_IT(&huart3, UART_IT_IDLE);
	__HAL_DMA_DISABLE(&hdma_usart3_rx);
		hdma_usart3_rx.Instance->CNDTR = PFC_LLC_RX_DMA_LEN;
		hdma_usart3_rx.Instance->CPAR = (uint32_t)&huart3.Instance->RDR;
		hdma_usart3_rx.Instance->CMAR = (uint32_t)&Rxbuffer_usat3;	
	//HAL_DMA_Start(&hdma_usart3_rx, (uint32_t)&huart3.Instance->RDR,(uint32_t)&Rxbuffer_usat3, BUFFER_SIZE);
	__HAL_DMA_ENABLE(&hdma_usart3_rx);
	huart3.Instance->CR3 |= 0x40;	
}

// 联合体用于 float 重组
typedef union {
    float f; 
    uint8_t bytes[4];
} FloatBytes;

void rx_pfc_data(uint8_t *rxdata,uint8_t len){
	  uint8_t idx = 0;
	
	  idx+=1;
    pfc_DataFlowFace.RunState = rxdata[idx++];
    pfc_DataFlowFace.FaultSta.all = ((uint32_t)rxdata[idx] << 24) |
                            ((uint32_t)rxdata[idx+1] << 16) |
                            ((uint32_t)rxdata[idx+2] << 8)  |
                            (uint32_t)rxdata[idx+3];
    idx += 4;
    pfc_DataFlowFace.PfcState   = rxdata[idx++];
    pfc_DataFlowFace.RelaySta   = rxdata[idx++];
    pfc_DataFlowFace.PwmSta     = rxdata[idx++];
    pfc_DataFlowFace.ssFinsh    = rxdata[idx++];
    pfc_DataFlowFace.Zero       = rxdata[idx++];
    pfc_DataFlowFace.VinDcFlag  = rxdata[idx++];
    pfc_DataFlowFace.Mrise      = rxdata[idx++];
    pfc_DataFlowFace.PFC_ok     = rxdata[idx++];
    pfc_DataFlowFace.LLC_en     = rxdata[idx++];
    pfc_DataFlowFace.Negative   = rxdata[idx++];

    FloatBytes fb;

    // OutVolt
    fb.bytes[3] = rxdata[idx++];
    fb.bytes[2] = rxdata[idx++];
    fb.bytes[1] = rxdata[idx++];
    fb.bytes[0] = rxdata[idx++];
    pfc_DataFlowFace.OutVolt = fb.f;



    // VbusSetVol
    fb.bytes[3] = rxdata[idx++];
    fb.bytes[2] = rxdata[idx++];
    fb.bytes[1] = rxdata[idx++];
    fb.bytes[0] = rxdata[idx++];
    pfc_DataFlowFace.VbusSetVol = fb.f;

    // Vbus_ref
    fb.bytes[3] = rxdata[idx++];
    fb.bytes[2] = rxdata[idx++];
    fb.bytes[1] = rxdata[idx++];
    fb.bytes[0] = rxdata[idx++];
    pfc_DataFlowFace.Vbus_ref = fb.f;

    // VbusVolt
    fb.bytes[3] = rxdata[idx++];
    fb.bytes[2] = rxdata[idx++];
    fb.bytes[1] = rxdata[idx++];
    fb.bytes[0] = rxdata[idx++];
    pfc_DataFlowFace.VbusVolt = fb.f;

    // ACinFlv
    fb.bytes[3] = rxdata[idx++];
    fb.bytes[2] = rxdata[idx++];
    fb.bytes[1] = rxdata[idx++];
    fb.bytes[0] = rxdata[idx++];
    pfc_DataFlowFace.ACinFlv = fb.f;

    // ACinFreFir
    fb.bytes[3] = rxdata[idx++];
    fb.bytes[2] = rxdata[idx++];
    fb.bytes[1] = rxdata[idx++];
    fb.bytes[0] = rxdata[idx++];
    pfc_DataFlowFace.ACinFreFir = fb.f;

    // ACinVolRMS
    fb.bytes[3] = rxdata[idx++];
    fb.bytes[2] = rxdata[idx++];
    fb.bytes[1] = rxdata[idx++];
    fb.bytes[0] = rxdata[idx++];
    pfc_DataFlowFace.ACinVolRMS = fb.f;

    // ACinVolRmsFir
    fb.bytes[3] = rxdata[idx++];
    fb.bytes[2] = rxdata[idx++];
    fb.bytes[1] = rxdata[idx++];
    fb.bytes[0] = rxdata[idx++];
    pfc_DataFlowFace.ACinVolRmsFir = fb.f;

    // ACinCurRMS
    fb.bytes[3] = rxdata[idx++];
    fb.bytes[2] = rxdata[idx++];
    fb.bytes[1] = rxdata[idx++];
    fb.bytes[0] = rxdata[idx++];
    pfc_DataFlowFace.ACinCurRMS = fb.f;

    // ACinCurRmsFir
    fb.bytes[3] = rxdata[idx++];
    fb.bytes[2] = rxdata[idx++];
    fb.bytes[1] = rxdata[idx++];
    fb.bytes[0] = rxdata[idx++];
    pfc_DataFlowFace.ACinCurRmsFir = fb.f;

    // ACinPower
    fb.bytes[3] = rxdata[idx++];
    fb.bytes[2] = rxdata[idx++];
    fb.bytes[1] = rxdata[idx++];
    fb.bytes[0] = rxdata[idx++];
    pfc_DataFlowFace.ACinPower = fb.f;

    // ACinMaxPow
    fb.bytes[3] = rxdata[idx++];
    fb.bytes[2] = rxdata[idx++];
    fb.bytes[1] = rxdata[idx++];
    fb.bytes[0] = rxdata[idx++];
    pfc_DataFlowFace.ACinMaxPow = fb.f;
	
	  fb.bytes[3] = rxdata[idx++];
    fb.bytes[2] = rxdata[idx++];
    fb.bytes[1] = rxdata[idx++];
    fb.bytes[0] = rxdata[idx++];
    pfc_DataFlowFace.pfcAux = fb.f;
	
	  fb.bytes[3] = rxdata[idx++];
    fb.bytes[2] = rxdata[idx++];
    fb.bytes[1] = rxdata[idx++];
    fb.bytes[0] = rxdata[idx++];
    pfc_DataFlowFace.pfcInDcComp = fb.f;
    pfc_DataFlowFace.pfcInDcComp = 0;

    // OutCurr
    fb.bytes[3] = rxdata[idx++];
    fb.bytes[2] = rxdata[idx++];
    fb.bytes[1] = rxdata[idx++];
    fb.bytes[0] = rxdata[idx++];
    pfc_DataFlowFace.pfcHeatSinkTemp = fb.f;
		
	  fb.bytes[3] = rxdata[idx++];
    fb.bytes[2] = rxdata[idx++];
    fb.bytes[1] = rxdata[idx++];
    fb.bytes[0] = rxdata[idx++];
    pfc_DataFlowFace.pfcTransformerTemp = fb.f;
    
	
	  fb.bytes[3] = rxdata[idx++];
    fb.bytes[2] = rxdata[idx++];
    fb.bytes[1] = rxdata[idx++];
    fb.bytes[0] = rxdata[idx++];
    
    pfc_DataFlowFace.pfcInductanceTemp = fb.f;
		
		DataFlowFace.FaultSta.all &= 0xFFFEDB80;
	  DataFlowFace.FaultSta.all |= pfc_DataFlowFace.FaultSta.all;
	
}

/*******************************************************************************
 * @fn        : void Modbus_Slave_Rx(uint8_t *RxData ,uint16_t Len)
 * @brief     : Modbus Slave Rx
 * @param     : RxData - Pointer to the Rx Data
 *              Len - Length of the Rx Data
 * @return    : null
 * @example   : Modbus_Slave_Rx(RxData,Len);
 * @author    : KJW
 * @date      : 2026-02-09
 ******************************************************************************/
void Modbus_Slave_Rx(uint8_t *RxData ,uint16_t Len){
	uint16_t crc = 0;
	uint8_t id = 0;
	id = RxData[0];
    crc = (uint16_t)RxData[Len - 1] | (uint16_t)(RxData[Len - 2] << 8);
    if(crc == Modbus_CRC16(RxData,Len - 2) ){
        switch (id)
        {
         case pfc_id:
		    rx_pfc_data(RxData,Len);
			lastRxPfctime = gCAN_DATA.Xp_HandShake.ctRoll;
			overU = 1;
         break;
        default:
         break;
        }
    }   
}

//发送回pfc数据
void Send_to_Pfc_info(void){
	
	uint16_t crc = 0;
	uint8_t *pVolt, *pCurr;
	float temp = 0;
//	static float test1 = 1.0f,temst2 = 47.0f;
//	static uint16_t cont = 0;
	if(Ctrl_interFace.CtrMode == OvLoadPFM) DataLLC.Stuts = 1;
	else DataLLC.Stuts = 0;
  temp = ADSample_Info.vOut_Bat_adc - DataLLC.outVolt;
	
	if((temp >= 0.25f)||(temp <= -0.25f))
	{
    DataLLC.outVolt = ADSample_Info.vOut_Bat_adc;  
	}
	DataLLC.outCurr = ADSample_Info.iOut_Bat_FIR;	
	LLcData[0] = (uint8_t)llc_id;
	LLcData[1] = (uint8_t)DataLLC.Stuts;
#if OTA_UPGRADE_ENABLE
	if (LlcOta_GetPfcSafeReqFlag() != 0U)
	{
		LLcData[1] |= LLC_MODBUS_OTA_SAFE_REQ;
	}
#endif
	pVolt = (uint8_t*)&DataLLC.outVolt;
	pCurr = (uint8_t*)&DataLLC.outCurr;

	LLcData[2] = pVolt[0];
	LLcData[3] = pVolt[1];
	LLcData[4] = pVolt[2];
	LLcData[5] = pVolt[3];
	
	LLcData[6] = pCurr[0];
	LLcData[7] = pCurr[1];
	LLcData[8] = pCurr[2];
	LLcData[9] = pCurr[3];
	
	crc = Modbus_CRC16(LLcData, 10);

	LLcData[10] = (uint8_t)((crc >> 8) & 0xFF);
  LLcData[11] = (uint8_t)(crc & 0xFF);

	SendData_API(LLcData, 12);
}

void overTime_pfc(void){

#if OTA_UPGRADE_ENABLE
	if (OtaGw_IsSciWatchdogPaused() != 0U)
	{
		DataFlowFace.FaultSta.bit.SCI_Err = 0;
		overU = 0;
		return;
	}
#endif

	if(overU != 1){
		if(gCAN_DATA.Xp_HandShake.ctRoll - lastRxPfctime > 3 * 1000){
			if(Delay1ms.delay_ini >= 100)DataFlowFace.FaultSta.bit.SCI_Err = 1;
		}
	}else{
		DataFlowFace.FaultSta.bit.SCI_Err = 0;
	}

	overU = 0;
}


