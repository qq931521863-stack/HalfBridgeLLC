#include "CAN_Control_2800W.h"
#include "operateStatus.h"
#include "HwConfig.h"
#include "UpgradeFeatureConfig.h"
#if OTA_UPGRADE_ENABLE
#include "OtaGw.h"
#include "HostA2FactoryInfo.h"
#endif
#include "CanFdTxQueue.h"
CAN_DATA gCAN_DATA;
Chrgr_Debug_1_Data sDebug;
Chrgr_ChrgMsg0_Data sChrgMsg0;



/***************************************************************************************/
/***************************************************************************************/
/*********************************版*本*号**********************************************/
/***************************************************************************************/
/***************************************************************************************/
const char Set_Chrgr_AppRevs[3] = {'1','1','1'};
const char Set_Chrgr_BootLoadRevs[3] = {'B','.','1'};
const char Set_Chrgr_HardWareRevs[3] = {'H','.','2'};
const char Set_Chrgr_PartNumberRevs1[8] = {'P','7','0','1','0','0','2','R'};
const char Set_Chrgr_PartNumberRevs2[6] = {'B','1','0','2','0','0'};

void configCANFD(void){
	FDCAN_FilterTypeDef sFilterConfig;
	/* Configure Rx filter */
	sFilterConfig.IdType = FDCAN_STANDARD_ID;
	sFilterConfig.FilterIndex = 0;
	sFilterConfig.FilterType = FDCAN_FILTER_DUAL;
	sFilterConfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
	sFilterConfig.FilterID1 = 0x2F7;
	sFilterConfig.FilterID2 = 0x2DA;
	if (HAL_FDCAN_ConfigFilter(&hfdcan1, &sFilterConfig) != HAL_OK)
	{
	Error_Handler();
	}
	sFilterConfig.IdType = FDCAN_EXTENDED_ID;
	sFilterConfig.FilterIndex = 0;
	sFilterConfig.FilterType = FDCAN_FILTER_RANGE;
	sFilterConfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
	sFilterConfig.FilterID1 = 0x00000000;
	sFilterConfig.FilterID2 = 0x1FFFFFFF;
	if (HAL_FDCAN_ConfigFilter(&hfdcan1, &sFilterConfig) != HAL_OK)
	{
	Error_Handler();
	}
	/* Configure global filter:Filter all remote frames with STD and EXT ID Reject non matching frames with STD ID and EXT ID */
	if (HAL_FDCAN_ConfigGlobalFilter(&hfdcan1, FDCAN_ACCEPT_IN_RX_FIFO0, FDCAN_ACCEPT_IN_RX_FIFO0, FDCAN_REJECT_REMOTE, FDCAN_REJECT_REMOTE) != HAL_OK)
	{
	Error_Handler();
	}
	if(HAL_FDCAN_ConfigInterruptLines(&hfdcan1,FDCAN_IT_GROUP_RX_FIFO0,FDCAN_INTERRUPT_LINE0) != HAL_OK){
		Error_Handler();
	}
	if (HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0) != HAL_OK)
	{
	Error_Handler();
	}
	/* Start the FDCAN module */
	if (HAL_FDCAN_Start(&hfdcan1) != HAL_OK)
	{
		Error_Handler();
	}

	HAL_NVIC_EnableIRQ(FDCAN1_IT0_IRQn);
}

/*******************************************************************************
 * @fn        : void initCANFDData(void)
 * @brief     : Initialize CANFD data
 * @param     : void
 * @return    : void
 * @example   : initCANFDData();
 * @date      : 2026/02/06
 ******************************************************************************/
void initCANFDData(void){
	configCANFD();
	CanFdTxQueue_Init();
    memset(&gCAN_DATA,0x00,sizeof(CAN_DATA));
	memset(&sDebug,0x00,sizeof(Chrgr_Debug_1_Data));
	memset(&sChrgMsg0,0x00,sizeof(Chrgr_ChrgMsg0_Data));
}

/*******************************************************************************
 * @fn        : 
 * @brief     : 
 * @param     : 
 * @return    : 
 * @example   : 
 * @date      : 
 ******************************************************************************/
void sendCAN_FD_Data(uint32_t ID,uint8_t *TxData,uint32_t len){
	FDCAN_TxHeaderTypeDef TxHeader;
	/* Prepare Tx Header */
	TxHeader.Identifier = ID;
	TxHeader.IdType = FDCAN_STANDARD_ID;
	TxHeader.TxFrameType = FDCAN_DATA_FRAME;
	TxHeader.DataLength = len;
	TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
	TxHeader.BitRateSwitch = FDCAN_BRS_ON;
	TxHeader.FDFormat = FDCAN_FD_CAN;
	TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
	TxHeader.MessageMarker = 0;
	
	(void)CanFdTxQueue_Enqueue(CANFD_TX_PRIO_CHARGER, &TxHeader, TxData);
}

/*******************************************************************************
 * @fn        : uint8_t Checksum_XOR(uint8_t checkLen, uint8_t *CheckData)
 * @brief     : Calculate Checksum XOR
 * @param     : uint8_t checkLen, uint8_t *CheckData
 * @return    : uint8_t Checksum
 * @example   : Checksum_XOR(checkLen,CheckData);
 * @date      : 2026/02/06
 ******************************************************************************/
uint8_t Checksum_XOR(uint8_t checkLen, uint8_t *CheckData){
    uint8_t Checksum = 0;
    uint8_t Len = 0;

    if (CheckData != NULL && checkLen != 0) {
        for (Len = 0; Len < checkLen; Len++) {
            Checksum += CheckData[Len];
        }
    }
	
	Checksum ^= 0xFF;

    return Checksum;
}
/*******************************************************************************
 * @fn        : uint8_t parse_ID_BMS_ChrgInfo_Data(uint8_t Len,uint8_t *RxData)
 * @brief     : Parse ID_BMS_ChrgInfo_Data
 * @param     : void
 * @return    : 0 Data Ture / 1 Data Full
 * @example   : parse_ID_BMS_ChrgInfo_Data(Len,RxData);
 * @date      : 2026/02/06
 ******************************************************************************/
