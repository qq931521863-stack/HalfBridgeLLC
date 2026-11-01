/******************************************************************************
    HostOtaHandoff.c
******************************************************************************/
#include "UpgradeFeatureConfig.h"
#if OTA_UPGRADE_ENABLE

#include "HostOtaHandoff.h"

void HostOtaHandoff_MarkA2ToA1(void)
{
    *HOST_OTA_HANDOFF_RAM_ADDR = HOST_OTA_A2_TO_A1_MAGIC;
}

uint8_t HostOtaHandoff_ConsumeA2ToA1(void)
{
    if (*HOST_OTA_HANDOFF_RAM_ADDR == HOST_OTA_A2_TO_A1_MAGIC)
    {
        *HOST_OTA_HANDOFF_RAM_ADDR = 0U;
        return 1U;
    }
    return 0U;
}

#else /* OTA_UPGRADE_ENABLE */

#include "HostOtaHandoff.h"

void HostOtaHandoff_MarkA2ToA1(void) {}
uint8_t HostOtaHandoff_ConsumeA2ToA1(void) { return 0U; }

#endif /* OTA_UPGRADE_ENABLE */
