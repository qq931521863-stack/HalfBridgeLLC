/******************************************************************************
    HostA2Inquire.c — APPA2 本地应答设备信息查询（仅 CmdType 0x03，不触发 A1 跳转）
******************************************************************************/
#include "UpgradeFeatureConfig.h"
#if OTA_UPGRADE_ENABLE

#include "HostA2Inquire.h"
#include "HostX1Gate.h"
#include "AppA2CanCmd.h"
#include "UpgradeMsgSaveFunctionDf.h"
#include "main.h"
#include <string.h>

extern const char Set_Chrgr_AppRevs[3];
extern const char Set_Chrgr_BootLoadRevs[3];
extern const char Set_Chrgr_HardWareRevs[3];

#define DF_DEV_RUN_MODE_APP   0x0000U

#pragma pack(1)
typedef struct
{
    uint16_t u16DevType;
    uint16_t u16HardwareVersion;
    uint16_t u16SoftVersion;
    uint16_t u16SysRunTime;
    uint16_t u16DevOnlineStatus;
} HostA2_DevMsg_t;

typedef struct
{
    uint16_t u16DevType;
    uint16_t u16HardwareVersion;
    uint16_t u16SoftVersion;
    uint16_t u16SysRunTime;
    uint16_t u16DevOnlineStatus;
    uint16_t u16RunMode;
    uint16_t u16ActiveSlot;
    uint16_t u16SlotAFwVer;
    uint16_t u16SlotBFwVer;
    uint16_t u16PowerState;
} HostA2_SlaveExtMsg_t;

typedef struct
{
    uint16_t u16DevType;
    uint16_t u16HardwareVersion;
    uint16_t u16SoftVersion;
    uint16_t u16SysRunTime;
    uint16_t u16DevOnlineStatus;
    uint16_t u16RunMode;
    uint16_t u16ActiveSlot;
    uint16_t u16SlotAFwVer;
    uint16_t u16SlotBFwVer;
    uint16_t u16BootVersion;
} HostA2_HostExtMsg_t;
#pragma pack()

static HostA2_DevMsg_t s_stDevMsg;
static HostA2_SlaveExtMsg_t s_stSlaveExtMsg;
static uint16_t s_u16BootVersion = 0U;

static uint16_t HostA2_DecRevToU16(const char rev[3])
{
    if (rev == NULL)
    {
        return 0U;
    }
    if (rev[0] >= '0' && rev[0] <= '9')
    {
        return (uint16_t)(((uint16_t)(rev[0] - '0') * 100U)
                        + ((uint16_t)(rev[1] - '0') * 10U)
                        + (uint16_t)(rev[2] - '0'));
    }
    return (uint16_t)(((uint16_t)(uint8_t)rev[0] << 8) | (uint16_t)(uint8_t)rev[2]);
}

static uint16_t HostA2_NormalizeSlotVer(uint16_t u16Ver, uint8_t u8Valid, uint16_t u16Fallback)
{
    if (u16Ver == 0U || u16Ver == 0xFFFFU)
    {
        if (u8Valid == DF_UPGRADE_FILE_VAILE_STATUS)
        {
            return u16Fallback;
        }
        return 0U;
    }
    return u16Ver;
}

static void HostA2_RefreshProgStatus(void)
{
    (void)ReadWriteUpgradeFileMsg(&StMasterProgStatus, DF_MASTET_FIRMWARE_MSG_START_ADDRESS);
}

static void HostA2_FillHostExt(HostA2_HostExtMsg_t *pExt)
{
    uint16_t u16Active;
    uint16_t u16SlotA;
    uint16_t u16SlotB;

    if (pExt == NULL)
    {
        return;
    }

    HostA2_RefreshProgStatus();
    u16Active = (uint16_t)StMasterProgStatus.StProgStatusMsg.u8FlashUserBlock;
    if (u16Active != (uint16_t)DF_CHS_BLOCK_A && u16Active != (uint16_t)DF_CHS_BLOCK_B)
    {
        u16Active = (uint16_t)DF_CHS_BLOCK_A;
    }

    pExt->u16DevType         = s_stDevMsg.u16DevType;
    pExt->u16HardwareVersion = s_stDevMsg.u16HardwareVersion;
    pExt->u16SoftVersion     = s_stDevMsg.u16SoftVersion;
    pExt->u16SysRunTime      = s_stDevMsg.u16SysRunTime;
    pExt->u16DevOnlineStatus = s_stDevMsg.u16DevOnlineStatus;
    pExt->u16RunMode         = DF_DEV_RUN_MODE_APP;
    pExt->u16ActiveSlot      = u16Active;
    pExt->u16BootVersion     = s_u16BootVersion;

    u16SlotA = HostA2_NormalizeSlotVer(StMasterProgStatus.StProgStatusMsg.u16BlockAFirmwareVersion,
                                       StMasterProgStatus.StProgStatusMsg.u8BlockABinFileValidFlag,
                                       StMasterProgStatus.StProgStatusMsg.u16BlockABinFlieCrc16);
    u16SlotB = HostA2_NormalizeSlotVer(StMasterProgStatus.StProgStatusMsg.u16BlockBFirmwareVersion,
                                       StMasterProgStatus.StProgStatusMsg.u8BlockBBinFileValidFlag,
                                       StMasterProgStatus.StProgStatusMsg.u16BlockBBinFlieCrc16);
    if (u16Active == (uint16_t)DF_CHS_BLOCK_A && u16SlotA == 0U)
    {
        u16SlotA = s_stDevMsg.u16SoftVersion;
    }
    else if (u16Active == (uint16_t)DF_CHS_BLOCK_B && u16SlotB == 0U)
    {
        u16SlotB = s_stDevMsg.u16SoftVersion;
    }
    pExt->u16SlotAFwVer = u16SlotA;
    pExt->u16SlotBFwVer = u16SlotB;
}