//uint8_t parse_ID_BMS_ChrgInfo_Data(uint8_t Len,uint8_t *RxData){
//    uint8_t CheckNub = 0;
//    uint8_t Dataflag = 0;
//    CheckNub = Checksum_XOR(Len - 1,RxData);
//    if(CheckNub == RxData[Len - 1]){
//        gCAN_DATA.sBMS_ChrgInfo_Data.BMS_RdyForChrg = (RxData[0] >> 6u) & 0x03;
//        gCAN_DATA.sBMS_ChrgInfo_Data.BMS_ChrgSt =  (RxData[0] >> 2u) & 0x0F;
//        gCAN_DATA.sBMS_ChrgInfo_Data.BMS_ChrgReqCur = (uint16_t)(RxData[0] & 0x03 << 8u) | (uint16_t)(RxData[1] & 0xFF); 
//        gCAN_DATA.sBMS_ChrgInfo_Data.BMS_ChrgReqVolt = (uint16_t)((RxData[2] & 0xFF) << 2u)  | (uint16_t)((RxData[3] >> 8u) & 0x03) ;    
//        gCAN_DATA.sBMS_ChrgInfo_Data.BMS_ChrgInfo_ctRoll =  RxData[6] & 0x0F;
//        gCAN_DATA.sBMS_ChrgInfo_Data.BMS_ChrgInfo_Checksum =  RxData[7];
//    }else{
//        Dataflag = 1;
//    }
//    return Dataflag;
//}

/*******************************************************************************
 * @fn        : uint8_t parse_ID_BMS2_ChrgMsg0_Data(uint8_t *RxData)
 * @brief     : Parse ID_BMS2_ChrgMsg0_Data
 * @param     : void
 * @return    : 1 Data Ture / 0 Data Full
 * @example   : parse_ID_BMS2_ChrgMsg0_Data(Len,RxData);
 * @date      : 2026/02/06
 ******************************************************************************/
uint8_t parse_ID_BMS2_ChrgMsg0_Data(uint8_t Len,uint8_t *RxData){
    uint8_t CheckNub = 0;
    uint8_t Dataflag = 1;
    CheckNub = Checksum_XOR(Len - 1,RxData);
    if(CheckNub == RxData[Len - 1]){
        gCAN_DATA.sBMS2_ChrgMsg0_Data.BMS2_HandShake = (RxData[0] >> 6u) & 0x03;
        gCAN_DATA.sBMS2_ChrgMsg0_Data.BMS2_ChrgEna =  (RxData[0] >> 4u) & 0x03;
        gCAN_DATA.sBMS2_ChrgMsg0_Data.BMS2_ChrgReqCur = (uint16_t)((RxData[0] & 0x0F) << 6u) | (uint16_t)((RxData[1] >> 2u) & 0x3F); 
        gCAN_DATA.sBMS2_ChrgMsg0_Data.BMS2_ChrgReqVolt = (uint16_t)((RxData[1] & 0x03) << 8u ) | (uint16_t)(RxData[2] & 0xFF) ;    
        gCAN_DATA.sBMS2_ChrgMsg0_Data.BMS2_ChrgSt =  (RxData[3] >> 4u) & 0x0F ; 
        gCAN_DATA.sBMS2_ChrgMsg0_Data.BMS2_ChrgMsg_ctRoll =  RxData[6] & 0x0F;
        gCAN_DATA.sBMS2_ChrgMsg0_Data.BMS2_ChrgMsg_Checksum =  RxData[7];
    }else{
        Dataflag = 0;
    }
    return Dataflag;
}

/*******************************************************************************
 * @fn        : uint8_t parse_ID_PMS_ChrgMsg0_Data(uint8_t Len,uint8_t *RxData)
 * @brief     : Parse ID_PMS_ChrgMsg0_Data
 * @param     : void
 * @return    : 1 Data Ture / 0 Data Full
 * @example   : parse_ID_PMS_ChrgMsg0_Data(RxData);
 * @date      : 2026/02/06
 ******************************************************************************/
uint8_t parse_ID_PMS_ChrgMsg0_Data(uint8_t Len,uint8_t *RxData){
    uint8_t CheckNub = 0;
    uint8_t Dataflag = 1;
    CheckNub = Checksum_XOR(Len - 1,RxData);
    if(CheckNub == RxData[Len - 1]){
        gCAN_DATA.sPMS_ChrgMsg0_Data.PMS_HandShake = (RxData[0] >> 6u) & 0x03;
        gCAN_DATA.sPMS_ChrgMsg0_Data.PMS_ChrgEna =  (RxData[0] >> 4u) & 0x03;
        gCAN_DATA.sPMS_ChrgMsg0_Data.PMS_ChrgReqCur = (uint16_t)((RxData[0] & 0x0F) << 6u) | (uint16_t)((RxData[1] >> 2u) & 0x3F); 
        gCAN_DATA.sPMS_ChrgMsg0_Data.PMS_ChrgReqVolt = (uint16_t)((RxData[1] & 0x03) << 8u) | (uint16_t)(RxData[2] & 0xFF) ;    
        gCAN_DATA.sPMS_ChrgMsg0_Data.PMS_CVModeReq =  (RxData[3] >> 7u) & 0x01; 
        gCAN_DATA.sPMS_ChrgMsg0_Data.PMS_ChrgMsg_ctRoll =  RxData[6] & 0x0F;
        gCAN_DATA.sPMS_ChrgMsg0_Data.PMS_ChrgMsg_Checksum =  RxData[7];
   }else{
       Dataflag = 0;
    }
	

    return Dataflag;
}

/*******************************************************************************
 * @fn        : void send_ID_Chrgr_ChrgMsg0_Data(Chrgr_ChrgMsg0_Data *chrgr_ChrgMsg0_Data)
 * @brief     : Send ID_Chrgr_ChrgMsg0_Data
 * @param     : Chrgr_ChrgMsg0_Data *chrgr_ChrgMsg0_Data
 * @return    : void
 * @example   : send_ID_Chrgr_ChrgMsg0_Data(&chrgr_ChrgMsg0_Data);
 * @date      : 2026/02/06
 ******************************************************************************/
