#ifndef _OtaGw_H_
#define _OtaGw_H_

#include "stm32g4xx_hal.h"
typedef enum
{
    OTA_HANDOFF_IDLE = 0,
    OTA_HANDOFF_WAIT_PFC,
    OTA_HANDOFF_WAIT_CAN_TX,
    OTA_HANDOFF_DO_JUMP
} OtaHandoffState_t;

void OtaGw_Init(void);
void OtaGw_RxIsrHook(FDCAN_RxHeaderTypeDef *pHeader, const uint8_t *pu8Data);
void OtaGw_Poll(void);
void OtaGw_HandoffTask(void);
uint8_t OtaGw_IsTrafficActive(void);
uint8_t OtaGw_IsSciWatchdogPaused(void);
uint8_t OtaGw_GetRxPayloadBytes(const FDCAN_RxHeaderTypeDef *pHeader);
extern OtaHandoffState_t s_eHandoffState;
#endif /* _OtaGw_H_ */
