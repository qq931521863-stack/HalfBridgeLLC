#ifndef _LlcOta_H_
#define _LlcOta_H_

#include "stdint.h"

/* 1：LLC 自身处于充电运行态时，B0 立即 Fail（仅入口门禁）
 * 0：不判断 LLC 运行态，允许进入 Handoff */
#ifndef LLC_OTA_RUN_STATE_GATE_EN
#define LLC_OTA_RUN_STATE_GATE_EN   0U
#endif

/* 1：Handoff 前等待 PFC 进入安全态（需 UartCom + PFC 状态回读）
 * 0：强制升级，不等待 PFC 状态；StartPfcSafeRequest 仍发 1 次 0x80（策略 A） */
#ifndef LLC_OTA_PFC_SAFE_WAIT_EN
#define LLC_OTA_PFC_SAFE_WAIT_EN    0U
#endif

#define LLC_MODBUS_OTA_SAFE_REQ   0x80U
#define PFC_OTA_SAFE_WAIT_MS      500U

uint8_t LlcOta_IsRunStateBlockingUpgrade(void);
void LlcOta_PreHandoffShutdown(void);
void LlcOta_StartPfcSafeRequest(void);
/* 0=等待 1=已安全 2=超时失败 */
uint8_t LlcOta_PollPfcSafeRequest(void);
uint8_t LlcOta_GetPfcSafeReqFlag(void);

#endif /* _LlcOta_H_ */
