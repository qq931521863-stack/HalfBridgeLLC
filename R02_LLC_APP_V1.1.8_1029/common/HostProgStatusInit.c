#include "UpgradeFeatureConfig.h"
#if OTA_UPGRADE_ENABLE

#include "HostProgStatusInit.h"
#include "UpgradeMsgSaveFunctionDf.h"
#include "HostSlotValidate.h"
#include "HostBootState.h"
#include <string.h>

void HostProgStatus_ApplyFactoryDefaults(uint8_t *pu8Buf)
{
    _StProgStatus *pStatus = (_StProgStatus *)pu8Buf;

    if (pStatus == NULL)
    {
        return;
    }

    memset(pStatus->u8Buf, 0, sizeof(_StProgStatus));

    pStatus->StProgStatusMsg.u8FlashUserBlock = DF_HOST_SLOT_GROUP_A;
    pStatus->StProgStatusMsg.u8FlashUPtoBlock = 0xFFU;
    pStatus->StProgStatusMsg.u16UpgradeState = DF_NOT_UPGRADE_STATUS;
    pStatus->StProgStatusMsg.u8BlockABinFileValidFlag = DF_UPGRADE_FILE_INVAILE_STATUS;
    pStatus->StProgStatusMsg.u8BlockBBinFileValidFlag = DF_UPGRADE_FILE_INVAILE_STATUS;
    pStatus->StProgStatusMsg.u8UpgradePhase = DF_HOST_UPGRADE_PHASE_IDLE;
    pStatus->StProgStatusMsg.u8OtaServiceFlag = 0U;

    if (HostSlot_X1Valid(DF_HOST_SLOT_GROUP_A) == 0U)
    {
        pStatus->StProgStatusMsg.u8ImageA1Valid = DF_HOST_IMAGE_VALID;
    }
    if (HostSlot_X2Valid(DF_HOST_SLOT_GROUP_A) == 0U)
    {
        pStatus->StProgStatusMsg.u8ImageA2Valid = DF_HOST_IMAGE_VALID;
    }
    if (HostSlot_X1Valid(DF_HOST_SLOT_GROUP_B) == 0U)
    {
        pStatus->StProgStatusMsg.u8ImageB1Valid = DF_HOST_IMAGE_VALID;
    }
    if (HostSlot_X2Valid(DF_HOST_SLOT_GROUP_B) == 0U)
    {
        pStatus->StProgStatusMsg.u8ImageB2Valid = DF_HOST_IMAGE_VALID;
    }
}

uint8_t HostProgStatus_SanitizeAfterRead(uint8_t *pu8Buf)
{
    _StProgStatus *pStatus = (_StProgStatus *)pu8Buf;
    uint8_t u8Changed = 0U;

    if (pStatus == NULL)
    {
        return 0U;
    }

    if (pStatus->StProgStatusMsg.u8OtaServiceFlag != 0U
        && pStatus->StProgStatusMsg.u8OtaServiceFlag != 1U)
    {
        pStatus->StProgStatusMsg.u8OtaServiceFlag = 0U;
        u8Changed = 1U;
    }

    if (pStatus->StProgStatusMsg.u8FlashUserBlock != DF_HOST_SLOT_GROUP_A
        && pStatus->StProgStatusMsg.u8FlashUserBlock != DF_HOST_SLOT_GROUP_B)
    {
        pStatus->StProgStatusMsg.u8FlashUserBlock = DF_HOST_SLOT_GROUP_A;
    }

    if (pStatus->StProgStatusMsg.u8FlashUPtoBlock != DF_HOST_SLOT_GROUP_A
        && pStatus->StProgStatusMsg.u8FlashUPtoBlock != DF_HOST_SLOT_GROUP_B
        && pStatus->StProgStatusMsg.u8FlashUPtoBlock != 0xFFU)
    {
        pStatus->StProgStatusMsg.u8FlashUPtoBlock = 0xFFU;
        u8Changed = 1U;
    }

    return u8Changed;
}

#else /* OTA_UPGRADE_ENABLE */

#include "HostProgStatusInit.h"

void HostProgStatus_ApplyFactoryDefaults(uint8_t *pu8Buf) { (void)pu8Buf; }
uint8_t HostProgStatus_SanitizeAfterRead(uint8_t *pu8Buf) { (void)pu8Buf; return 0U; }

#endif /* OTA_UPGRADE_ENABLE */
