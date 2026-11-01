/******************************************************************************
    HostA2FactoryInfo.c — 产测出厂信息：CAN 仅校验/应答/置 pending，外扩 Flash 在 main while 写入
******************************************************************************/
#include "UpgradeFeatureConfig.h"
#if OTA_UPGRADE_ENABLE

#include "HostA2FactoryInfo.h"
#include "HostA2Inquire.h"
#include "HostX1Gate.h"
#include "AppA2CanCmd.h"
#include "HostFlashMemoryMap.h"
#include "OtaGw.h"
#include "mx25L1606E.h"
#include "CAN_Control_2800W.h"
#include <string.h>

extern const char Set_Chrgr_AppRevs[3];
extern const char Set_Chrgr_BootLoadRevs[3];
extern const char Set_Chrgr_HardWareRevs[3];
extern const char Set_Chrgr_PartNumberRevs1[8];
extern const char Set_Chrgr_PartNumberRevs2[6];

#define FACTORY_INFO_MAGIC              0x46414354U
#define FACTORY_INFO_WIRE_SIZE          52U
#define FACTORY_INFO_PART_NO_LEN        10U
#define FACTORY_INFO_SUPPLIER_LEN       6U
#define FACTORY_INFO_SERIAL_LEN         7U
#define FACTORY_INFO_REWORK_LEN         1U
#define FACTORY_INFO_RECORD_VER         0x02U
#define FACTORY_FLAG_SN_VALID           0x01U

#define FACTORY_SUBCMD_DELETE           0x02U
#define FACTORY_UNLOCK0                 0xD0U
#define FACTORY_UNLOCK1                 0xB1U
#define FACTORY_UNLOCK2                 0x4EU

#define FACTORY_RESULT_OK               0x00U
#define FACTORY_RESULT_PARAM            0x01U
#define FACTORY_RESULT_FLASH            0x02U
#define FACTORY_RESULT_BUSY             0x03U
#define FACTORY_RESULT_UNLOCK           0x07U

#pragma pack(1)
typedef struct
{
    uint32_t magic;
    uint8_t version;
    uint8_t flags;
    uint8_t sales_region;
    uint8_t reserved0;
    char app_rev[3];
    char hw_rev[3];
    char boot_rev[3];
    char part_no[FACTORY_INFO_PART_NO_LEN];
    char supplier_code[FACTORY_INFO_SUPPLIER_LEN];
    char serial_no[FACTORY_INFO_SERIAL_LEN];
    char rework_code;
    char reserved_pad[6];
    uint16_t mfg_year;
    uint8_t mfg_month;
    uint8_t mfg_day;
    uint8_t pad;
} FactoryInfoWire_t;
#pragma pack()

typedef enum
{
    FACTORY_FLASH_IDLE = 0,
    FACTORY_FLASH_PENDING_WRITE
} FactoryFlashPending_t;

static FactoryInfoWire_t s_stFactoryRam;
static FactoryFlashPending_t s_eFlashPending = FACTORY_FLASH_IDLE;

static void HostA2FactoryInfo_SendResult(uint8_t u8Cmd, uint8_t u8Result)
{
    uint8_t data[8];

    memset(data, 0, sizeof(data));
    data[0] = u8Result;
    HostA2_CanSendExtMsg(DF_INQURE_DEV_MSG_CMD_TYPE, u8Cmd, data, FDCAN_DLC_BYTES_8);
}

static void HostA2FactoryInfo_CopyAsciiField(char *dst, const char *src, uint8_t u8Len)
{
    uint8_t i;

    if (dst == NULL || u8Len == 0U)
    {
        return;
    }
    for (i = 0U; i < u8Len; i++)
    {
        dst[i] = 0;
    }
    if (src == NULL)
    {
        return;
    }
    for (i = 0U; i < u8Len && src[i] != '\0'; i++)
    {
        dst[i] = src[i];
    }
}

static void HostA2FactoryInfo_ApplyCompileDefaults(FactoryInfoWire_t *pRec)
{
    if (pRec == NULL)
    {
        return;
    }

    memset(pRec, 0, sizeof(*pRec));
    pRec->magic = FACTORY_INFO_MAGIC;
    pRec->version = FACTORY_INFO_RECORD_VER;
    pRec->flags = 0U;
    pRec->sales_region = 0U;
    HostA2FactoryInfo_CopyAsciiField(pRec->app_rev, Set_Chrgr_AppRevs, 3U);
    HostA2FactoryInfo_CopyAsciiField(pRec->hw_rev, Set_Chrgr_HardWareRevs, 3U);
    HostA2FactoryInfo_CopyAsciiField(pRec->boot_rev, Set_Chrgr_BootLoadRevs, 3U);
    HostA2FactoryInfo_CopyAsciiField(pRec->part_no, Set_Chrgr_PartNumberRevs1, 8U);
    pRec->part_no[8] = '0';
    pRec->part_no[9] = '0';
    HostA2FactoryInfo_CopyAsciiField(pRec->supplier_code, Set_Chrgr_PartNumberRevs2, 6U);
    pRec->rework_code = '0';
}

