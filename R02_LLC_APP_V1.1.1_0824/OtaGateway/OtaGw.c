#include "UpgradeFeatureConfig.h"
#if OTA_UPGRADE_ENABLE

#include "OtaGw.h"
#include "OtaCanTypes.h"
#include "AppA2CanCmd.h"
#include <string.h>
#include "HostX1Gate.h"
#include "LlcOta.h"
#include "UpgradeMsgSaveFunctionDf.h"
#include "OtaHandoffDevType.h"
#include "HostA2Inquire.h"
#include "HostA2FactoryInfo.h"
#include "mx25L1606E.h"
#include "CanFdTxQueue.h"

#define OTA_GW_CAN_TX_DRAIN_MS    50U
#define OTA_GW_CAN_TX_FIFO_FREE   3U
#define OTA_GW_RX_QUEUE_DEPTH     4U
#define OTA_GW_POLL_MAX_PER_TICK  2U
#define OTA_GW_TRAFFIC_HOLD_MS    80U
/* PFC IAP/Flash 期间 LLC 收不到 86B 应答，暂停 SCI 看门狗 */
#define OTA_GW_SCI_HOLD_MS        120000U



static uint8_t s_u8OtaRxPending = 0U;
static can_receive_message_struct s_aOtaRxQueue[OTA_GW_RX_QUEUE_DEPTH];
static uint8_t s_u8OtaRxHead = 0U;
static uint8_t s_u8OtaRxTail = 0U;
static uint8_t s_u8OtaRxCount = 0U;

static uint8_t s_u8HandoffActive = 0U;
OtaHandoffState_t s_eHandoffState = OTA_HANDOFF_IDLE;
static uint8_t s_u8HandoffNeedB0Reply = 0U;
static can_receive_message_struct s_stHandoffMsg;
static uint32_t s_u32LastOtaRxMs = 0U;
static uint8_t s_u8OtaRxSeen = 0U;

static uint8_t OtaGw_IsHandoffCmd(uint8_t u8Cmd);
static void OtaGw_BeginHandoff(can_receive_message_struct *pMsg);

static uint8_t OtaGw_GetDataLen(FDCAN_RxHeaderTypeDef *pHeader)
{
    return OtaGw_GetRxPayloadBytes(pHeader);
}

uint8_t OtaGw_GetRxPayloadBytes(const FDCAN_RxHeaderTypeDef *pHeader)
{
    static const uint8_t u8CanFDRecDataLenTab[16] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 12, 16, 20, 24, 32, 48, 64
    };
    uint32_t u32Dlc;

    if (pHeader == NULL)
    {
        return 0U;
    }
    /* HAL_FDCAN_GetRxMessage 已将 DataLength 解为 DLC 编码 0..15，不可再 >>16 */
    u32Dlc = pHeader->DataLength;
    if (u32Dlc > 15U)
    {
        u32Dlc = (pHeader->DataLength >> 16) & 0x0FU;
    }
    return u8CanFDRecDataLenTab[u32Dlc];
}

static uint8_t OtaGw_IsGwExtFrame(uint32_t u32Id)
{
    _UStCanIDMsg idMsg;

    idMsg.all = u32Id;
    if (idMsg.StCanIDMsgBit.DstAddr != DF_CAN_MASTER_ADDR)
    {
        return 0U;
    }
    if (idMsg.StCanIDMsgBit.CmdType == DF_UPGRADE_CMD_TYPE
        || idMsg.StCanIDMsgBit.CmdType == DF_INQURE_DEV_MSG_CMD_TYPE)
    {
        return 1U;
    }
    return 0U;
}

static uint8_t OtaGw_GetCmdCode(const can_receive_message_struct *pMsg)
{
    _UStCanIDMsg idMsg;

    idMsg.all = pMsg->CANRxHeader.Identifier;
    return (uint8_t)idMsg.StCanIDMsgBit.CmdCode;
}

static uint8_t OtaGw_GetCmdType(const can_receive_message_struct *pMsg)
{
    _UStCanIDMsg idMsg;

    idMsg.all = pMsg->CANRxHeader.Identifier;
    return (uint8_t)idMsg.StCanIDMsgBit.CmdType;
}

