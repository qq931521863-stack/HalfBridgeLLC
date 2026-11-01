#include "CanFdTxQueue.h"
#include "UpgradeFeatureConfig.h"
#if OTA_UPGRADE_ENABLE
#include "OtaGw.h"
#endif
#include "CAN_Control_2800W.h"
#include "main.h"
#include <string.h>

#define CANFD_TX_OTA_QUEUE_DEPTH      6U
#define CANFD_TX_CHARGER_QUEUE_DEPTH  4U
#define CANFD_TX_HW_FIFO_FREE_FULL    3U

typedef struct
{
    FDCAN_TxHeaderTypeDef stHeader;
    uint8_t au8Data[64];
} CanFdTxSlot_t;

static CanFdTxSlot_t s_aOtaTxQueue[CANFD_TX_OTA_QUEUE_DEPTH];
static uint8_t s_u8OtaTxHead = 0U;
static uint8_t s_u8OtaTxTail = 0U;
static uint8_t s_u8OtaTxCount = 0U;

static CanFdTxSlot_t s_aChgTxQueue[CANFD_TX_CHARGER_QUEUE_DEPTH];
static uint8_t s_u8ChgTxHead = 0U;
static uint8_t s_u8ChgTxTail = 0U;
static uint8_t s_u8ChgTxCount = 0U;

static volatile uint8_t s_u8HwTxFifoEmpty = 1U;

static uint8_t CanFdTxQueue_Push(CanFdTxSlot_t *pQueue, uint8_t *pu8Head, uint8_t *pu8Tail,
                                 uint8_t *pu8Count, uint8_t u8Depth,
                                 const FDCAN_TxHeaderTypeDef *pHeader, const uint8_t *pu8Data)
{
    CanFdTxSlot_t *pSlot;

    (void)pu8Tail;

    if (pHeader == NULL || pu8Data == NULL || *pu8Count >= u8Depth)
    {
        return 0U;
    }

    pSlot = &pQueue[*pu8Head];
    pSlot->stHeader = *pHeader;
    memcpy(pSlot->au8Data, pu8Data, sizeof(pSlot->au8Data));

    *pu8Head = (uint8_t)((*pu8Head + 1U) % u8Depth);
    (*pu8Count)++;
    return 1U;
}

static const CanFdTxSlot_t *CanFdTxQueue_PeekOta(void)
{
    if (s_u8OtaTxCount == 0U)
    {
        return NULL;
    }
    return &s_aOtaTxQueue[s_u8OtaTxTail];
}

static const CanFdTxSlot_t *CanFdTxQueue_PeekCharger(void)
{
    if (s_u8ChgTxCount == 0U)
    {
        return NULL;
    }
    return &s_aChgTxQueue[s_u8ChgTxTail];
}

static void CanFdTxQueue_PopOta(void)
{
    if (s_u8OtaTxCount == 0U)
    {
        return;
    }
    s_u8OtaTxTail = (uint8_t)((s_u8OtaTxTail + 1U) % CANFD_TX_OTA_QUEUE_DEPTH);
    s_u8OtaTxCount--;
}

static void CanFdTxQueue_PopCharger(void)
{
    if (s_u8ChgTxCount == 0U)
    {
        return;
    }
    s_u8ChgTxTail = (uint8_t)((s_u8ChgTxTail + 1U) % CANFD_TX_CHARGER_QUEUE_DEPTH);
    s_u8ChgTxCount--;
}

static uint8_t CanFdTxQueue_TryHwSend(FDCAN_HandleTypeDef *phfdcan, const CanFdTxSlot_t *pSlot)
{
    if (phfdcan == NULL || pSlot == NULL)
    {
        return 0U;
    }
    if (HAL_FDCAN_GetTxFifoFreeLevel(phfdcan) == 0U)
    {
        return 0U;
    }
    if (HAL_FDCAN_AddMessageToTxFifoQ(phfdcan, &pSlot->stHeader, pSlot->au8Data) != HAL_OK)
    {
        return 0U;
    }

    s_u8HwTxFifoEmpty = 0U;
    return 1U;
}

