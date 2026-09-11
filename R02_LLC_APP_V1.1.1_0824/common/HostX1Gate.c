#include "UpgradeFeatureConfig.h"
#if OTA_UPGRADE_ENABLE

#include "HostX1Gate.h"
#include "AppA2CanCmd.h"
#include "HostFlashMemoryMap.h"
#include "CanFdTxQueue.h"
#include <string.h>
#include "HostSlotValidate.h"
#include "HostOtaHandoff.h"
#include "UpgradeMsgSaveFunctionDf.h"
#include "main.h"
#include "stm32g4xx.h"

void HostA2_CanSendExtMsg(uint8_t u8CmdType, uint8_t u8CmdCode, const uint8_t *pu8Data, uint32_t u32Dlc)
{
    FDCAN_TxHeaderTypeDef txHeader;
    _UStCanIDMsg idMsg;
    uint8_t txData[64] = {0};
    uint32_t u32CopyLen = 0U;

    if (u32Dlc == FDCAN_DLC_BYTES_12)
    {
        u32CopyLen = 12U;
    }
    else if (u32Dlc == FDCAN_DLC_BYTES_20)
    {
        u32CopyLen = 20U;
    }
    else if (u32Dlc == FDCAN_DLC_BYTES_64)
    {
        u32CopyLen = 52U;
    }
    else
    {
        u32CopyLen = 8U;
        u32Dlc = FDCAN_DLC_BYTES_8;
    }
    if (pu8Data != NULL && u32CopyLen > 0U)
    {
        memcpy(txData, pu8Data, u32CopyLen);
    }

    idMsg.StCanIDMsgBit.none = 0U;
    idMsg.StCanIDMsgBit.CmdType = u8CmdType;
    idMsg.StCanIDMsgBit.CmdCode = u8CmdCode;
    idMsg.StCanIDMsgBit.SrcAddr = DF_CAN_MASTER_ADDR;
    idMsg.StCanIDMsgBit.DstAddr = DF_CAN_PC_SFOT_ADDR;

    txHeader.Identifier = idMsg.all;
    txHeader.IdType = FDCAN_EXTENDED_ID;
    txHeader.TxFrameType = FDCAN_DATA_FRAME;
    txHeader.DataLength = u32Dlc;
    txHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    txHeader.BitRateSwitch = FDCAN_BRS_ON;
    txHeader.FDFormat = FDCAN_FD_CAN;
    txHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    txHeader.MessageMarker = 0U;

    (void)CanFdTxQueue_Enqueue(CANFD_TX_PRIO_OTA, &txHeader, txData);
}

static void HostA2_CanSendExt8(uint8_t u8CmdCode, const uint8_t *pu8Data, uint8_t u8Len)
{
    uint8_t txData[8] = {0};

    if (pu8Data != NULL && u8Len > 8U)
    {
        u8Len = 8U;
    }
    if (pu8Data != NULL && u8Len > 0U)
    {
        memcpy(txData, pu8Data, u8Len);
    }
    HostA2_CanSendExtMsg(DF_UPGRADE_CMD_TYPE, u8CmdCode, txData, FDCAN_DLC_BYTES_8);
}

static void HostA2_CanSendStatusReply(uint8_t u8DevType, uint8_t u8Status)
{
    uint8_t data[8];

    data[0] = u8DevType;
    data[1] = u8Status;
    data[2] = 0x64U;
    data[3] = 0x00U;
    data[4] = 0U;
    data[5] = 0U;
    data[6] = 0U;
    data[7] = 0U;
    HostA2_CanSendExt8(DF_CAN_UPGRADE_STATUS_INQURE_CMD, data, 8U);
}

__weak void HostX1Gate_PlatformShutdown(void)
{
    __disable_irq();
}

uint8_t HostX1Gate_Check(void)
{
    if (HostSlot_VectorValid(DF_HOST_APP_A1_START, DF_HOST_APP_A1_SIZE, DF_HOST_APP_A1_START) != 0U)
    {
        return HOST_X1GATE_X1_INVALID;
    }
    return HOST_X1GATE_OK;
}

uint8_t HostA2_SetOtaServiceFlag(void)
{
    if (ReadWriteUpgradeFileMsg(&StMasterProgStatus, DF_MASTET_FIRMWARE_MSG_START_ADDRESS) != 0x01U)
    {
        return 1U;
    }
    StMasterProgStatus.StProgStatusMsg.u8OtaServiceFlag = 1U;
    SaveWriteUpgradeFileMsg(&StMasterProgStatus, DF_MASTET_FIRMWARE_MSG_START_ADDRESS);
    return 0U;
}

void HostX1Gate_JumpToX1(void)
{
    typedef void (*iapfun)(void);
    uint32_t u32X1 = DF_HOST_APP_A1_START;
    uint32_t u32Sp;
    uint32_t u32Reset;
    iapfun jump2app;

    if (HostX1Gate_Check() != HOST_X1GATE_OK)
    {
        return;
    }

    u32Sp = *(__IO uint32_t *)u32X1;
    u32Reset = *(__IO uint32_t *)(u32X1 + 4U);

    HostX1Gate_PlatformShutdown();
    HostOtaHandoff_MarkA2ToA1();

    SCB->VTOR = u32X1;
    __DSB();
    __ISB();

    jump2app = (iapfun)u32Reset;
    __set_MSP(u32Sp);
    __set_PRIMASK(0U);
    jump2app();
}