static void OtaGw_QueuePush(const can_receive_message_struct *pMsg)
{
    if (pMsg == NULL)
    {
        return;
    }
    if (s_u8OtaRxCount >= OTA_GW_RX_QUEUE_DEPTH)
    {
        s_u8OtaRxTail = (uint8_t)((s_u8OtaRxTail + 1U) % OTA_GW_RX_QUEUE_DEPTH);
        s_u8OtaRxCount--;
    }
    s_aOtaRxQueue[s_u8OtaRxHead] = *pMsg;
    s_u8OtaRxHead = (uint8_t)((s_u8OtaRxHead + 1U) % OTA_GW_RX_QUEUE_DEPTH);
    s_u8OtaRxCount++;
    s_u8OtaRxPending = 1U;
    s_u8OtaRxSeen = 1U;
    s_u32LastOtaRxMs = HAL_GetTick();
}

static uint8_t OtaGw_QueuePop(can_receive_message_struct *pMsg)
{
    if (s_u8OtaRxCount == 0U || pMsg == NULL)
    {
        return 0U;
    }
    *pMsg = s_aOtaRxQueue[s_u8OtaRxTail];
    s_u8OtaRxTail = (uint8_t)((s_u8OtaRxTail + 1U) % OTA_GW_RX_QUEUE_DEPTH);
    s_u8OtaRxCount--;
    if (s_u8OtaRxCount == 0U)
    {
        s_u8OtaRxPending = 0U;
    }
    return 1U;
}

static uint8_t OtaGw_QueuePopPreferB5(can_receive_message_struct *pMsg)
{
    uint8_t i;
    uint8_t u8Idx;

    if (s_u8OtaRxCount == 0U || pMsg == NULL)
    {
        return 0U;
    }

    for (i = 0U; i < s_u8OtaRxCount; i++)
    {
        u8Idx = (uint8_t)((s_u8OtaRxTail + i) % OTA_GW_RX_QUEUE_DEPTH);
        if (OtaGw_GetCmdCode(&s_aOtaRxQueue[u8Idx]) == DF_CAN_UPGRADE_STATUS_INQURE_CMD)
        {
            *pMsg = s_aOtaRxQueue[u8Idx];
            while ((i + 1U) < s_u8OtaRxCount)
            {
                uint8_t u8From = (uint8_t)((s_u8OtaRxTail + i + 1U) % OTA_GW_RX_QUEUE_DEPTH);
                uint8_t u8To = (uint8_t)((s_u8OtaRxTail + i) % OTA_GW_RX_QUEUE_DEPTH);

                s_aOtaRxQueue[u8To] = s_aOtaRxQueue[u8From];
                i++;
            }
            s_u8OtaRxHead = (uint8_t)((s_u8OtaRxHead + OTA_GW_RX_QUEUE_DEPTH - 1U) % OTA_GW_RX_QUEUE_DEPTH);
            s_u8OtaRxCount--;
            if (s_u8OtaRxCount == 0U)
            {
                s_u8OtaRxPending = 0U;
            }
            return 1U;
        }
    }

    return OtaGw_QueuePop(pMsg);
}
volatile uint8_t u8Cmd;
volatile uint8_t u8CmdType;
static void OtaGw_HandleRxMessage(can_receive_message_struct *pMsg)
{

    if (pMsg == NULL)
    {
        return;
    }

    u8Cmd = OtaGw_GetCmdCode(pMsg);
    u8CmdType = OtaGw_GetCmdType(pMsg);

    if (u8CmdType == DF_INQURE_DEV_MSG_CMD_TYPE)
    {
        switch (u8Cmd)
        {
        case DF_CAN_INQURE_DEV_MSG_CMD:
            HostA2_ReplyInquireDevMsgFromCan(pMsg);
            break;
        case DF_CAN_INQURE_SLAVE_DEV_MSG_CMD:
            HostA2_ReplyInquireSlaveDevMsgFromCan(pMsg);
            break;
        case DF_CAN_INQURE_HOST_DEV_EXT_CMD:
            HostA2_ReplyInquireHostDevExtFromCan(pMsg);
            break;
        case DF_CAN_FACTORY_INFO_READ_CMD:
            HostA2_ReplyFactoryInfoReadFromCan(pMsg);
            break;
        case DF_CAN_FACTORY_INFO_WRITE_CMD:
            HostA2_QueueFactoryInfoWriteFromCan(pMsg);
            break;
        case DF_CAN_FACTORY_INFO_DELETE_CMD:
            HostA2_QueueFactoryInfoDeleteFromCan(pMsg);
            break;
        default:
            break;
        }
        return;
    }

    if (u8Cmd == DF_CAN_UPGRADE_STATUS_INQURE_CMD)
    {
				u8Cmd = DF_CAN_UPGRADE_STATUS_INQURE_CMD;
        HostA2_ReplyUpgradeStatusFromCan(pMsg, DF_CAN1_CHANN);
        return;
    }

    if (OtaGw_IsHandoffCmd(u8Cmd) != 0U)
    {
        can_receive_message_struct stHandoffMsg = *pMsg;

        OtaGw_BeginHandoff(&stHandoffMsg);
    }
}