static uint8_t CanFdTxQueue_SendOnePending(FDCAN_HandleTypeDef *phfdcan)
{
    const CanFdTxSlot_t *pSlot;

    pSlot = CanFdTxQueue_PeekOta();
    if (pSlot != NULL)
    {
        if (CanFdTxQueue_TryHwSend(phfdcan, pSlot) != 0U)
        {
            CanFdTxQueue_PopOta();
            return 1U;
        }
        return 0U;
    }

    pSlot = CanFdTxQueue_PeekCharger();
    if (pSlot != NULL)
    {
        if (CanFdTxQueue_TryHwSend(phfdcan, pSlot) != 0U)
        {
            CanFdTxQueue_PopCharger();
            return 1U;
        }
        return 0U;
    }

    return 0U;
}

void CanFdTxQueue_Init(void)
{
    s_u8OtaTxHead = 0U;
    s_u8OtaTxTail = 0U;
    s_u8OtaTxCount = 0U;
    s_u8ChgTxHead = 0U;
    s_u8ChgTxTail = 0U;
    s_u8ChgTxCount = 0U;
    s_u8HwTxFifoEmpty = 1U;
}

uint8_t CanFdTxQueue_Enqueue(uint8_t u8Priority,
                             const FDCAN_TxHeaderTypeDef *pHeader,
                             const uint8_t *pu8Data)
{
    if (u8Priority == CANFD_TX_PRIO_OTA)
    {
        return CanFdTxQueue_Push(s_aOtaTxQueue, &s_u8OtaTxHead, &s_u8OtaTxTail, &s_u8OtaTxCount,
                                 CANFD_TX_OTA_QUEUE_DEPTH, pHeader, pu8Data);
    }

    return CanFdTxQueue_Push(s_aChgTxQueue, &s_u8ChgTxHead, &s_u8ChgTxTail, &s_u8ChgTxCount,
                             CANFD_TX_CHARGER_QUEUE_DEPTH, pHeader, pu8Data);
}

void CanFdTxQueue_Drain(FDCAN_HandleTypeDef *phfdcan)
{
    uint8_t u8Sent;

    if (phfdcan == NULL)
    {
        return;
    }

    for (u8Sent = 0U; u8Sent < CANFD_TX_DRAIN_MAX_PER_MS; u8Sent++)
    {
        if (CanFdTxQueue_SendOnePending(phfdcan) == 0U)
        {
            break;
        }
    }
}

uint8_t CanFdTxQueue_GetPendingCount(void)
{
    return (uint8_t)(s_u8OtaTxCount + s_u8ChgTxCount);
}

uint8_t CanFdTx_IsFullyDrained(FDCAN_HandleTypeDef *phfdcan)
{
    if (phfdcan == NULL)
    {
        return 0U;
    }
    if (CanFdTxQueue_GetPendingCount() > 0U)
    {
        return 0U;
    }
    if (HAL_FDCAN_GetTxFifoFreeLevel(phfdcan) < CANFD_TX_HW_FIFO_FREE_FULL)
    {
        return 0U;
    }
    s_u8HwTxFifoEmpty = 1U;
    return 1U;
}

void CanFd_SysTickCanService(void)
{
#if OTA_UPGRADE_ENABLE
    OtaGw_Poll();
#endif
    CanFdTxQueue_Drain(&hfdcan1);
    CAN_SendData_Run();
    CanFdTxQueue_Drain(&hfdcan1);
}

void HAL_FDCAN_TxFifoEmptyCallback(FDCAN_HandleTypeDef *hfdcan)
{
    if (hfdcan == &hfdcan1)
    {
        s_u8HwTxFifoEmpty = 1U;
    }
}