uint8_t HostA2_ShouldHandoffToA1(uint8_t u8CmdCode)
{
    if (u8CmdCode == DF_CAN_INQURE_DEV_MSG_CMD
        || u8CmdCode == DF_CAN_INQURE_SLAVE_DEV_MSG_CMD
        || u8CmdCode == DF_CAN_INQURE_HOST_DEV_EXT_CMD
        || u8CmdCode == DF_CAN_FACTORY_INFO_READ_CMD
        || u8CmdCode == DF_CAN_FACTORY_INFO_WRITE_CMD
        || u8CmdCode == DF_CAN_FACTORY_INFO_DELETE_CMD
        || u8CmdCode == DF_CAN_UPGRADE_STATUS_INQURE_CMD)
    {
        return 0U;
    }

    if (u8CmdCode == DF_CAN_INTO_UPGRADE_MODE_CMD
        || u8CmdCode == DF_CAN_START_UPGRADE_CMD
        || u8CmdCode == DF_CAN_FILE_TRANSFER_REQUEST_CMD
        || u8CmdCode == DF_CAN_FILE_TRANSFER_CMD
        || u8CmdCode == DF_CAN_FILE_TRANSFER_END_CMD)
    {
        if (ReadWriteUpgradeFileMsg(&StMasterProgStatus, DF_MASTET_FIRMWARE_MSG_START_ADDRESS) != 0x01U)
        {
            return 0U;
        }
        if (u8CmdCode != DF_CAN_INTO_UPGRADE_MODE_CMD
            && StMasterProgStatus.StProgStatusMsg.u16UpgradeState == DF_UPGRADR_FINISH_STATUS
            && StMasterProgStatus.StProgStatusMsg.u8OtaServiceFlag == 0U)
        {
            return 0U;
        }
        return 1U;
    }

    return 0U;
}

void HostA2_EnterOtaService(void)
{
    /* 写 OtaServiceFlag 后 NVIC 复位，由 Bootloader 冷启进 APPA1 OTA 态。
     * 软跳转会在 PlatformShutdown 中 DeInit CAN；若跳转失败则 CAN 永久失效且 Flag 已置 1。 */
    if (HostA2_SetOtaServiceFlag() != 0U)
    {
        return;
    }
    NVIC_SystemReset();
}

void HostA2_ReplyIntoUpgradeMode(can_receive_message_struct *can_receive_message, uint8_t u8Result)
{
    uint8_t data[8];

    (void)can_receive_message;
    data[0] = u8Result;
    HostA2_CanSendExt8(DF_CAN_INTO_UPGRADE_MODE_CMD, data, 8U);
}

void HostA2_ReplyUpgradeStatusFromCan(can_receive_message_struct *can_receive_message, uint8_t u8CANChannel)
{
    uint8_t u8DevType;
    uint8_t u8Status;

    (void)u8CANChannel;

    if (can_receive_message == NULL)
    {
        return;
    }

    u8DevType = can_receive_message->rx_data[0];
    if (u8DevType != (uint8_t)(Df_UPGRADE_DEV_MERGED_PACK & 0xFFU)
        && u8DevType != (uint8_t)(Df_UPGRADE_DEV_SLAVE1 & 0xFFU))
    {
        return;
    }

    if (StMasterProgStatus.StProgStatusMsg.u16UpgradeState == DF_UPGRADR_FINISH_STATUS
        || StMasterProgStatus.StProgStatusMsg.u16UpgradeState == DF_NOT_UPGRADE_STATUS)
    {
        u8Status = (uint8_t)DF_INQUIRE_UPGRADE_UPGRADE_SUCCEED_ACK;
    }
    else if (StMasterProgStatus.StProgStatusMsg.u16UpgradeState == DF_WAIT_INTO_UPGRADR_STATUS
             || StMasterProgStatus.StProgStatusMsg.u16UpgradeState == DF_WAIT_DOWNLOAD_UPGRADR_FILE_STATUS)
    {
        u8Status = (uint8_t)DF_INQUIRE_UPGRADE_UPGRADING_STATUS_ACK;
    }
    else if (StMasterProgStatus.StProgStatusMsg.u16UpgradeState == DF_UPGRADE_FAIL_STATUS)
    {
        u8Status = (uint8_t)DF_INQUIRE_UPGRADE_UPGRADE_FAIL_ACK;
    }
    else
    {
        u8Status = (uint8_t)DF_INQUIRE_UPGRADE_UPGRADING_STATUS_ACK;
    }

    HostA2_CanSendStatusReply(u8DevType, u8Status);
}

#else /* OTA_UPGRADE_ENABLE */

#include "HostX1Gate.h"
#include "OtaCanTypes.h"

void HostX1Gate_PlatformShutdown(void) {}
uint8_t HostX1Gate_Check(void) { return HOST_X1GATE_X1_INVALID; }
void HostX1Gate_JumpToX1(void) {}
uint8_t HostA2_SetOtaServiceFlag(void) { return 1U; }
uint8_t HostA2_ShouldHandoffToA1(uint8_t u8CmdCode) { (void)u8CmdCode; return 0U; }
void HostA2_EnterOtaService(void) {}
void HostA2_ReplyUpgradeStatusFromCan(can_receive_message_struct *can_receive_message, uint8_t u8CANChannel)
{
    (void)can_receive_message;
    (void)u8CANChannel;
}
void HostA2_ReplyIntoUpgradeMode(can_receive_message_struct *can_receive_message, uint8_t u8Result)
{
    (void)can_receive_message;
    (void)u8Result;
}
void HostA2_CanSendExtMsg(uint8_t u8CmdType, uint8_t u8CmdCode, const uint8_t *pu8Data, uint32_t u32Dlc)
{
    (void)u8CmdType;
    (void)u8CmdCode;
    (void)pu8Data;
    (void)u32Dlc;
}

#endif /* OTA_UPGRADE_ENABLE */