static uint8_t OtaGw_IsHandoffCmd(uint8_t u8Cmd)
{
    if (u8Cmd == DF_CAN_INTO_UPGRADE_MODE_CMD
        || u8Cmd == DF_CAN_START_UPGRADE_CMD
        || u8Cmd == DF_CAN_FILE_TRANSFER_REQUEST_CMD
        || u8Cmd == DF_CAN_FILE_TRANSFER_CMD
        || u8Cmd == DF_CAN_FILE_TRANSFER_END_CMD)
    {
        return 1U;
    }
    return 0U;
}

static void OtaGw_AbortHandoff(uint8_t u8SendB0Fail)
{
    if (u8SendB0Fail != 0U && s_u8HandoffNeedB0Reply != 0U)
    {
        HostA2_ReplyIntoUpgradeMode(&s_stHandoffMsg, DF_CAN_INTO_UPGRADE_MODE_ACK_FAIL);
    }

    s_u8HandoffActive = 0U;
    s_eHandoffState = OTA_HANDOFF_IDLE;
    s_u8HandoffNeedB0Reply = 0U;
}

static uint8_t OtaGw_IsCanTxFifoDrained(void)
{
    return CanFdTx_IsFullyDrained(&hfdcan1);
}

uint8_t OtaGw_IsTrafficActive(void)
{
    if (s_u8HandoffActive != 0U)
    {
        return 1U;
    }
    if (s_u8OtaRxPending != 0U || s_u8OtaRxCount > 0U)
    {
        return 1U;
    }
    if (s_u8OtaRxSeen != 0U
        && (uint32_t)(HAL_GetTick() - s_u32LastOtaRxMs) < OTA_GW_TRAFFIC_HOLD_MS)
    {
        return 1U;
    }
    return 0U;
}

uint8_t OtaGw_IsSciWatchdogPaused(void)
{
    if (s_u8HandoffActive != 0U)
    {
        return 1U;
    }
    if (OtaGw_IsTrafficActive() != 0U)
    {
        return 1U;
    }
    if (s_u8OtaRxSeen != 0U
        && (uint32_t)(HAL_GetTick() - s_u32LastOtaRxMs) < OTA_GW_SCI_HOLD_MS)
    {
        return 1U;
    }
    return 0U;
}

static void OtaGw_BeginHandoff(can_receive_message_struct *pMsg)
{
    uint8_t u8Cmd;

    if (pMsg == NULL || s_u8HandoffActive != 0U)
    {
        return;
    }

    u8Cmd = OtaGw_GetCmdCode(pMsg);
    if (HostA2_ShouldHandoffToA1(u8Cmd) == 0U)
    {
        return;
    }

    if (LlcOta_IsRunStateBlockingUpgrade() != 0U)
    {
        if (u8Cmd == DF_CAN_INTO_UPGRADE_MODE_CMD)
        {
            HostA2_ReplyIntoUpgradeMode(pMsg, DF_CAN_INTO_UPGRADE_MODE_ACK_FAIL);
        }
        return;
    }

    LlcOta_PreHandoffShutdown();
    LlcOta_StartPfcSafeRequest();

    s_stHandoffMsg = *pMsg;
    s_u8HandoffNeedB0Reply = (u8Cmd == DF_CAN_INTO_UPGRADE_MODE_CMD) ? 1U : 0U;
    s_u8HandoffActive = 1U;
    s_eHandoffState = OTA_HANDOFF_WAIT_PFC;
}

void OtaGw_Init(void)
{
    MX25L1606E_Flash_Init();
    (void)ReadWriteUpgradeFileMsg(&StMasterProgStatus, DF_MASTET_FIRMWARE_MSG_START_ADDRESS);
    HostA2Inquire_Init();
    HostA2FactoryInfo_Init();
}

