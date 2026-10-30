/******************************************************************************
    HostFlashMemoryMap.h — 与 CAN1FD升级/common/HostFlashMemoryMap.h 同步（v4.0 4KB 单 Bank）
******************************************************************************/
#ifndef _HostFlashMemoryMap_H_
#define _HostFlashMemoryMap_H_

#include "stdint.h"

#define DF_INTERNAL_FLASH_TOTAL_BYTES       ((uint32_t)0x00040000U)
#define DF_INTERNAL_FLASH_PAGE_SIZE         ((uint32_t)0x1000U)
#define DF_INTERNAL_FLASH_PAGE_SIZE_2K      ((uint32_t)0x800U)
#define DF_INTERNAL_FLASH_TOTAL_PAGES       64U
#define DF_FLASH_END_ADDRESS                ((uint32_t)0x08040000U)
#define DF_HOST_FLASH_SINGLE_BANK           1U
#define DF_HOST_HW_BANK2_BASE               DF_FLASH_END_ADDRESS
#define DF_HOST_INTERNAL_BANK_SPLIT         DF_FLASH_END_ADDRESS

#define DF_SYS_BOOT_LOADER_START_ADDR       ((uint32_t)0x08000000U)
#define DF_SYS_BOOT_LOADER_END_ADDR         ((uint32_t)0x08009000U)
#define DF_SYS_BOOT_LOADER_SIZE_BYTES       ((uint32_t)0x00009000U)
#define DF_SYS_BOOT_LOADER_PAGE_NUM         9U

#define DF_SYS_STATE_MSG_SAVE_START_ADDR    ((uint32_t)0x08009000U)
#define DF_SYS_STATE_MSG_SAVE_END_ADDR      ((uint32_t)0x0800A000U)
#define DF_SYS_STATE_MSG_SAVE_SIZE_BYTES    DF_INTERNAL_FLASH_PAGE_SIZE
#define DF_SYS_STATE_MSG_SAVE_PAGE_NUM      1U

#define DF_HOST_APP_A1_START                ((uint32_t)0x0800A000U)
#define DF_HOST_APP_A1_SIZE                 ((uint32_t)0x00015800U)
#define DF_HOST_APP_A1_END                  ((uint32_t)0x08022000U)
#define DF_HOST_APP_A1_PAGE_NUM             22U
#define DF_HOST_APP_A1_ERASE_FIRST_PAGE     10U

#define DF_HOST_APP_A2_START                ((uint32_t)0x08022000U)
#define DF_HOST_APP_A2_SIZE                 ((uint32_t)0x00019000U)
#define DF_HOST_APP_A2_END                  (DF_HOST_APP_A2_START + DF_HOST_APP_A2_SIZE)
#define DF_HOST_APP_A2_PAGE_NUM             25U
#define DF_HOST_APP_A2_ERASE_FIRST_PAGE     32U

#define DF_HOST_INTERNAL_RESERVE_START      DF_HOST_APP_A2_END
#define DF_HOST_INTERNAL_RESERVE_END        DF_FLASH_END_ADDRESS
#define DF_HOST_INTERNAL_RESERVE_SIZE       (DF_HOST_INTERNAL_RESERVE_END - DF_HOST_INTERNAL_RESERVE_START)

#define DF_SYS_STATE_MSG_MIRROR_START_ADDR  DF_HOST_INTERNAL_RESERVE_START
#define DF_SYS_STATE_MSG_MIRROR_END_ADDR    (DF_SYS_STATE_MSG_MIRROR_START_ADDR + DF_INTERNAL_FLASH_PAGE_SIZE)

#define DF_HOST_IAP_APP_REGION_START        DF_HOST_APP_A1_START
#define DF_HOST_IAP_APP_REGION_END          DF_HOST_APP_A2_END

#define DF_HOST_APP_A1_MAX_BIN_BYTES        DF_HOST_APP_A1_SIZE
#define DF_HOST_APP_A2_MAX_BIN_BYTES        DF_HOST_APP_A2_SIZE
#define DF_HOST_APP_MIN_BIN_LENGTH_BYTES    ((uint32_t)10240U)

#define DF_HOST_APP_A1_VTOR                 DF_HOST_APP_A1_START
#define DF_HOST_APP_A2_VTOR                 DF_HOST_APP_A2_START

#define DF_HOST_EXT_BANK_A                  ((uint8_t)0x55U)
#define DF_HOST_EXT_BANK_B                  ((uint8_t)0xAAU)
#define DF_HOST_SLOT_GROUP_A                DF_HOST_EXT_BANK_A
#define DF_HOST_SLOT_GROUP_B                DF_HOST_EXT_BANK_B
#define DF_HOST_UPBLOCK_INVALID             ((uint8_t)0xFFU)

#define DF_HOST_IMAGE_ID_A1                 0U
#define DF_HOST_IMAGE_ID_A2                 1U

