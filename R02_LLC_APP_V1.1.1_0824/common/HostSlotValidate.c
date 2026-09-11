#include "UpgradeFeatureConfig.h"
#if OTA_UPGRADE_ENABLE

#include "HostSlotValidate.h"
#include <stdlib.h>
#include <stdint.h>

#ifndef __IO
#define __IO    volatile
#endif

static uint8_t HostSlot_IsKnownGroup(uint8_t u8Group)
{
    return (u8Group == DF_HOST_SLOT_GROUP_A || u8Group == DF_HOST_SLOT_GROUP_B) ? 1U : 0U;
}

uint8_t HostSlot_GetX1FromGroup(uint8_t u8Group, uint32_t *pBase)
{
    if (pBase == NULL || HostSlot_IsKnownGroup(u8Group) == 0U)
    {
        return 1U;
    }
    *pBase = DF_HOST_APP_A1_START;
    return 0U;
}

uint8_t HostSlot_GetX2FromGroup(uint8_t u8Group, uint32_t *pBase)
{
    if (pBase == NULL || HostSlot_IsKnownGroup(u8Group) == 0U)
    {
        return 1U;
    }
    *pBase = DF_HOST_APP_A2_START;
    return 0U;
}

uint8_t HostSlot_GetX2EndFromGroup(uint8_t u8Group, uint32_t *pEnd)
{
    if (pEnd == NULL || HostSlot_IsKnownGroup(u8Group) == 0U)
    {
        return 1U;
    }
    *pEnd = DF_HOST_APP_A2_END;
    return 0U;
}

uint8_t HostSlot_GetInactiveGroup(uint8_t u8Active, uint8_t *pInactive)
{
    if (pInactive == NULL)
    {
        return 1U;
    }
    *pInactive = HostFlash_InactiveGroup(u8Active);
    if (*pInactive != DF_HOST_SLOT_GROUP_A && *pInactive != DF_HOST_SLOT_GROUP_B)
    {
        return 1U;
    }
    return 0U;
}

uint8_t HostSlot_VectorValid(uint32_t u32SegStart, uint32_t u32SegSize, uint32_t u32ImageAddr)
{
    uint32_t u32Sp;
    uint32_t u32Reset;
    uint32_t u32ResetAddr;

    u32Sp = *(__IO uint32_t *)u32ImageAddr;
    u32Reset = *(__IO uint32_t *)(u32ImageAddr + 4U);
    u32ResetAddr = u32Reset & ~1U;

    if ((u32Sp & 0x2FFE0000U) != 0x20000000U)
    {
        return 1U;
    }
    if (u32Reset == 0U || u32Reset == 0xFFFFFFFFU)
    {
        return 1U;
    }
    if ((u32Reset & 1U) == 0U)
    {
        return 1U;
    }
    if (u32ResetAddr < u32SegStart || u32ResetAddr >= (u32SegStart + u32SegSize))
    {
        return 1U;
    }
    return 0U;
}

uint8_t HostSlot_X1Valid(uint8_t u8Group)
{
    uint32_t u32Base;
    uint32_t u32Size;

    if (HostSlot_GetX1FromGroup(u8Group, &u32Base) != 0U)
    {
        return 1U;
    }
    u32Size = HostFlash_GetX1Size(u8Group);
    return HostSlot_VectorValid(u32Base, u32Size, u32Base);
}

uint8_t HostSlot_X2Valid(uint8_t u8Group)
{
    uint32_t u32Base;
    uint32_t u32Size;

    if (HostSlot_GetX2FromGroup(u8Group, &u32Base) != 0U)
    {
        return 1U;
    }
    u32Size = HostFlash_GetX2Size(u8Group);
    return HostSlot_VectorValid(u32Base, u32Size, u32Base);
}

uint8_t HostSlot_GroupRunnable(uint8_t u8Group)
{
    if (HostSlot_X1Valid(u8Group) != 0U)
    {
        return 1U;
    }
    return HostSlot_X2Valid(u8Group);
}

