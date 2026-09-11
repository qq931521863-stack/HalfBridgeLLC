/******************************************************************************
    OtaHandoffDevType.c — 与 APPA1/Bootloader OtaStateSector 同格式，供 A2 复位前落盘
******************************************************************************/
#include "UpgradeFeatureConfig.h"
#if OTA_UPGRADE_ENABLE

#include "OtaHandoffDevType.h"
#include "crc16.h"
#include "mx25L1606E.h"
#include <string.h>

#define OTA_STATE_SECTOR_ADDR   ((uint32_t)0x00205000U)
#define OTA_STATE_MAGIC         ((uint32_t)0x31544F4FU)
#define OTA_PHASE_BOOT_ENTER    ((uint8_t)0x01U)

#pragma pack(1)
typedef struct
{
    uint32_t magic;
    uint8_t  ota_phase;
    uint8_t  dev_type;
    uint8_t  flags;
    uint8_t  reserved0;
    uint32_t session_id;
    uint32_t file_len;
    uint32_t file_crc32;
    uint32_t recv_len;
    uint16_t next_pkt_num;
    uint16_t reserved1;
    uint32_t state_crc32;
} OtaHandoffRecord_t;
#pragma pack()

static uint32_t OtaHandoffDevType_CalcCrc(const OtaHandoffRecord_t *pRec)
{
    uint32_t u32Crc = 0xFFFFFFFFU;

    CalcCRCStandard((uint8_t *)pRec,
                    (uint32_t)(sizeof(OtaHandoffRecord_t) - sizeof(uint32_t)),
                    &u32Crc);
    return u32Crc;
}

void OtaHandoffDevType_Write(uint8_t u8DevType)
{
    OtaHandoffRecord_t stRec;

    memset(&stRec, 0, sizeof(stRec));
    stRec.magic = OTA_STATE_MAGIC;
    stRec.ota_phase = OTA_PHASE_BOOT_ENTER;
    stRec.dev_type = u8DevType;
    stRec.next_pkt_num = 1U;
    stRec.state_crc32 = OtaHandoffDevType_CalcCrc(&stRec);

    FlashSector_Erase(OTA_STATE_SECTOR_ADDR);
    SPI_FLASH_BufferWrite((uint8_t *)&stRec, OTA_STATE_SECTOR_ADDR, (uint16_t)sizeof(stRec));
}

#else /* OTA_UPGRADE_ENABLE */

#include "OtaHandoffDevType.h"

void OtaHandoffDevType_Write(uint8_t u8DevType) { (void)u8DevType; }

#endif /* OTA_UPGRADE_ENABLE */