void OtaGw_RxIsrHook(FDCAN_RxHeaderTypeDef *pHeader, const uint8_t *pu8Data)
{
    uint8_t u8Len;

    if (pHeader == NULL || pu8Data == NULL)
    {
        return;
    }
    if (pHeader->IdType != FDCAN_EXTENDED_ID)
    {
        return;
    }
    if (OtaGw_IsGwExtFrame(pHeader->Identifier) == 0U)
    {
        return;
    }

    u8Len = OtaGw_GetDataLen(pHeader);
    if (u8Len > DF_CAN_DATA_LEN)
    {
        u8Len = DF_CAN_DATA_LEN;
    }

    {
        can_receive_message_struct stMsg;

        stMsg.CANRxHeader = *pHeader;
        memset(stMsg.rx_data, 0, sizeof(stMsg.rx_data));
        memcpy(stMsg.rx_data, pu8Data, u8Len);
        OtaGw_QueuePush(&stMsg);
    }
}

void OtaGw_Poll(void)
{
    can_receive_message_struct msg;
    uint8_t u8Handled;

    if (s_u8OtaRxPending == 0U)
    {
        return;
    }

    for (u8Handled = 0U; u8Handled < OTA_GW_POLL_MAX_PER_TICK; u8Handled++)
    {
        if (OtaGw_QueuePopPreferB5(&msg) == 0U)
        {
            break;
        }
        OtaGw_HandleRxMessage(&msg);
    }

    if (s_u8OtaRxCount == 0U)
    {
        s_u8OtaRxPending = 0U;
    }
}

void OtaGw_HandoffTask(void)
{
    uint8_t u8PfcResult;
    static uint32_t s_u32CanTxWaitStartMs = 0U;

    if (s_u8HandoffActive == 0U)
    {
        return;
    }

    switch (s_eHandoffState)
    {
    case OTA_HANDOFF_WAIT_PFC:
        u8PfcResult = LlcOta_PollPfcSafeRequest();
        if (u8PfcResult == 0U)
        {
            break;
        }
        if (u8PfcResult == 2U)
        {
            OtaGw_AbortHandoff(1U);
            break;
        }

        if (s_u8HandoffNeedB0Reply != 0U)
        {
            HostA2_ReplyIntoUpgradeMode(&s_stHandoffMsg, DF_CAN_INTO_UPGRADE_MODE_ACK_SUCC);
            s_u32CanTxWaitStartMs = HAL_GetTick();
            s_eHandoffState = OTA_HANDOFF_WAIT_CAN_TX;
        }
        else
        {
            s_eHandoffState = OTA_HANDOFF_DO_JUMP;
        }
        break;

    case OTA_HANDOFF_WAIT_CAN_TX:
        if (OtaGw_IsCanTxFifoDrained() != 0U)
        {
            s_eHandoffState = OTA_HANDOFF_DO_JUMP;
            break;
        }
        if ((uint32_t)(HAL_GetTick() - s_u32CanTxWaitStartMs) >= OTA_GW_CAN_TX_DRAIN_MS)
        {
            s_eHandoffState = OTA_HANDOFF_DO_JUMP;
        }
        break;

    case OTA_HANDOFF_DO_JUMP:
        if (s_u8HandoffNeedB0Reply != 0U)
        {
            uint8_t u8Dev = s_stHandoffMsg.rx_data[0];

            if (u8Dev == (uint8_t)(Df_UPGRADE_DEV_SLAVE1 & 0xFFU)
                || u8Dev == (uint8_t)(Df_UPGRADE_DEV_MERGED_PACK & 0xFFU))
            {
                OtaHandoffDevType_Write(u8Dev);
            }
        }
        s_u8HandoffActive = 0U;
        s_eHandoffState = OTA_HANDOFF_IDLE;
        s_u8HandoffNeedB0Reply = 0U;
        HostA2_EnterOtaService();
        break;

    default:
        OtaGw_AbortHandoff(0U);
        break;
    }
}

#else /* OTA_UPGRADE_ENABLE */

#include "OtaGw.h"

OtaHandoffState_t s_eHandoffState = OTA_HANDOFF_IDLE;

void OtaGw_Init(void) {}
void OtaGw_RxIsrHook(FDCAN_RxHeaderTypeDef *pHeader, const uint8_t *pu8Data)
{
    (void)pHeader;
    (void)pu8Data;
}
void OtaGw_Poll(void) {}
void OtaGw_HandoffTask(void) {}
uint8_t OtaGw_IsTrafficActive(void) { return 0U; }
uint8_t OtaGw_IsSciWatchdogPaused(void) { return 0U; }

#endif /* OTA_UPGRADE_ENABLE */