void send_ID_Chrgr_ChrgMsg0_Data(Chrgr_ChrgMsg0_Data *chrgr_ChrgMsg0_Data){
    uint8_t TxData[48] = {0};
    TxData[0] = (uint8_t)((chrgr_ChrgMsg0_Data->Chrgr_HandShake & 0x03) << 6u);
    TxData[0] |= (uint8_t)((chrgr_ChrgMsg0_Data->FAULT_IN_OV & 0x01) << 5u);
    TxData[0] |= (uint8_t)((chrgr_ChrgMsg0_Data->FAULT_IN_UV & 0x01) << 4u);
    TxData[0] |= (uint8_t)((chrgr_ChrgMsg0_Data->FAULT_IN_OC & 0x01) << 3u);
    TxData[0] |= (uint8_t)((chrgr_ChrgMsg0_Data->FAULT_AC_OVER_FREQ & 0x01) << 2u);
    TxData[0] |= (uint8_t)((chrgr_ChrgMsg0_Data->Chrgr_DCOutpCur >> 8u) & 0x03);
    TxData[1] = (uint8_t)(chrgr_ChrgMsg0_Data->Chrgr_DCOutpCur & 0xFF);
    TxData[2] = (uint8_t)((chrgr_ChrgMsg0_Data->Chrgr_DCOutpVolt >> 2u) & 0xFF);
    TxData[3] = (uint8_t)((chrgr_ChrgMsg0_Data->Chrgr_DCOutpVolt & 0x03) << 6u);
    TxData[3] |= (uint8_t)((chrgr_ChrgMsg0_Data->Chrgr_ACInputCur >> 4u) & 0x3F);
    TxData[4] = (uint8_t)((chrgr_ChrgMsg0_Data->Chrgr_ACInputCur & 0x0F) << 4u);
    TxData[4] |= (uint8_t)((chrgr_ChrgMsg0_Data->Chrgr_ACInputVolt >> 6u) & 0x0F);
    TxData[5] = (uint8_t)((chrgr_ChrgMsg0_Data->Chrgr_ACInputVolt & 0x3F) << 2u);
    TxData[5] |= (uint8_t)((chrgr_ChrgMsg0_Data->Chrgr_WorkSt >> 1u) & 0x03);
    TxData[6] = (uint8_t)((chrgr_ChrgMsg0_Data->Chrgr_WorkSt & 0x01) << 7u);
    TxData[6] |= (uint8_t)((chrgr_ChrgMsg0_Data->Chrgr_Temp >> 1u) & 0x7F);
    TxData[7] = (uint8_t)((chrgr_ChrgMsg0_Data->Chrgr_Temp & 0x01) << 7u);
    TxData[7] |= (uint8_t)((chrgr_ChrgMsg0_Data->FAULT_AC_UNDER_FREQ & 0x01) << 6u);
    TxData[7] |= (uint8_t)((chrgr_ChrgMsg0_Data->FAULT_RES_ERR4 & 0x01) << 5u);
    TxData[7] |= (uint8_t)((chrgr_ChrgMsg0_Data->FAULT_BUS_OV & 0x01) << 4u);
    TxData[7] |= (uint8_t)((chrgr_ChrgMsg0_Data->FAULT_BUS_UV & 0x01) << 3u);
    TxData[7] |= (uint8_t)((chrgr_ChrgMsg0_Data->FAULT_OUT_OV & 0x01) << 2u);
    TxData[7] |= (uint8_t)((chrgr_ChrgMsg0_Data->FAULT_OUT_UV & 0x01) << 1u);
    TxData[7] |= (uint8_t)(chrgr_ChrgMsg0_Data->FAULT_OUT_OC & 0x01);
    TxData[8] = (uint8_t)((chrgr_ChrgMsg0_Data->FAULT_PFC_HW_ERR & 0x01) << 7u);
    TxData[8] |= (uint8_t)((chrgr_ChrgMsg0_Data->FAULT_LLC_HW_ERR & 0x01) << 6u);
    TxData[8] |= (uint8_t)((chrgr_ChrgMsg0_Data->FAULT_OUT_SHORT & 0x01) << 5u);
    TxData[8] |= (uint8_t)((chrgr_ChrgMsg0_Data->FAULT_PFC_OTP & 0x01) << 4u);
    TxData[8] |= (uint8_t)((chrgr_ChrgMsg0_Data->FAULT_LLC_OTP & 0x01) << 3u);
    TxData[8] |= (uint8_t)((chrgr_ChrgMsg0_Data->FAULT_BATT_REVS & 0x01) << 2u);
    TxData[8] |= (uint8_t)((chrgr_ChrgMsg0_Data->FAULT_IN_RLY_ERR & 0x01) << 1u);
    TxData[8] |= (uint8_t)(chrgr_ChrgMsg0_Data->FAULT_OUT_RLY_ERR & 0x01);
    TxData[9] = (uint8_t)((chrgr_ChrgMsg0_Data->FAULT_FAN_ERR & 0x01) << 7u);
    TxData[9] |= (uint8_t)((chrgr_ChrgMsg0_Data->FAULT_CAN_CMM_ERR & 0x01) << 6u);
    TxData[9] |= (uint8_t)((chrgr_ChrgMsg0_Data->FAULT_INSULATION_RES_Err & 0x01) << 5u);
    TxData[9] |= (uint8_t)((chrgr_ChrgMsg0_Data->FAULT_SCI_CMM_ERR & 0x01) << 4u);
    TxData[9] |= (uint8_t)((chrgr_ChrgMsg0_Data->FAULT_RES_ERR1 & 0x01) << 3u);
    TxData[9] |= (uint8_t)((chrgr_ChrgMsg0_Data->FAULT_RES_ERR2 & 0x01) << 2u);
    TxData[9] |= (uint8_t)((chrgr_ChrgMsg0_Data->FAULT_RES_ERR3 & 0x01) << 1u);
    TxData[9] |= (uint8_t)(chrgr_ChrgMsg0_Data->FAULT_RES_ERR5 & 0x01);
    TxData[10] = (uint8_t)((chrgr_ChrgMsg0_Data->FAULT_RES_ERR6 & 0x01) << 7u);
    TxData[10] |= (uint8_t)((chrgr_ChrgMsg0_Data->FAULT_RES_ERR7 & 0x01) << 6u);
    TxData[10] |= (uint8_t)((chrgr_ChrgMsg0_Data->FAULT_RES_ERR8 & 0x01) << 5u);
    TxData[10] |= (uint8_t)(chrgr_ChrgMsg0_Data->Chrgr_SalesRegion & 0x07);
    TxData[11] = (uint8_t)(chrgr_ChrgMsg0_Data->Chrgr_BootLoadRev[2] & 0xFF);
    TxData[12] = (uint8_t)(chrgr_ChrgMsg0_Data->Chrgr_BootLoadRev[1] & 0xFF);
    TxData[13] = (uint8_t)(chrgr_ChrgMsg0_Data->Chrgr_BootLoadRev[0] & 0xFF);
    TxData[14] = (uint8_t)(chrgr_ChrgMsg0_Data->Chrgr_AppRev[2] & 0xFF);
    TxData[15] = (uint8_t)(chrgr_ChrgMsg0_Data->Chrgr_AppRev[1] & 0xFF);
    TxData[16] = (uint8_t)(chrgr_ChrgMsg0_Data->Chrgr_AppRev[0] & 0xFF);
    TxData[17] = (uint8_t)(chrgr_ChrgMsg0_Data->Chrgr_HardWareRev[2] & 0xFF);
    TxData[18] = (uint8_t)(chrgr_ChrgMsg0_Data->Chrgr_HardWareRev[1] & 0xFF);
    TxData[19] = (uint8_t)(chrgr_ChrgMsg0_Data->Chrgr_HardWareRev[0] & 0xFF);
    TxData[20] = (uint8_t)(chrgr_ChrgMsg0_Data->Chrgr_PartNumberRev1[7] & 0xFF);
    TxData[21] = (uint8_t)(chrgr_ChrgMsg0_Data->Chrgr_PartNumberRev1[6] & 0xFF);
    TxData[22] = (uint8_t)(chrgr_ChrgMsg0_Data->Chrgr_PartNumberRev1[5] & 0xFF);
    TxData[23] = (uint8_t)(chrgr_ChrgMsg0_Data->Chrgr_PartNumberRev1[4] & 0xFF);
    TxData[24] = (uint8_t)(chrgr_ChrgMsg0_Data->Chrgr_PartNumberRev1[3] & 0xFF);
    TxData[25] = (uint8_t)(chrgr_ChrgMsg0_Data->Chrgr_PartNumberRev1[2] & 0xFF);
    TxData[26] = (uint8_t)(chrgr_ChrgMsg0_Data->Chrgr_PartNumberRev1[1] & 0xFF);
    TxData[27] = (uint8_t)(chrgr_ChrgMsg0_Data->Chrgr_PartNumberRev1[0] & 0xFF);
    TxData[28] = (uint8_t)(chrgr_ChrgMsg0_Data->Chrgr_PartNumberRev2[5] & 0xFF);
    TxData[29] = (uint8_t)(chrgr_ChrgMsg0_Data->Chrgr_PartNumberRev2[4] & 0xFF);
    TxData[30] = (uint8_t)(chrgr_ChrgMsg0_Data->Chrgr_PartNumberRev2[3] & 0xFF);
    TxData[31] = (uint8_t)(chrgr_ChrgMsg0_Data->Chrgr_PartNumberRev2[2] & 0xFF);
    TxData[32] = (uint8_t)(chrgr_ChrgMsg0_Data->Chrgr_PartNumberRev2[1] & 0xFF);
    TxData[33] = (uint8_t)(chrgr_ChrgMsg0_Data->Chrgr_PartNumberRev2[0] & 0xFF);
    TxData[34] = (uint8_t)(chrgr_ChrgMsg0_Data->Chrgr_DerateInfo & 0x0F << 4u);
    TxData[34] |= (uint8_t)(chrgr_ChrgMsg0_Data->Chrgr_FalutLevel & 0x0F );
    TxData[46] = (uint8_t)(chrgr_ChrgMsg0_Data->Chrgr_ChrgMsg_ctRoll & 0x0F );
    chrgr_ChrgMsg0_Data->Chrgr_ChrgMsg_Checksum = Checksum_XOR(47, TxData);
    TxData[47] = chrgr_ChrgMsg0_Data->Chrgr_ChrgMsg_Checksum;
    
    sendCAN_FD_Data(ID_Chrgr_ChrgMsg0, TxData,FDCAN_DLC_BYTES_48);
}

