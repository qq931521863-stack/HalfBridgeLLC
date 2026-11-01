/******************************************************************************
    HostA2Inquire.h — APPA2 运行态应答 C0/C1/C2（不 handoff 至 A1）
******************************************************************************/
#ifndef _HostA2Inquire_H_
#define _HostA2Inquire_H_

#include "OtaCanTypes.h"

void HostA2Inquire_Init(void);
void HostA2Inquire_SyncIdentityFromFactory(const char app_rev[3], const char hw_rev[3], const char boot_rev[3]);
void HostA2_ReplyInquireDevMsgFromCan(const can_receive_message_struct *pMsg);
void HostA2_ReplyInquireSlaveDevMsgFromCan(const can_receive_message_struct *pMsg);
void HostA2_ReplyInquireHostDevExtFromCan(const can_receive_message_struct *pMsg);

#endif
