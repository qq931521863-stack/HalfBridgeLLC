#include "UpgradeFeatureConfig.h"
#if OTA_UPGRADE_ENABLE

#include "LlcOta.h"
#include "HwConfig.h"
#include "operateStatus.h"
#include "MODBUS_SLAVE.h"
#include "main.h"

static volatile uint8_t s_u8PfcOtaSafeReq = 0U;
static volatile uint8_t s_u8PfcWaitActive = 0U;
static uint32_t s_u32PfcWaitStartMs = 0U;
static uint32_t s_u32PfcLastSendMs = 0U;

static void LlcOta_SendPfcSafeRequestOnce(void)
{
    s_u8PfcOtaSafeReq = 1U;
    Send_to_Pfc_info();
    s_u8PfcOtaSafeReq = 0U;
}

#if (LLC_OTA_PFC_SAFE_WAIT_EN != 0U)
static uint8_t LlcOta_IsPfcRunStateSafe(void)
{
    if (pfc_DataFlowFace.PfcState == 0U)
    {
        return 1U;
    }
    return 0U;
}
#endif

uint8_t LlcOta_IsRunStateBlockingUpgrade(void)
{
#if (LLC_OTA_RUN_STATE_GATE_EN == 0U)
    return 0U;
#else
    if (gSys_State.SysSta == Charging_state)
    {
        return 1U;
    }
    if (DataFlowFace.RunState == Charging)
    {
        return 1U;
    }
    return 0U;
#endif
}

void LlcOta_PreHandoffShutdown(void)
{
    DrvH_Off();
    DrvL_Off();
    SarH_Off();
    SarL_Off();
}

void LlcOta_StartPfcSafeRequest(void)
{
    LlcOta_SendPfcSafeRequestOnce();

#if (LLC_OTA_PFC_SAFE_WAIT_EN != 0U)
    s_u32PfcWaitStartMs = HAL_GetTick();
    s_u32PfcLastSendMs = s_u32PfcWaitStartMs;
    s_u8PfcWaitActive = 1U;
#else
    s_u8PfcWaitActive = 0U;
#endif
}

uint8_t LlcOta_PollPfcSafeRequest(void)
{
#if (LLC_OTA_PFC_SAFE_WAIT_EN == 0U)
    return 1U;
#else
    uint32_t u32Now;

    if (s_u8PfcWaitActive == 0U)
    {
        return 1U;
    }

    u32Now = HAL_GetTick();

    if (LlcOta_IsPfcRunStateSafe() != 0U)
    {
        s_u8PfcWaitActive = 0U;
        return 1U;
    }

    if ((uint32_t)(u32Now - s_u32PfcWaitStartMs) >= PFC_OTA_SAFE_WAIT_MS)
    {
        s_u8PfcWaitActive = 0U;
        return 2U;
    }

    if ((uint32_t)(u32Now - s_u32PfcLastSendMs) >= 20U)
    {
        LlcOta_SendPfcSafeRequestOnce();
        s_u32PfcLastSendMs = u32Now;
    }

    return 0U;
#endif
}

uint8_t LlcOta_GetPfcSafeReqFlag(void)
{
    return s_u8PfcOtaSafeReq;
}

#else /* OTA_UPGRADE_ENABLE */

#include "LlcOta.h"

uint8_t LlcOta_IsRunStateBlockingUpgrade(void) { return 0U; }
void LlcOta_PreHandoffShutdown(void) {}
void LlcOta_StartPfcSafeRequest(void) {}
uint8_t LlcOta_PollPfcSafeRequest(void) { return 1U; }
uint8_t LlcOta_GetPfcSafeReqFlag(void) { return 0U; }

#endif /* OTA_UPGRADE_ENABLE */