static uint8_t HostA2FactoryInfo_IsAsciiPrintable(const char *p, uint8_t u8Len, uint8_t u8AllowEmpty)
{
    uint8_t i;
    uint8_t u8NonZero = 0U;

    if (p == NULL)
    {
        return 0U;
    }
    for (i = 0U; i < u8Len; i++)
    {
        if (p[i] == '\0' || p[i] == ' ')
        {
            continue;
        }
        u8NonZero = 1U;
        if (p[i] < 0x20 || p[i] > 0x7E)
        {
            return 0U;
        }
    }
    return (u8AllowEmpty != 0U || u8NonZero != 0U) ? 1U : 0U;
}

static uint8_t HostA2FactoryInfo_IsAlnumField(const char *p, uint8_t u8Len)
{
    uint8_t i;

    if (p == NULL)
    {
        return 0U;
    }
    for (i = 0U; i < u8Len; i++)
    {
        char c = p[i];
        if (!((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')))
        {
            return 0U;
        }
    }
    return 1U;
}

static uint8_t HostA2FactoryInfo_IsDigitField(const char *p, uint8_t u8Len)
{
    uint8_t i;

    if (p == NULL)
    {
        return 0U;
    }
    for (i = 0U; i < u8Len; i++)
    {
        if (p[i] < '0' || p[i] > '9')
        {
            return 0U;
        }
    }
    return 1U;
}

static uint8_t HostA2FactoryInfo_IsValidRework(char cRework)
{
    return (cRework >= '0' && cRework <= '9') ? 1U : 0U;
}

static void HostA2FactoryInfo_UpperAlnumField(char *p, uint8_t u8Len)
{
    uint8_t i;

    if (p == NULL)
    {
        return;
    }
    for (i = 0U; i < u8Len; i++)
    {
        if (p[i] >= 'a' && p[i] <= 'z')
        {
            p[i] = (char)(p[i] - ('a' - 'A'));
        }
    }
}

static uint8_t HostA2FactoryInfo_IsValidDate(uint16_t u16Year, uint8_t u8Month, uint8_t u8Day)
{
    if (u16Year < 2000U || u8Month < 1U || u8Month > 12U || u8Day < 1U || u8Day > 31U)
    {
        return 0U;
    }
    return 1U;
}

static uint8_t HostA2FactoryInfo_IsValidHwRev(const char *pRev)
{
    if (pRev == NULL)
    {
        return 0U;
    }
    if (pRev[0] != 'H' || pRev[1] != '.')
    {
        return 0U;
    }
    if (pRev[2] < '0' || pRev[2] > '9')
    {
        return 0U;
    }
    return 1U;
}

static uint8_t HostA2FactoryInfo_ValidateWireRecord(const FactoryInfoWire_t *pRec)
{
    if (pRec == NULL)
    {
        return 0U;
    }
    if (pRec->magic != FACTORY_INFO_MAGIC || pRec->version != FACTORY_INFO_RECORD_VER)
    {
        return 0U;
    }
    if (pRec->sales_region > 7U)
    {
        return 0U;
    }
    if (HostA2FactoryInfo_IsAsciiPrintable(pRec->app_rev, 3U, 0U) == 0U)
    {
        return 0U;
    }
    if (HostA2FactoryInfo_IsValidHwRev(pRec->hw_rev) == 0U)
    {
        return 0U;
    }
    if (HostA2FactoryInfo_IsAlnumField(pRec->part_no, FACTORY_INFO_PART_NO_LEN) == 0U)
    {
        return 0U;
    }
    if (HostA2FactoryInfo_IsDigitField(pRec->supplier_code, FACTORY_INFO_SUPPLIER_LEN) == 0U)
    {
        return 0U;
    }
    if (HostA2FactoryInfo_IsAlnumField(pRec->serial_no, FACTORY_INFO_SERIAL_LEN) == 0U)
    {
        return 0U;
    }
    if (HostA2FactoryInfo_IsValidRework(pRec->rework_code) == 0U)
    {
        return 0U;
    }
    if (HostA2FactoryInfo_IsValidDate(pRec->mfg_year, pRec->mfg_month, pRec->mfg_day) == 0U)
    {
        return 0U;
    }
    return 1U;
}

static uint8_t HostA2FactoryInfo_GetRxDataLen(const can_receive_message_struct *pMsg)
{
    if (pMsg == NULL)
    {
        return 0U;
    }
    return OtaGw_GetRxPayloadBytes(&pMsg->CANRxHeader);
}

static uint8_t HostA2FactoryInfo_IsBusyContext(void)
{
    if (s_eFlashPending != FACTORY_FLASH_IDLE)
    {
        return 1U;
    }
    if (s_eHandoffState != OTA_HANDOFF_IDLE)
    {
        return 1U;
    }
    return 0U;
}

static void HostA2FactoryInfo_NormalizeWriteRecord(FactoryInfoWire_t *pRec)
{
    char cHwDigit;
    uint8_t i;

    if (pRec == NULL)
    {
        return;
    }
    HostA2FactoryInfo_UpperAlnumField(pRec->part_no, FACTORY_INFO_PART_NO_LEN);
    HostA2FactoryInfo_UpperAlnumField(pRec->serial_no, FACTORY_INFO_SERIAL_LEN);
    if (HostA2FactoryInfo_IsValidRework(pRec->rework_code) == 0U)
    {
        pRec->rework_code = '0';
    }
    for (i = 0U; i < 6U; i++)
    {
        pRec->reserved_pad[i] = 0;
    }
    HostA2FactoryInfo_CopyAsciiField(pRec->app_rev, Set_Chrgr_AppRevs, 3U);
    cHwDigit = Set_Chrgr_HardWareRevs[2];
    if (pRec->hw_rev[2] >= '0' && pRec->hw_rev[2] <= '9')
    {
        cHwDigit = pRec->hw_rev[2];
    }
    pRec->hw_rev[0] = 'H';
    pRec->hw_rev[1] = '.';
    pRec->hw_rev[2] = cHwDigit;
    HostA2FactoryInfo_CopyAsciiField(pRec->boot_rev, Set_Chrgr_BootLoadRevs, 3U);
    pRec->magic = FACTORY_INFO_MAGIC;
    pRec->version = FACTORY_INFO_RECORD_VER;
    pRec->flags |= FACTORY_FLAG_SN_VALID;
    pRec->reserved0 = 0U;
    pRec->pad = 0U;
}

static void HostA2FactoryInfo_SaveToFlash(void)
{
    FlashSector_Erase(DF_HOST_FACTORY_INFO_EXT_ADDR);
    SPI_FLASH_BufferWrite((uint8_t *)&s_stFactoryRam, DF_HOST_FACTORY_INFO_EXT_ADDR, FACTORY_INFO_WIRE_SIZE);
}

static void HostA2FactoryInfo_LoadFromFlash(void)
{
    FactoryInfoWire_t stTmp;

    FlashRead((uint8_t *)&stTmp, DF_HOST_FACTORY_INFO_EXT_ADDR, FACTORY_INFO_WIRE_SIZE);
    if (stTmp.magic == FACTORY_INFO_MAGIC && stTmp.version == FACTORY_INFO_RECORD_VER)
    {
        s_stFactoryRam = stTmp;
        return;
    }
    HostA2FactoryInfo_ApplyCompileDefaults(&s_stFactoryRam);
}

static void HostA2FactoryInfo_QueueFlashSave(void)
{
    s_eFlashPending = FACTORY_FLASH_PENDING_WRITE;
}

void HostA2FactoryInfo_FillChrgrIdentityFields(void)
{
    uint8_t i;

    for (i = 0U; i < 3U; i++)
    {
        sChrgMsg0.Chrgr_BootLoadRev[i] = s_stFactoryRam.boot_rev[i];
        sChrgMsg0.Chrgr_AppRev[i] = s_stFactoryRam.app_rev[i];
        sChrgMsg0.Chrgr_HardWareRev[i] = s_stFactoryRam.hw_rev[i];
    }
    for (i = 0U; i < 8U; i++)
    {
        sChrgMsg0.Chrgr_PartNumberRev1[i] = s_stFactoryRam.part_no[i];
    }
    for (i = 0U; i < 6U; i++)
    {
        sChrgMsg0.Chrgr_PartNumberRev2[i] = s_stFactoryRam.supplier_code[i];
    }
    sChrgMsg0.Chrgr_SalesRegion = s_stFactoryRam.sales_region;
}

static void HostA2FactoryInfo_ApplyRuntime(void)
{
    HostA2FactoryInfo_FillChrgrIdentityFields();
    HostA2Inquire_SyncIdentityFromFactory(
        s_stFactoryRam.app_rev,
        s_stFactoryRam.hw_rev,
        s_stFactoryRam.boot_rev);
}

void HostA2FactoryInfo_Init(void)
{
    s_eFlashPending = FACTORY_FLASH_IDLE;
    HostA2FactoryInfo_LoadFromFlash();
    HostA2FactoryInfo_ApplyRuntime();
}

void HostA2FactoryInfo_MainLoopTask(void)
{
    if (s_eFlashPending != FACTORY_FLASH_PENDING_WRITE)
    {
        return;
    }

    HostA2FactoryInfo_SaveToFlash();
    s_eFlashPending = FACTORY_FLASH_IDLE;
}

void HostA2_ReplyFactoryInfoReadFromCan(const can_receive_message_struct *pMsg)
{
    (void)pMsg;
    HostA2_CanSendExtMsg(DF_INQURE_DEV_MSG_CMD_TYPE,
                         DF_CAN_FACTORY_INFO_READ_CMD,
                         (const uint8_t *)&s_stFactoryRam,
                         FDCAN_DLC_BYTES_64);
}

void HostA2_QueueFactoryInfoWriteFromCan(const can_receive_message_struct *pMsg)
{
    FactoryInfoWire_t stNew;

    if (pMsg == NULL)
    {
        return;
    }
    if (HostA2FactoryInfo_IsBusyContext() != 0U)
    {
        HostA2FactoryInfo_SendResult(DF_CAN_FACTORY_INFO_WRITE_CMD, FACTORY_RESULT_BUSY);
        return;
    }
    if (HostA2FactoryInfo_GetRxDataLen(pMsg) < FACTORY_INFO_WIRE_SIZE)
    {
        HostA2FactoryInfo_SendResult(DF_CAN_FACTORY_INFO_WRITE_CMD, FACTORY_RESULT_PARAM);
        return;
    }
    memset(&stNew, 0, sizeof(stNew));
    memcpy(&stNew, pMsg->rx_data, FACTORY_INFO_WIRE_SIZE);
    HostA2FactoryInfo_NormalizeWriteRecord(&stNew);
    if (HostA2FactoryInfo_ValidateWireRecord(&stNew) == 0U)
    {
        HostA2FactoryInfo_SendResult(DF_CAN_FACTORY_INFO_WRITE_CMD, FACTORY_RESULT_PARAM);
        return;
    }

    s_stFactoryRam = stNew;
    HostA2FactoryInfo_ApplyRuntime();
    HostA2FactoryInfo_QueueFlashSave();
    HostA2FactoryInfo_SendResult(DF_CAN_FACTORY_INFO_WRITE_CMD, FACTORY_RESULT_OK);
}

void HostA2_QueueFactoryInfoDeleteFromCan(const can_receive_message_struct *pMsg)
{
    if (pMsg == NULL)
    {
        return;
    }
    if (HostA2FactoryInfo_IsBusyContext() != 0U)
    {
        HostA2FactoryInfo_SendResult(DF_CAN_FACTORY_INFO_DELETE_CMD, FACTORY_RESULT_BUSY);
        return;
    }
    if (pMsg->rx_data[0] != FACTORY_SUBCMD_DELETE
        || pMsg->rx_data[1] != FACTORY_UNLOCK0
        || pMsg->rx_data[2] != FACTORY_UNLOCK1
        || pMsg->rx_data[3] != FACTORY_UNLOCK2)
    {
        HostA2FactoryInfo_SendResult(DF_CAN_FACTORY_INFO_DELETE_CMD, FACTORY_RESULT_UNLOCK);
        return;
    }

    HostA2FactoryInfo_ApplyCompileDefaults(&s_stFactoryRam);
    HostA2FactoryInfo_ApplyRuntime();
    HostA2FactoryInfo_QueueFlashSave();
    HostA2FactoryInfo_SendResult(DF_CAN_FACTORY_INFO_DELETE_CMD, FACTORY_RESULT_OK);
}

#else /* OTA_UPGRADE_ENABLE */

#include "HostA2FactoryInfo.h"
#include "OtaCanTypes.h"

void HostA2FactoryInfo_Init(void) {}
void HostA2FactoryInfo_MainLoopTask(void) {}
void HostA2FactoryInfo_FillChrgrIdentityFields(void) {}
void HostA2_ReplyFactoryInfoReadFromCan(const can_receive_message_struct *pMsg) { (void)pMsg; }
void HostA2_QueueFactoryInfoWriteFromCan(const can_receive_message_struct *pMsg) { (void)pMsg; }
void HostA2_QueueFactoryInfoDeleteFromCan(const can_receive_message_struct *pMsg) { (void)pMsg; }

#endif /* OTA_UPGRADE_ENABLE */