static void HostSlot_FillPick(HostSlot_ScanResult *pResult, uint8_t u8Group)
{
    (void)HostSlot_GetX1FromGroup(u8Group, &pResult->u32PickX1);
    (void)HostSlot_GetX2FromGroup(u8Group, &pResult->u32PickX2);
    pResult->u8PickGroup = u8Group;
}

uint8_t HostSlot_Scan(HostSlot_ScanResult *pResult, uint8_t u8PreferGroup)
{
    if (pResult == NULL)
    {
        return DF_HOST_UPBLOCK_INVALID;
    }

    pResult->u8GroupA_X1Ok = (HostSlot_X1Valid(DF_HOST_SLOT_GROUP_A) == 0U) ? 1U : 0U;
    pResult->u8GroupA_X2Ok = (HostSlot_X2Valid(DF_HOST_SLOT_GROUP_A) == 0U) ? 1U : 0U;
    pResult->u8GroupB_X1Ok = (HostSlot_X1Valid(DF_HOST_SLOT_GROUP_B) == 0U) ? 1U : 0U;
    pResult->u8GroupB_X2Ok = (HostSlot_X2Valid(DF_HOST_SLOT_GROUP_B) == 0U) ? 1U : 0U;
    pResult->u8PickGroup = DF_HOST_UPBLOCK_INVALID;
    pResult->u32PickX1 = 0U;
    pResult->u32PickX2 = 0U;

    if (u8PreferGroup == DF_HOST_SLOT_GROUP_A
        && pResult->u8GroupA_X1Ok != 0U && pResult->u8GroupA_X2Ok != 0U)
    {
        HostSlot_FillPick(pResult, DF_HOST_SLOT_GROUP_A);
    }
    else if (u8PreferGroup == DF_HOST_SLOT_GROUP_B
             && pResult->u8GroupB_X1Ok != 0U && pResult->u8GroupB_X2Ok != 0U)
    {
        HostSlot_FillPick(pResult, DF_HOST_SLOT_GROUP_B);
    }
    else if (pResult->u8GroupA_X1Ok != 0U && pResult->u8GroupA_X2Ok != 0U)
    {
        HostSlot_FillPick(pResult, DF_HOST_SLOT_GROUP_A);
    }
    else if (pResult->u8GroupB_X1Ok != 0U && pResult->u8GroupB_X2Ok != 0U)
    {
        HostSlot_FillPick(pResult, DF_HOST_SLOT_GROUP_B);
    }

    return pResult->u8PickGroup;
}

#else /* OTA_UPGRADE_ENABLE */

#include "HostSlotValidate.h"

uint8_t HostSlot_GetX1FromGroup(uint8_t u8Group, uint32_t *pBase)
{
    (void)u8Group;
    (void)pBase;
    return 1U;
}
uint8_t HostSlot_GetX2FromGroup(uint8_t u8Group, uint32_t *pBase)
{
    (void)u8Group;
    (void)pBase;
    return 1U;
}
uint8_t HostSlot_GetX2EndFromGroup(uint8_t u8Group, uint32_t *pEnd)
{
    (void)u8Group;
    (void)pEnd;
    return 1U;
}
uint8_t HostSlot_GetInactiveGroup(uint8_t u8Active, uint8_t *pInactive)
{
    (void)u8Active;
    (void)pInactive;
    return 1U;
}
uint8_t HostSlot_VectorValid(uint32_t u32SegStart, uint32_t u32SegSize, uint32_t u32ImageAddr)
{
    (void)u32SegStart;
    (void)u32SegSize;
    (void)u32ImageAddr;
    return 1U;
}
uint8_t HostSlot_X1Valid(uint8_t u8Group) { (void)u8Group; return 1U; }
uint8_t HostSlot_X2Valid(uint8_t u8Group) { (void)u8Group; return 1U; }
uint8_t HostSlot_GroupRunnable(uint8_t u8Group) { (void)u8Group; return 1U; }
uint8_t HostSlot_Scan(HostSlot_ScanResult *pResult, uint8_t u8PreferGroup)
{
    (void)pResult;
    (void)u8PreferGroup;
    return 0U;
}

#endif /* OTA_UPGRADE_ENABLE */
