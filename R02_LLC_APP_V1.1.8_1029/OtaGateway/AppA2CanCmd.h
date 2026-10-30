#ifndef _AppA2CanCmd_H_
#define _AppA2CanCmd_H_

#include "stdint.h"

#define DF_CAN_INTO_UPGRADE_MODE_CMD        0xB0U
#define DF_CAN_START_UPGRADE_CMD            0xB1U
#define DF_CAN_FILE_TRANSFER_REQUEST_CMD    0xB2U
#define DF_CAN_FILE_TRANSFER_CMD            0xB3U
#define DF_CAN_FILE_TRANSFER_END_CMD        0xB4U
#define DF_CAN_UPGRADE_STATUS_INQURE_CMD    0xB5U
#define DF_CAN_INQURE_DEV_MSG_CMD           0xC0U
#define DF_CAN_INQURE_SLAVE_DEV_MSG_CMD     0xC1U
#define DF_CAN_INQURE_HOST_DEV_EXT_CMD      0xC2U
#define DF_CAN_FACTORY_INFO_READ_CMD        0xC3U
#define DF_CAN_FACTORY_INFO_WRITE_CMD       0xC4U
#define DF_CAN_FACTORY_INFO_DELETE_CMD      0xC5U
#define DF_INQURE_DEV_MSG_CMD_TYPE          0x03U
#define DF_CAN_MASTER_ADDR                  0x20U
#define DF_CAN_PC_SFOT_ADDR                 0xA0U
#define DF_UPGRADE_CMD_TYPE                 0x05U
#define DF_CAN1_CHANN                       0x01U

#define DF_CAN_INTO_UPGRADE_MODE_ACK_SUCC   0x00U
#define DF_CAN_INTO_UPGRADE_MODE_ACK_FAIL   0x01U

#pragma pack(1)
typedef struct
{
    uint32_t SrcAddr : 8;
    uint32_t DstAddr : 8;
    uint32_t CmdCode : 8;
    uint32_t CmdType : 5;
    uint32_t none    : 3;
} _StCanIDMsg;
#pragma pack()

typedef union
{
    _StCanIDMsg StCanIDMsgBit;
    uint32_t all;
} _UStCanIDMsg;

#endif /* _AppA2CanCmd_H_ */