#define DF_HOST_EXT_BANK_SIZE               ((uint32_t)0x00064000U)
#define DF_HOST_EXT_BANK_A_BASE             ((uint32_t)0x00000000U)
#define DF_HOST_EXT_BANK_B_BASE             (DF_HOST_EXT_BANK_A_BASE + DF_HOST_EXT_BANK_SIZE)
#define DF_HOST_EXT_FLASH_64K_BYTES         ((uint32_t)0x10000U)
#define DF_HOST_EXT_BANK_A_64K_START        ((uint16_t)(DF_HOST_EXT_BANK_A_BASE / DF_HOST_EXT_FLASH_64K_BYTES))
#define DF_HOST_EXT_BANK_B_64K_START        ((uint16_t)(DF_HOST_EXT_BANK_B_BASE / DF_HOST_EXT_FLASH_64K_BYTES))
#define DF_HOST_EXT_BANK_64K_ERASE_BLOCKS   ((uint16_t)((DF_HOST_EXT_BANK_SIZE + DF_HOST_EXT_FLASH_64K_BYTES - 1U) / DF_HOST_EXT_FLASH_64K_BYTES))
#define DF_HOST_EXT_BUNDLE_A1_OFF           0U
#define DF_HOST_EXT_BUNDLE_A2_OFF           DF_HOST_APP_A1_SIZE
#define DF_HOST_EXT_BUNDLE_BYTES            (DF_HOST_APP_A1_SIZE + DF_HOST_APP_A2_SIZE)

#define DF_MASTET_A_FIRMWARE_START_ADDRESS  DF_HOST_EXT_BANK_A_BASE
#define DF_MASTET_B_FIRMWARE_START_ADDRESS  DF_HOST_EXT_BANK_B_BASE
#define DF_MASTER_FIRMWARE_BANK_SIZE        DF_HOST_EXT_BANK_SIZE
#define DF_MASTER_EXT_A1_OFFSET             DF_HOST_EXT_BUNDLE_A1_OFF
#define DF_MASTER_EXT_A2_OFFSET             DF_HOST_EXT_BUNDLE_A2_OFF

#define DF_SLAVE1_FIRMWARE_START_ADDRESS    ((uint32_t)0x00100000U)
#define DF_SLAVE2_FIRMWARE_START_ADDRESS    ((uint32_t)0x00180000U)

#define DF_MASTET_FIRMWARE_MSG_START_ADDRESS ((uint32_t)0x00202000U)
#define DF_SLAVE1_FIRMWARE_MSG_START_ADDRESS ((uint32_t)0x00203000U)
#define DF_SLAVE2_FIRMWARE_MSG_START_ADDRESS ((uint32_t)0x00204000U)
#define DF_HOST_STAGING_META_EXT_ADDR       ((uint32_t)0x00206000U)
#define DF_HOST_FACTORY_INFO_EXT_ADDR       ((uint32_t)0x00207000U)
#define DF_HOST_FACTORY_INFO_SIZE           ((uint32_t)0x1000U)

#define DF_EXT_FLASH_PACK_STORE_BASE        ((uint32_t)0x00300000U)
#define DF_EXT_FLASH_PACK_STORE_SIZE        ((uint32_t)0x00200000U)
#define DF_EXT_FLASH_PACK_STORE_TAIL_INFO_RESERVE  (8U * 1024U)

#define DF_SLAVE_INTERNAL_APP_A_START_ADDR  ((uint32_t)0x08008000U)
#define DF_SLAVE_INTERNAL_APP_A_END_ADDR    ((uint32_t)0x08013000U)
#define DF_SLAVE_INTERNAL_APP_B_START_ADDR  ((uint32_t)0x08013000U)
#define DF_SLAVE_INTERNAL_APP_B_END_ADDR    ((uint32_t)0x0801E000U)
#define DF_SLAVE_APP_A_LINK_BASE            DF_SLAVE_INTERNAL_APP_A_START_ADDR
#define DF_SLAVE_APP_B_LINK_BASE            DF_SLAVE_INTERNAL_APP_B_START_ADDR
#define DF_SLAVE_APP_SLOT_SIZE              ((uint32_t)0x0000B000U)

static inline uint32_t HostFlash_GetExtBankBase(uint8_t u8Bank)
{
    return (u8Bank == DF_HOST_EXT_BANK_B) ? DF_HOST_EXT_BANK_B_BASE : DF_HOST_EXT_BANK_A_BASE;
}

static inline uint8_t HostFlash_InactiveExtBank(uint8_t u8ActiveBank)
{
    return (u8ActiveBank == DF_HOST_EXT_BANK_A) ? DF_HOST_EXT_BANK_B : DF_HOST_EXT_BANK_A;
}

static inline uint32_t HostFlash_GetX1Start(uint8_t u8Group)
{
    (void)u8Group;
    return DF_HOST_APP_A1_START;
}

static inline uint32_t HostFlash_GetX2Start(uint8_t u8Group)
{
    (void)u8Group;
    return DF_HOST_APP_A2_START;
}

static inline uint32_t HostFlash_GetX1Size(uint8_t u8Group)
{
    (void)u8Group;
    return DF_HOST_APP_A1_SIZE;
}

static inline uint32_t HostFlash_GetX2Size(uint8_t u8Group)
{
    (void)u8Group;
    return DF_HOST_APP_A2_SIZE;
}

static inline uint8_t HostFlash_InactiveGroup(uint8_t u8ActiveGroup)
{
    return HostFlash_InactiveExtBank(u8ActiveGroup);
}

#endif /* _HostFlashMemoryMap_H_ */
