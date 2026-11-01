/******************************************************************************
    HostSlotValidate.h — 主机四槽 x1/x2 向量校验与组切换
******************************************************************************/
#ifndef _HostSlotValidate_H_
#define _HostSlotValidate_H_

#include "stdint.h"
#include "HostFlashMemoryMap.h"

#define DF_HOST_UPBLOCK_INVALID             ((uint8_t)0x00U)

typedef struct
{
    uint8_t  u8GroupA_X1Ok;
    uint8_t  u8GroupA_X2Ok;
    uint8_t  u8GroupB_X1Ok;
    uint8_t  u8GroupB_X2Ok;
    uint8_t  u8PickGroup;
    uint32_t u32PickX1;
    uint32_t u32PickX2;
} HostSlot_ScanResult;

uint8_t HostSlot_GetX1FromGroup(uint8_t u8Group, uint32_t *pBase);
uint8_t HostSlot_GetX2FromGroup(uint8_t u8Group, uint32_t *pBase);
uint8_t HostSlot_GetX2EndFromGroup(uint8_t u8Group, uint32_t *pEnd);
uint8_t HostSlot_GetInactiveGroup(uint8_t u8Active, uint8_t *pInactive);

uint8_t HostSlot_VectorValid(uint32_t u32SegStart, uint32_t u32SegSize, uint32_t u32ImageAddr);
uint8_t HostSlot_X1Valid(uint8_t u8Group);
uint8_t HostSlot_X2Valid(uint8_t u8Group);
uint8_t HostSlot_GroupRunnable(uint8_t u8Group);

uint8_t HostSlot_Scan(HostSlot_ScanResult *pResult, uint8_t u8PreferGroup);

#endif
