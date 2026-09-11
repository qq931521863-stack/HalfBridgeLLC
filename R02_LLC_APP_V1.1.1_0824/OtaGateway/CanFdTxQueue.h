#ifndef _CanFdTxQueue_H_
#define _CanFdTxQueue_H_

#include "stm32g4xx_hal.h"

#define CANFD_TX_PRIO_OTA       0U
#define CANFD_TX_PRIO_CHARGER     1U

#define CANFD_TX_DRAIN_MAX_PER_MS 3U

void CanFdTxQueue_Init(void);
uint8_t CanFdTxQueue_Enqueue(uint8_t u8Priority,
                             const FDCAN_TxHeaderTypeDef *pHeader,
                             const uint8_t *pu8Data);
void CanFdTxQueue_Drain(FDCAN_HandleTypeDef *phfdcan);
uint8_t CanFdTxQueue_GetPendingCount(void);
uint8_t CanFdTx_IsFullyDrained(FDCAN_HandleTypeDef *phfdcan);
void CanFd_SysTickCanService(void);

#endif /* _CanFdTxQueue_H_ */
