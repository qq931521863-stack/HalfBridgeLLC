#ifndef _HostX1Gate_H_
#define _HostX1Gate_H_

#include "stdint.h"
#include "OtaCanTypes.h"

#define HOST_X1GATE_OK              0U
#define HOST_X1GATE_X1_INVALID      1U

void HostX1Gate_PlatformShutdown(void);
uint8_t HostX1Gate_Check(void);
void HostX1Gate_JumpToX1(void);
uint8_t HostA2_SetOtaServiceFlag(void);
uint8_t HostA2_ShouldHandoffToA1(uint8_t u8CmdCode);
void HostA2_EnterOtaService(void);
void HostA2_ReplyUpgradeStatusFromCan(can_receive_message_struct *can_receive_message, uint8_t u8CANChannel);
void HostA2_ReplyIntoUpgradeMode(can_receive_message_struct *can_receive_message, uint8_t u8Result);
void HostA2_CanSendExtMsg(uint8_t u8CmdType, uint8_t u8CmdCode, const uint8_t *pu8Data, uint32_t u32Dlc);

#endif /* _HostX1Gate_H_ */
