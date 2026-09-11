/******************************************************************************
    文件名:   crc16.h
    版本 :   v1.0
    作者  :   安诺科技 
    修改日期：20290608
    淘宝店连接：https://shop252056293.taobao.com/
******************************************************************************/
#ifndef CRC16_H
#define CRC16_H
#include "stdint.h"
extern uint16_t  Modbus_CalCRC(const uint8_t buff[], const uint16_t u16len);
extern uint16_t File_Bin_ModBusCRC16(uint8_t *CRC_Buf,uint16_t CRC_Leni,uint16_t CRC_Sum_value);
extern void CalcCRCStandard(uint8_t *pSrc1, uint32_t nNumberOfBytes, uint32_t  *pChecksum);
#endif 