/*******************************************************************************
 * @fn        : void send_ID_TESTER_CHRG_DiagReq_Data(TESTER_CHRG_DiagReq_Data *tESTER_CHRG_DiagReq_Data)
 * @brief     : Send ID_TESTER_CHRG_DiagReq_Data
 * @param     : TESTER_CHRG_DiagReq_Data *tESTER_CHRG_DiagReq_Data
 * @return    : void
 * @example   : send_ID_TESTER_CHRG_DiagReq_Data(&tESTER_CHRG_DiagReq_Data);
 * @date      : 2026/02/06
 ******************************************************************************/
void send_ID_TESTER_CHRG_DiagReq_Data(TESTER_CHRG_DiagReq_Data *tESTER_CHRG_DiagReq_Data){

}

/*******************************************************************************
 * @fn        : void send_ID_TESTER_FunctionDiagReq_Data(TESTER_FunctionDiagReq_Data *tESTER_FunctionDiagReq_Data)
 * @brief     : Send ID_TESTER_FunctionDiagReq_Data
 * @param     : TESTER_FunctionDiagReq_Data *tESTER_FunctionDiagReq_Data
 * @return    : void
 * @example   : send_ID_TESTER_FunctionDiagReq_Data(&tESTER_FunctionDiagReq_Data);
 * @date      : 2026/02/06
 ******************************************************************************/
void send_ID_TESTER_FunctionDiagReq_Data(TESTER_FunctionDiagReq_Data *tESTER_FunctionDiagReq_Data){

}

/*******************************************************************************
 * @fn        : void send_ID_Chrgr_Debug_1_Data(Chrgr_Debug_1_Data *chrgr_Debug_1_Data)
 * @brief     : Send ID_Chrgr_Debug_1_Data
 * @param     : Chrgr_Debug_1_Data *chrgr_Debug_1_Data
 * @return    : void
 * @example   : send_ID_Chrgr_Debug_1_Data(&chrgr_Debug_1_Data);
 * @date      : 2026/02/06
 ******************************************************************************/
