#ifndef _HostProgStatusInit_H_
#define _HostProgStatusInit_H_

#include "stdint.h"

/* pu8Buf 指向 _StProgStatus.u8Buf，长度 44 字节 */
void HostProgStatus_ApplyFactoryDefaults(uint8_t *pu8Buf);
/* 返回 1 表示字段被修正，调用方可选择写回外扩 Flash */
uint8_t HostProgStatus_SanitizeAfterRead(uint8_t *pu8Buf);

#endif
