/******************************************************************************
    HostBootState.h — 主机四槽 IAP Boot/OTA 状态枚举与元数据扩展（冻结 v1.0）
    与 主机/docs/HostFourSlotIAP_FrozenSpec_v1.0.md §6 一致。
    P1 实施时合并进 UpgradeMsgSaveFunctionDf.h / Boot 状态机。
******************************************************************************/
#ifndef _HostBootState_H_
#define _HostBootState_H_

#include "stdint.h"
#include "HostFlashMemoryMap.h"

/* ======================== Boot 主步骤（UpgradeStateJumpPro） ======================== */

#define DF_HOST_BOOT_STEP_INIT              0U
#define DF_HOST_BOOT_STEP_READ_META         1U
#define DF_HOST_BOOT_STEP_OTA_DECIDE        2U
#define DF_HOST_BOOT_STEP_IAP_WRITE         3U
#define DF_HOST_BOOT_STEP_CHAIN_JUMP        4U
#define DF_HOST_BOOT_STEP_OTA_SERVICE       5U
#define DF_HOST_BOOT_STEP_RECOVERY          9U

/* ======================== 升级阶段（u8UpgradePhase） ======================== */

#define DF_HOST_UPGRADE_PHASE_IDLE          0U
#define DF_HOST_UPGRADE_PHASE_CAN_RX        1U
#define DF_HOST_UPGRADE_PHASE_SLAVE_RS485   2U
#define DF_HOST_UPGRADE_PHASE_HOST_STAGING  3U
#define DF_HOST_UPGRADE_PHASE_HOST_IAP      4U
#define DF_HOST_UPGRADE_PHASE_DONE          5U
#define DF_HOST_UPGRADE_PHASE_FAIL          6U

/* ======================== 四段有效标志 ======================== */

#define DF_HOST_IMAGE_INVALID               ((uint8_t)0x55U)
#define DF_HOST_IMAGE_VALID                 ((uint8_t)0xAAU)
#define DF_HOST_IMAGE_IN_PROGRESS           ((uint8_t)0x01U)   /* 写片中，掉电视为无效 */

/* ======================== x1→x2 门禁结果 ======================== */

#define DF_HOST_X2_GATE_OK                  0U
#define DF_HOST_X2_GATE_VECTOR_FAIL         1U
#define DF_HOST_X2_GATE_CRC_FAIL            2U
#define DF_HOST_X2_GATE_LEN_FAIL            3U
#define DF_HOST_X2_GATE_NOT_VALID           4U

/* ======================== 回退原因（日志/上位机诊断） ======================== */

#define DF_HOST_ROLLBACK_NONE               0U
#define DF_HOST_ROLLBACK_ACTIVE_X1_FAIL     1U
#define DF_HOST_ROLLBACK_ACTIVE_X2_FAIL     2U
#define DF_HOST_ROLLBACK_IAP_VERIFY_FAIL      3U
#define DF_HOST_ROLLBACK_BOTH_GROUPS_BAD    4U

/* ======================== Boot 写片内 IAP 返回值（扩展 UserProgramUpdate） ======================== */

#define DF_HOST_IAP_OK                      0U
#define DF_HOST_IAP_META_ERR                1U
#define DF_HOST_IAP_EXT_CRC_ERR             2U
#define DF_HOST_IAP_ERASE_ERR               3U
#define DF_HOST_IAP_WRITE_ERR               4U
#define DF_HOST_IAP_X1_VECTOR_ERR           5U
#define DF_HOST_IAP_X2_VECTOR_ERR           6U
#define DF_HOST_IAP_VERIFY_ERR              7U

/* ======================== 启动模式（Boot 读元数据后） ======================== */

typedef enum
{
    HOST_BOOT_MODE_NORMAL_CHAIN = 0,   /* Boot → 活动 x1 → x2 */
    HOST_BOOT_MODE_OTA_SERVICE  = 1,   /* Boot → 活动 x1，不跳 x2 */
    HOST_BOOT_MODE_IAP_WRITE    = 2,   /* 外扩 → 片内非活动组 */
    HOST_BOOT_MODE_RECOVERY     = 3    /* 元数据损坏，强制 A1 OTA */
} HostBootMode_t;

/* ======================== 扩展元数据（P1 并入 _StProgStatusMsg 或并列结构） ======================== */

#pragma pack(1)
typedef struct
{
    uint8_t  u8ImageA1Valid;
    uint8_t  u8ImageA2Valid;
    uint8_t  u8ImageB1Valid;
    uint8_t  u8ImageB2Valid;

    uint32_t u32ImageA1Len;
    uint32_t u32ImageA2Len;
    uint32_t u32ImageB1Len;
    uint32_t u32ImageB2Len;

    uint32_t u32ImageA1Crc32;
    uint32_t u32ImageA2Crc32;
    uint32_t u32ImageB1Crc32;
    uint32_t u32ImageB2Crc32;

    uint8_t  u8UpgradePhase;
    uint8_t  u8PendingBootGroup;       /* DF_HOST_SLOT_GROUP_A / B */
    uint8_t  u8BootConfirmState;       /* 与从机 DF_BOOT_CONFIRM_STATE_* 语义一致 */
    uint8_t  u8LastRollbackReason;

    uint8_t  u8OtaServiceFlag;         /* 1=复位后仅进 x1 */
    uint8_t  u8Res[3];
} _StHostFourSlotExtMsg;
#pragma pack()

/* 双备份：Primary @ DF_SYS_STATE_MSG_SAVE_* ，Mirror @ DF_SYS_STATE_MSG_MIRROR_* */

#endif /* _HostBootState_H_ */
