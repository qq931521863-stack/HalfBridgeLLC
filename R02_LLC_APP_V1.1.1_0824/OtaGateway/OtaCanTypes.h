#ifndef _OtaCanTypes_H_
#define _OtaCanTypes_H_

#include "stm32g4xx_hal.h"

#define DF_CAN_DATA_LEN 64U

typedef struct
{
    FDCAN_RxHeaderTypeDef CANRxHeader;
    uint8_t rx_data[DF_CAN_DATA_LEN];
} can_receive_message_struct;

typedef struct
{
    FDCAN_TxHeaderTypeDef CANTxHeader;
    uint8_t tx_data[DF_CAN_DATA_LEN];
} can_trasnmit_message_struct;

#endif /* _OtaCanTypes_H_ */
