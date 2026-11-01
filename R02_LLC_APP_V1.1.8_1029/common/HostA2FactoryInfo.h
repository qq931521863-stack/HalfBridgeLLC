/******************************************************************************
    HostA2FactoryInfo.h — APPA2 产测出厂信息 C3/C4/C5（Flash 擦写在 main while 执行）
******************************************************************************/
#ifndef _HostA2FactoryInfo_H_
#define _HostA2FactoryInfo_H_

#include "OtaCanTypes.h"

void HostA2FactoryInfo_Init(void);
void HostA2FactoryInfo_MainLoopTask(void);

void HostA2_ReplyFactoryInfoReadFromCan(const can_receive_message_struct *pMsg);
void HostA2_QueueFactoryInfoWriteFromCan(const can_receive_message_struct *pMsg);
void HostA2_QueueFactoryInfoDeleteFromCan(const can_receive_message_struct *pMsg);

void HostA2FactoryInfo_FillChrgrIdentityFields(void);

#endif /* _HostA2FactoryInfo_H_ */