void send_ID_Chrgr_Debug_1_Data(Chrgr_Debug_1_Data *chrgr_Debug_1_Data){
    uint8_t TxData[32] = {0};

    TxData[0] = (uint8_t)((chrgr_Debug_1_Data->Chrgr_TimeStamp >> 24u) & 0xFF);
    TxData[1] = (uint8_t)((chrgr_Debug_1_Data->Chrgr_TimeStamp >> 16u) & 0xFF);
    TxData[2] = (uint8_t)((chrgr_Debug_1_Data->Chrgr_TimeStamp >> 8u) & 0xFF);
    TxData[3] = (uint8_t)(chrgr_Debug_1_Data->Chrgr_TimeStamp  & 0xFF);
    TxData[4] = (uint8_t)((chrgr_Debug_1_Data->Chrgr_Ms >> 2u) & 0xFF);
    TxData[5] = (uint8_t)((chrgr_Debug_1_Data->Chrgr_Ms & 0x03) << 6u);
    TxData[5] |= (uint8_t)((chrgr_Debug_1_Data->Chrgr_State & 0x07) << 3u);
    TxData[5] |= (uint8_t)(chrgr_Debug_1_Data->Chrgr_PfcState & 0x07);
    TxData[6] = (uint8_t)((chrgr_Debug_1_Data->Chrgr_LlcState & 0x07) << 5u);
    TxData[6] |= (uint8_t)((chrgr_Debug_1_Data->Chrgr_InPfcRelayEnable & 0x01) << 4u);
    TxData[6] |= (uint8_t)((chrgr_Debug_1_Data->Chrgr_PfcEn & 0x01) << 3u);
    TxData[6] |= (uint8_t)((chrgr_Debug_1_Data->Chrgr_PfcBurstEn & 0x01) << 2u);
    TxData[6] |= (uint8_t)((chrgr_Debug_1_Data->Chrgr_OutLlcRelay & 0x01) << 1u);
    TxData[6] |= (uint8_t)(chrgr_Debug_1_Data->Chrgr_PfcOk & 0x01);
    TxData[7] = (uint8_t)((chrgr_Debug_1_Data->Chrgr_DisChrgEn & 0x01) << 7u);
    TxData[7] |= (uint8_t)((chrgr_Debug_1_Data->Chrgr_LlcBurstEn & 0x01) << 6u);
    TxData[7] |= (uint8_t)((chrgr_Debug_1_Data->Chrgr_PfcAux >> 2u) & 0x3F);
    TxData[8] = (uint8_t)((chrgr_Debug_1_Data->Chrgr_PfcAux & 0x03) << 6);
    TxData[8] |= (uint8_t)((chrgr_Debug_1_Data->Chrgr_InAcFre >> 1u) & 0x3F);
    TxData[9] = (uint8_t)((chrgr_Debug_1_Data->Chrgr_InAcFre & 0x01) << 7u);
    TxData[9] |= (uint8_t)((chrgr_Debug_1_Data->Chrgr_InDcComp >> 1u) & 0x7F);
    TxData[10] = (uint8_t)((chrgr_Debug_1_Data->Chrgr_InDcComp & 0x01) << 7u);
    TxData[10] |= (uint8_t)((chrgr_Debug_1_Data->Chrgr_BusAveVolt >> 3u) & 0x7F);
    TxData[11] = (uint8_t)((chrgr_Debug_1_Data->Chrgr_BusAveVolt & 0x07) << 5u);
    TxData[11] |= (uint8_t)((chrgr_Debug_1_Data->Chrgr_LlcAux >> 3u) & 0x1F);
    TxData[12] = (uint8_t)((chrgr_Debug_1_Data->Chrgr_LlcAux & 0x07) << 5u);
    TxData[12] |= (uint8_t)((chrgr_Debug_1_Data->Chrgr_OutRelayVolt >> 5u) & 0x1F);
    TxData[13] = (uint8_t)((chrgr_Debug_1_Data->Chrgr_OutRelayVolt & 0x1F) << 3u);
    TxData[13] |= (uint8_t)((chrgr_Debug_1_Data->Chrgr_LlcFreq >> 12u) & 0x07);
    TxData[14] = (uint8_t)((chrgr_Debug_1_Data->Chrgr_LlcFreq >> 4u) & 0xFF);
    TxData[15] = (uint8_t)((chrgr_Debug_1_Data->Chrgr_LlcFreq & 0x0F) << 4u);
    TxData[15] |= (uint8_t)((chrgr_Debug_1_Data->Chrgr_FanDuty >> 3u) & 0x0F);
    TxData[16] = (uint8_t)((chrgr_Debug_1_Data->Chrgr_FanDuty & 0x07) << 5u);
    TxData[16] |= (uint8_t)((chrgr_Debug_1_Data->Chrgr_Fan1Cur >> 3u) & 0x1F);
    TxData[17] = (uint8_t)((chrgr_Debug_1_Data->Chrgr_Fan1Cur & 0x07) << 5u);
    TxData[17] |= (uint8_t)((chrgr_Debug_1_Data->Chrgr_Fan2Cur >> 3u) & 0x1F);
    TxData[18] = (uint8_t)((chrgr_Debug_1_Data->Chrgr_Fan2Cur & 0x07) << 5u);
    TxData[18] |= (uint8_t)((chrgr_Debug_1_Data->Chrgr_Fan3Cur >> 3u) & 0x1F);
    TxData[19] = (uint8_t)((chrgr_Debug_1_Data->Chrgr_Fan3Cur & 0x07) << 5u);
    TxData[19] |= (uint8_t)((chrgr_Debug_1_Data->Chrgr_Fan4Cur >> 3u) & 0x1F);
    TxData[20] = (uint8_t)((chrgr_Debug_1_Data->Chrgr_Fan4Cur & 0x07) << 5u);
    TxData[20] |= (uint8_t)((chrgr_Debug_1_Data->Chrgr_PfcHeatSinkTemp >> 3u) & 0x1F);
    TxData[21] = (uint8_t)((chrgr_Debug_1_Data->Chrgr_PfcHeatSinkTemp & 0x07) << 5u);
    TxData[21] |= (uint8_t)((chrgr_Debug_1_Data->Chrgr_PfcInductanceTemp >> 3u) & 0x1F);
    TxData[22] = (uint8_t)((chrgr_Debug_1_Data->Chrgr_PfcInductanceTemp & 0x07) << 5u);
    TxData[22] |= (uint8_t)((chrgr_Debug_1_Data->Chrgr_TransformerTemp >> 3u) & 0x1F);
    TxData[23] = (uint8_t)((chrgr_Debug_1_Data->Chrgr_TransformerTemp & 0x07) << 5u);
    TxData[23] |= (uint8_t)((chrgr_Debug_1_Data->Chrgr_LlcHeatSinkTempA >> 3u) & 0x1F);
    TxData[24] = (uint8_t)((chrgr_Debug_1_Data->Chrgr_LlcHeatSinkTempA & 0x07) << 5u);
    TxData[24] |= (uint8_t)((chrgr_Debug_1_Data->Chrgr_LlcHeatSinkTempB >> 3u) & 0x1F);
    TxData[25] = (uint8_t)((chrgr_Debug_1_Data->Chrgr_LlcHeatSinkTempB & 0x07) << 5u);
    TxData[25] |= (uint8_t)((chrgr_Debug_1_Data->Chrgr_DcInterfaceTemp >> 3u) & 0x1F);
    TxData[26] = (uint8_t)((chrgr_Debug_1_Data->Chrgr_DcInterfaceTemp & 0x07) << 5u);
    TxData[26] |= (uint8_t)((chrgr_Debug_1_Data->Chrgr_AirTemp >> 3u) & 0x1F);
    TxData[27] = (uint8_t)((chrgr_Debug_1_Data->Chrgr_AirTemp & 0x07) << 5u);
    TxData[30] = (uint8_t)(chrgr_Debug_1_Data->PMS_ChrgMsg_ctRoll & 0x0F);
    chrgr_Debug_1_Data->PMS_ChrgMsg_Checksum = Checksum_XOR(31,TxData);
    TxData[31] = (uint8_t)chrgr_Debug_1_Data->PMS_ChrgMsg_Checksum;

    sendCAN_FD_Data(ID_Chrgr_Debug_1,TxData,FDCAN_DLC_BYTES_32);
}