void HostA2Inquire_Init(void)
{
    memset(&s_stDevMsg, 0, sizeof(s_stDevMsg));
    memset(&s_stSlaveExtMsg, 0, sizeof(s_stSlaveExtMsg));

    s_stDevMsg.u16DevType         = Df_UPGRADE_DEV_MASTET;
    s_stDevMsg.u16HardwareVersion = HostA2_DecRevToU16(Set_Chrgr_HardWareRevs);
    s_stDevMsg.u16SoftVersion     = HostA2_DecRevToU16(Set_Chrgr_AppRevs);
    s_stDevMsg.u16DevOnlineStatus = 0x0001U;
    s_u16BootVersion = HostA2_DecRevToU16(Set_Chrgr_BootLoadRevs);

    HostA2_RefreshProgStatus();
}

void HostA2Inquire_SyncIdentityFromFactory(const char app_rev[3], const char hw_rev[3], const char boot_rev[3])
{
    s_stDevMsg.u16HardwareVersion = HostA2_DecRevToU16(hw_rev);
    s_stDevMsg.u16SoftVersion     = HostA2_DecRevToU16(app_rev);
    s_u16BootVersion              = HostA2_DecRevToU16(boot_rev);
}

void HostA2_ReplyInquireDevMsgFromCan(const can_receive_message_struct *pMsg)
{
    uint8_t u8TxBuf[12];

    (void)pMsg;
    memcpy(u8TxBuf, &s_stDevMsg, sizeof(s_stDevMsg));
    u8TxBuf[10] = (uint8_t)(s_u16BootVersion & 0xFFU);
    u8TxBuf[11] = (uint8_t)((s_u16BootVersion >> 8) & 0xFFU);
    HostA2_CanSendExtMsg(DF_INQURE_DEV_MSG_CMD_TYPE, DF_CAN_INQURE_DEV_MSG_CMD, u8TxBuf, FDCAN_DLC_BYTES_12);
}

void HostA2_ReplyInquireSlaveDevMsgFromCan(const can_receive_message_struct *pMsg)
{
    (void)pMsg;
    HostA2_CanSendExtMsg(DF_INQURE_DEV_MSG_CMD_TYPE, DF_CAN_INQURE_SLAVE_DEV_MSG_CMD,
                         (const uint8_t *)&s_stSlaveExtMsg, FDCAN_DLC_BYTES_20);
}

void HostA2_ReplyInquireHostDevExtFromCan(const can_receive_message_struct *pMsg)
{
    HostA2_HostExtMsg_t stExt;

    (void)pMsg;
    HostA2_FillHostExt(&stExt);
    HostA2_CanSendExtMsg(DF_INQURE_DEV_MSG_CMD_TYPE, DF_CAN_INQURE_HOST_DEV_EXT_CMD,
                         (const uint8_t *)&stExt, FDCAN_DLC_BYTES_20);
}

#else /* OTA_UPGRADE_ENABLE */

#include "HostA2Inquire.h"
#include "OtaCanTypes.h"

void HostA2Inquire_Init(void) {}
void HostA2Inquire_SyncIdentityFromFactory(const char app_rev[3], const char hw_rev[3], const char boot_rev[3])
{
    (void)app_rev;
    (void)hw_rev;
    (void)boot_rev;
}
void HostA2_ReplyInquireDevMsgFromCan(const can_receive_message_struct *pMsg) { (void)pMsg; }
void HostA2_ReplyInquireSlaveDevMsgFromCan(const can_receive_message_struct *pMsg) { (void)pMsg; }
void HostA2_ReplyInquireHostDevExtFromCan(const can_receive_message_struct *pMsg) { (void)pMsg; }

#endif /* OTA_UPGRADE_ENABLE */
