/******************************************************************************
    �ļ���:   mx25L1606E.h
    �汾 :   v1.0
    ����  :   ��ŵ�Ƽ� 
    �޸����ڣ�20290608
    �Ա������ӣ�https://shop252056293.taobao.com/
******************************************************************************/
#ifndef MX25L1606E_H
#define MX25L1606E_H
#include "OtaPortal.h"
#include "main.h"

#define FLASH_BUSY_TIMEOUT  200

//#define FLASH_PAGE_SIZE     256

#define RDID_CMD  0X9F
#define RDSD_CMD  0X05
#define WRSR_CMD  0x01
#define REMS_CMD  0X90
#define DP_CMD    0XB9
#define RDP_CMD   0XAB
#define RES_CMD   0XAB
#define PP_CMD    0X02
#define WREN_CMD  0X06
#define WRDI_CMD  0X04
#define CE_CMD    0XC7
#define BE_CMD    0X52
#define SE_CMD    0x20
#define READ_CMD  0X03
#define FREAD_CMD 0X0B
#define DREAD_CMD 0X3B
#define CMD_BLOCK_ERASE64KB       0xd8


typedef struct
{
    uint8_t ManufacturerID;
    uint8_t DeviceID[2];
}flashInfoTypedef;

/* 由 RS485 发包路径置 1：仅在 FlashRead 内打印分段/SPI 状态，避免 CAN 下载刷屏 */
extern volatile uint8_t g_Mx25FlashReadDebug;
void MX25L1606E_Flash_Init(void);
void FlashPage_Read(uint8_t *ptr,uint32_t pageAddr,uint32_t pageSize);
void FlashPage_Write(uint8_t *ptr,uint32_t pageAddr,uint32_t pageSize);
void FlashSector_Erase(uint32_t sectorAddr);
void FlashBlock_Erase(uint8_t blockAddr);
void FlashChip_Erase(void);
extern void FlashRead(uint8_t *ptr,uint32_t addr,uint32_t len);
/* u8SkipBusRecover!=0：跳过 BusRecover（合包校验连续读时由会话入口恢复一次） */
extern void FlashReadEx(uint8_t *ptr, uint32_t addr, uint32_t len, uint8_t u8SkipBusRecover);
extern void SPI_FLASH_BufferWrite(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);
extern void EraseMasterBinFileBlockA(void);
extern void EraseMasterBinFileBlockB(void);
extern void EraseSlave1BinFileBlock(void);
extern void EraseSlave2BinFileBlock(void);
extern void EraseExtFlashPackStore(void);

#ifndef FLASH_SPI_SELFTEST_ENABLE
#define FLASH_SPI_SELFTEST_ENABLE 0
#endif
#if FLASH_SPI_SELFTEST_ENABLE
/* 上电可选：连续读 0x300000 -> 0x300400 -> 0x300000，比对首尾两次是否一致；返回 0 通过 */
extern uint8_t MX25_Flash_RunSpiSelfTest(void);
#endif
#endif