/*******************************************************************************
 * @fn        : void send_ID_CHRG_TESTER_DiagResp_Data(CHRG_TESTER_DiagResp_Data *chrgr_Tester_DiagResp_Data)
 * @brief     : Send ID_CHRG_TESTER_DiagResp_Data
 * @param     : CHRG_TESTER_DiagResp_Data *chrgr_Tester_DiagResp_Data
 * @return    : void
 * @example   : send_ID_CHRG_TESTER_DiagResp_Data(&chrgr_Tester_DiagResp_Data);
 * @date      : 2026/02/06
 ******************************************************************************/
void send_ID_CHRG_TESTER_DiagResp_Data(CHRG_TESTER_DiagResp_Data *chrgr_Tester_DiagResp_Data){

}

/*******************************************************************************
 * @fn        : void BMS_HandShakeCheck(uint8_t BMSTemp)
 * @brief     : BMS HandShake Check
 * @param     : uint8_t BMSTemp
 * @return    : void
 * @example   : BMS_HandShakeCheck(BMSTemp);
 * @date      : 2026/02/06
 ******************************************************************************/
//void BMS_HandShakeCheck(uint8_t BMSTemp){
//    static uint32_t last_Rx_ctRoll = 0;
//    static uint8_t lastBMSTemp = 1;

//    if(BMSTemp != lastBMSTemp){
//        last_Rx_ctRoll = gCAN_DATA.Xp_HandShake.ctRoll;
//        lastBMSTemp = BMSTemp;
//    }

//    if(!BMSTemp){
//        if((gCAN_DATA.Xp_HandShake.ctRoll - last_Rx_ctRoll) > 3 * 1000){
//            gCAN_DATA.Xp_HandShake.HandShake |= 0x01;
//        }
//    }else{
//        if((gCAN_DATA.Xp_HandShake.ctRoll - last_Rx_ctRoll) > 10 * 1000){
//            gCAN_DATA.Xp_HandShake.HandShake &= 0x06;
//        }
//    }
//}

volatile uint8_t temp2 = 0;
volatile uint8_t temp3 = 0;
/*******************************************************************************
 * @fn        : void BMS2_HandShakeCheck(uint8_t BMS2Temp)
 * @brief     : BMS2 HandShake Check
 * @param     : uint8_t BMS2Temp
 * @return    : void
 * @example   : BMS2_HandShakeCheck(BMS2Temp);
 * @date      : 2026/02/06
 ******************************************************************************/
void BMS2_HandShakeCheck(uint8_t BMS2Temp){
    static uint32_t last_Rx_ctRoll = 0;
    static uint8_t lastBMSTemp = 1;

    if(BMS2Temp != lastBMSTemp){
        last_Rx_ctRoll = gCAN_DATA.Xp_HandShake.ctRoll;
        lastBMSTemp = BMS2Temp;
    }

    if(BMS2Temp){
			if(gCAN_DATA.sBMS2_ChrgMsg0_Data.BMS2_HandShake){
				gCAN_DATA.Xp_HandShake.HandShake |= 0x01;
			}else{
				gCAN_DATA.Xp_HandShake.HandShake &= 0x02;
			}
			gCAN_DATA.Xp_HandShake.online |= 0x01;
			temp2 = 0;
    }else{
        if((gCAN_DATA.Xp_HandShake.ctRoll - last_Rx_ctRoll) > 3 * 1000){
			gCAN_DATA.Xp_HandShake.HandShake &= 0x02;
			gCAN_DATA.Xp_HandShake.online &= 0x02;
			
			//over time clean PMSrx_Data
			gCAN_DATA.sBMS2_ChrgMsg0_Data.BMS2_HandShake = 0;
			gCAN_DATA.sBMS2_ChrgMsg0_Data.BMS2_ChrgEna =  0;
			gCAN_DATA.sBMS2_ChrgMsg0_Data.BMS2_ChrgReqCur = 0; 
			gCAN_DATA.sBMS2_ChrgMsg0_Data.BMS2_ChrgReqVolt = 0;    
			gCAN_DATA.sBMS2_ChrgMsg0_Data.BMS2_ChrgSt =  0; 
        }
    }
	temp2 = 0;
}
/*******************************************************************************
 * @fn        : void PMS_HandShakeCheck(uint8_t PMSTemp)
 * @brief     : PMS HandShake Check
 * @param     : uint8_t PMSTemp
 * @return    : void
 * @example   : PMS_HandShakeCheck(PMSTemp);
 * @date      : 2026/02/06
 ******************************************************************************/
void PMS_HandShakeCheck(uint8_t PMSTemp){
    static uint32_t last_Rx_ctRoll = 0;
    static uint8_t lastBMSTemp = 0;

    if(PMSTemp != lastBMSTemp){
        last_Rx_ctRoll = gCAN_DATA.Xp_HandShake.ctRoll;
        lastBMSTemp = PMSTemp;
    }

    if(PMSTemp){
			if(gCAN_DATA.sPMS_ChrgMsg0_Data.PMS_HandShake){
				gCAN_DATA.Xp_HandShake.HandShake |= 0x02;
			}else{
				gCAN_DATA.Xp_HandShake.HandShake &= 0x01;
			}
			gCAN_DATA.Xp_HandShake.online |= 0x02;
			temp3 = 0;

    }else{
        if((gCAN_DATA.Xp_HandShake.ctRoll - last_Rx_ctRoll) > 3 * 1000){
			gCAN_DATA.Xp_HandShake.HandShake &= 0x01;
			gCAN_DATA.Xp_HandShake.online &= 0x01;
			//over time clean PMSrx_Data
			gCAN_DATA.sPMS_ChrgMsg0_Data.PMS_HandShake = 0;
			gCAN_DATA.sPMS_ChrgMsg0_Data.PMS_ChrgEna =  0;
			gCAN_DATA.sPMS_ChrgMsg0_Data.PMS_ChrgReqCur = 0; 
			gCAN_DATA.sPMS_ChrgMsg0_Data.PMS_ChrgReqVolt = 0 ;    
			gCAN_DATA.sPMS_ChrgMsg0_Data.PMS_CVModeReq =  0; 
        }
    }
	
	temp3 = 0;
}

void ctRoll(void){
    gCAN_DATA.Xp_HandShake.ctRoll++;
}

//uint8_t temp1 = 0;

