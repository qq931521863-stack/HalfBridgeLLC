/******************************************************************************
    OtaHandoffDevType.h — A2 handoff 前将 B0 DevType 写入外扩 Flash OTA 状态区
******************************************************************************/
#ifndef _OtaHandoffDevType_H
#define _OtaHandoffDevType_H

#include <stdint.h>

void OtaHandoffDevType_Write(uint8_t u8DevType);

#endif