void Data_Analyze(uint32_t ID,uint8_t *RxData,uint32_t dataLen){
 //   uint8_t temp = 0;
	switch(ID){
		case ID_BMS_ChrgInfo:
			//temp1 = parse_ID_BMS_ChrgInfo_Data(dataLen,RxData);
            
			break;
		case ID_BMS2_ChrgMsg0:
			temp2 = parse_ID_BMS2_ChrgMsg0_Data(dataLen,RxData);
            
			break;
        case ID_PMS_ChrgMsg0:
			temp3 = parse_ID_PMS_ChrgMsg0_Data(dataLen,RxData);
            
			break;
		default:
			
			break;
	}

}

void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs){
	FDCAN_RxHeaderTypeDef RxHeader;
	uint8_t RxCanFD[64] = {0};
	HAL_FDCAN_GetRxMessage(hfdcan,FDCAN_RX_FIFO0,&RxHeader,RxCanFD);

	if (RxHeader.IdType == FDCAN_EXTENDED_ID)
	{
#if OTA_UPGRADE_ENABLE
		OtaGw_RxIsrHook(&RxHeader, RxCanFD);
#else
		Data_Analyze(RxHeader.Identifier, RxCanFD, RxHeader.DataLength);
#endif
	}
	else
	{
		Data_Analyze(RxHeader.Identifier,RxCanFD,RxHeader.DataLength);
	}
}

volatile uint8_t tempp = 0;

void CAN_SendData_Run(void){
    uint8_t temp = 0;
	BMS2_HandShakeCheck(temp2);
	PMS_HandShakeCheck(temp3);
		
	if(gCAN_DATA.Xp_HandShake.online != 0){
		DataFlowFace.FaultSta.bit.CAN_Err = 0;
	}
	
    ctRoll();
    
    gCAN_DATA.Xp_HandShake.tick++;
    if(gCAN_DATA.Xp_HandShake.tick < 100){
        return;
    }
    gCAN_DATA.Xp_HandShake.tick = 0;

#if OTA_UPGRADE_ENABLE
    if (OtaGw_IsTrafficActive() != 0U)
    {
        return;
    }
#endif
	
	sChrgMsg0.Chrgr_HandShake = 0x01;
	
    sChrgMsg0.Chrgr_WorkSt = gSys_State.SysSta;
	
	sChrgMsg0.Chrgr_DCOutpCur = ADSample_Info.iOut_Bat_FIR * 10 ;
	sChrgMsg0.Chrgr_DCOutpVolt = ADSample_Info.vOut_Bat_FIR * 10;
    
    sChrgMsg0.Chrgr_ACInputCur = pfc_DataFlowFace.ACinCurRmsFir * 10;
    sChrgMsg0.Chrgr_ACInputVolt = pfc_DataFlowFace.ACinVolRmsFir;
	
	
	//max temp up data 
	float maxTemp = 0.0f;
	maxTemp = ADSample_Info.Temp0_adc;
	if(maxTemp < ADSample_Info.Temp1_adc){
		maxTemp = ADSample_Info.Temp1_adc;
	}
	if(maxTemp < ADSample_Info.Temp2_adc){
		maxTemp = ADSample_Info.Temp2_adc;
	}
	if(maxTemp < ADSample_Info.Temp3_adc){
		maxTemp = ADSample_Info.Temp3_adc;
	}
	if(maxTemp < pfc_DataFlowFace.pfcHeatSinkTemp){
		maxTemp = pfc_DataFlowFace.pfcHeatSinkTemp;
	}
	if(maxTemp < pfc_DataFlowFace.pfcInductanceTemp){
		maxTemp = pfc_DataFlowFace.pfcInductanceTemp;
	}
	
    sChrgMsg0.Chrgr_Temp = (uint8_t)(maxTemp + 10);
	
	
    sChrgMsg0.FAULT_IN_OV = DataFlowFace.FaultSta.bit.ACin_ov == 1 ? 0x01 : 0x00;
    sChrgMsg0.FAULT_IN_UV = DataFlowFace.FaultSta.bit.ACin_uv == 1 ? 0x01 : 0x00;
    sChrgMsg0.FAULT_IN_OC = DataFlowFace.FaultSta.bit.ACin_oc == 1 ? 0x01 : 0x00;
    sChrgMsg0.FAULT_AC_OVER_FREQ = DataFlowFace.FaultSta.bit.ACin_of == 1 ? 0x01 : 0x00;
    sChrgMsg0.FAULT_AC_UNDER_FREQ = DataFlowFace.FaultSta.bit.ACin_uf == 1 ? 0x01 : 0x00;
    sChrgMsg0.FAULT_BUS_OV = DataFlowFace.FaultSta.bit.Vbus_ov == 1 ? 0x01 : 0x00;
    sChrgMsg0.FAULT_BUS_UV = DataFlowFace.FaultSta.bit.Vbus_uv == 1 ? 0x01 : 0x00;
    sChrgMsg0.FAULT_OUT_OV = DataFlowFace.FaultSta.bit.Out_ov == 1 ? 0x01 : 0x00;
    sChrgMsg0.FAULT_OUT_UV = DataFlowFace.FaultSta.bit.Out_uv == 1 ? 0x01 : 0x00;
    sChrgMsg0.FAULT_OUT_OC = DataFlowFace.FaultSta.bit.Out_oc == 1 ? 0x01 : 0x00;
    sChrgMsg0.FAULT_PFC_HW_ERR = DataFlowFace.FaultSta.bit.PFC_Err == 1 ? 0x01 : 0x00;
    sChrgMsg0.FAULT_LLC_HW_ERR = DataFlowFace.FaultSta.bit.LLC_Err == 1 ? 0x01 : 0x00;
    sChrgMsg0.FAULT_OUT_SHORT = DataFlowFace.FaultSta.bit.Out_short == 1 ? 0x01 : 0x00;
    sChrgMsg0.FAULT_PFC_OTP = DataFlowFace.FaultSta.bit.PFC_otp == 1 ? 0x01 : 0x00;
    sChrgMsg0.FAULT_LLC_OTP = DataFlowFace.FaultSta.bit.LLC_otp == 1 ? 0x01 : 0x00;
    sChrgMsg0.FAULT_BATT_REVS = DataFlowFace.FaultSta.bit.BAT_revs == 1 ? 0x01 : 0x00;
    sChrgMsg0.FAULT_IN_RLY_ERR = DataFlowFace.FaultSta.bit.inRLY_Err == 1 ? 0x01 : 0x00;
    sChrgMsg0.FAULT_OUT_RLY_ERR = DataFlowFace.FaultSta.bit.outRLY_Err == 1 ? 0x01 : 0x00;
    sChrgMsg0.FAULT_FAN_ERR = DataFlowFace.FaultSta.bit.FAN_Err == 1 ? 0x01 : 0x00;
    sChrgMsg0.FAULT_CAN_CMM_ERR = DataFlowFace.FaultSta.bit.CAN_Err == 1 ? 0x01 : 0x00;
    sChrgMsg0.FAULT_INSULATION_RES_Err = DataFlowFace.FaultSta.bit.INS_Err == 1 ? 0x01 : 0x00;
    sChrgMsg0.FAULT_SCI_CMM_ERR = DataFlowFace.FaultSta.bit.SCI_Err == 1 ? 0x01 : 0x00;
    sChrgMsg0.FAULT_RES_ERR1 = DataFlowFace.FaultSta.bit.ResonOc == 1 ? 0x01 : 0x00;
    sChrgMsg0.FAULT_RES_ERR2 = DataFlowFace.FaultSta.bit.InterOv == 1 ? 0x01 : 0x00;
    sChrgMsg0.FAULT_RES_ERR3 = 0x00;
    sChrgMsg0.FAULT_RES_ERR4 = 0x00;
    sChrgMsg0.FAULT_RES_ERR5 = 0x00;
    sChrgMsg0.FAULT_RES_ERR6 = 0x00;
    sChrgMsg0.FAULT_RES_ERR7 = 0x00;
    sChrgMsg0.FAULT_RES_ERR8 = 0x00;
    sChrgMsg0.Chrgr_FalutLevel = 0x00;
    sChrgMsg0.Chrgr_DerateInfo = HwOtpStr.ErrStaErr;

#if OTA_UPGRADE_ENABLE
    HostA2FactoryInfo_FillChrgrIdentityFields();
#else
    for(temp = 0;temp < 3;temp++){
        sChrgMsg0.Chrgr_BootLoadRev[temp] = Set_Chrgr_BootLoadRevs[temp];
    }
    for(temp = 0;temp < 3;temp++){
        sChrgMsg0.Chrgr_AppRev[temp] = Set_Chrgr_AppRevs[temp];
    }
    for(temp = 0;temp < 3;temp++){
        sChrgMsg0.Chrgr_HardWareRev[temp] = Set_Chrgr_HardWareRevs[temp];
    }
    for(temp = 0;temp < 8;temp++){
        sChrgMsg0.Chrgr_PartNumberRev1[temp] = Set_Chrgr_PartNumberRevs1[temp];
    }
    for(temp = 0;temp < 8;temp++){
        sChrgMsg0.Chrgr_PartNumberRev2[temp] = Set_Chrgr_PartNumberRevs2[temp];
    }
    sChrgMsg0.Chrgr_SalesRegion = 0x00;
#endif
	
//    sChrgMsg0.Chrgr_ChrgMsg_ctRoll++;
//    if(sChrgMsg0.Chrgr_ChrgMsg_ctRoll > 15)
//    sChrgMsg0.Chrgr_ChrgMsg_ctRoll = 0;

	tempp++;
	if(tempp > 15){
		tempp = 0;
	}
	sChrgMsg0.Chrgr_ChrgMsg_ctRoll = tempp;
	
	send_ID_Chrgr_ChrgMsg0_Data(&sChrgMsg0);
	
/********************************************************************************************************/
	
    sDebug.Chrgr_TimeStamp = 0x00;
    sDebug.Chrgr_Ms = 0x00;
    sDebug.Chrgr_State = gSys_State.SysSta;	  
    sDebug.Chrgr_PfcState = pfc_DataFlowFace.PfcState;
    sDebug.Chrgr_LlcState = Ctrl_interFace.CtrMode;
    sDebug.Chrgr_InPfcRelayEnable = pfc_DataFlowFace.RelaySta;
    sDebug.Chrgr_PfcEn = pfc_DataFlowFace.PFC_ok;
    sDebug.Chrgr_PfcBurstEn = 0x00;
    sDebug.Chrgr_OutLlcRelay = DataFlowFace.RelaySta;
    sDebug.Chrgr_PfcOk = pfc_DataFlowFace.PFC_ok;
    sDebug.Chrgr_DisChrgEn = DataFlowFace.DisCharge;
    sDebug.Chrgr_LlcBurstEn = 0x00;
    sDebug.Chrgr_PfcAux = pfc_DataFlowFace.pfcAux * 10.0f;
    sDebug.Chrgr_InAcFre = pfc_DataFlowFace.ACinFlv;
    sDebug.Chrgr_InDcComp = 0;
    sDebug.Chrgr_BusAveVolt = pfc_DataFlowFace.VbusVolt;
    sDebug.Chrgr_LlcAux = ADSample_Info.AuxVolt * 10.0f;
    sDebug.Chrgr_OutRelayVolt = ADSample_Info.vOut_Rly_adc * 10.0f;
    sDebug.Chrgr_LlcFreq = DriverPwm.Plv * 0.1f;
//    sDebug.Chrgr_LlcFreq = (uint16_t)(vParamPid.output * 1000.0f);	
    sDebug.Chrgr_FanDuty = DataFlowFace.Fanpwm;
    sDebug.Chrgr_Fan1Cur = ADSample_Info.Fan0_cur * 0.1f;
    sDebug.Chrgr_Fan2Cur = ADSample_Info.Fan1_cur * 0.1f;
    sDebug.Chrgr_Fan3Cur = ADSample_Info.Fan2_cur * 0.1f;
    sDebug.Chrgr_Fan4Cur = ADSample_Info.Fan3_cur * 0.1f;
    sDebug.Chrgr_PfcHeatSinkTemp = pfc_DataFlowFace.pfcHeatSinkTemp + 10; 
    sDebug.Chrgr_PfcInductanceTemp = pfc_DataFlowFace.pfcInductanceTemp + 10;
    sDebug.Chrgr_TransformerTemp = ADSample_Info.NTC3_AD_FIR + 10;
    sDebug.Chrgr_LlcHeatSinkTempA = pfc_DataFlowFace.pfcTransformerTemp + 10;
    sDebug.Chrgr_LlcHeatSinkTempB = ADSample_Info.NTC0_AD_FIR + 10;
    sDebug.Chrgr_DcInterfaceTemp = ADSample_Info.NTC2_AD_FIR + 10;
    sDebug.Chrgr_AirTemp = ADSample_Info.NTC1_AD_FIR + 10;

    sDebug.PMS_ChrgMsg_ctRoll+=1;
    if(sDebug.PMS_ChrgMsg_ctRoll > 15)
    sDebug.PMS_ChrgMsg_ctRoll = 0;
	
	send_ID_Chrgr_Debug_1_Data(&sDebug);
}
