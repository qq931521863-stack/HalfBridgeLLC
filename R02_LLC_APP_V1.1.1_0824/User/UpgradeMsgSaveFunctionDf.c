/******************************************************************************
    File:      UpgradeMsgSaveFunctionDf.c
    Version:   v1.0
    Author:    C7
    Date:      2029-06-08
    Store:     https://shop252056293.taobao.com/
******************************************************************************/
#include "UpgradeFeatureConfig.h"
#if OTA_UPGRADE_ENABLE

#include "HostProgStatusInit.h"
#include "crc16.h"
#include "mx25L1606E.h"
#include "OtaPortal.h"
#include "UpgradeMsgSaveFunctionDf.h"
_StProgStatus  StMasterProgStatus;    // Master device upgrade status structure
_StProgStatus  StSlave1ProgStatus;    // Slave 1 device upgrade status structure
_utSysStatusMsg utSysStatusMsg;

//-----------------------------------------------------------
// Description    : Save the upgrade file message
// Input          : None
// Output         : None
// Author         : C7 20190612
// Note(s)        : None
//-----------------------------------------------------------
void SaveWriteUpgradeFileMsg(_StProgStatus *Prog_Status, uint32_t u32Addr)
{	
    FlashSector_Erase(u32Addr);   
    Prog_Status->StProgStatusMsg.u16ProgMsgCRC16Value = Modbus_CalCRC(Prog_Status->u8Buf, sizeof(_StProgStatus) - 2);
    SPI_FLASH_BufferWrite(Prog_Status->u8Buf, u32Addr, sizeof(_StProgStatus));      // Write to flash
}

//-----------------------------------------------------------
// Description    : Read the upgrade file message
// Input          : None
// Output         : None
// Author         : C7 20190612
// Note(s)        : None
//-----------------------------------------------------------
uint16_t ReadWriteUpgradeFileMsg(_StProgStatus *Prog_Status, uint32_t u32Addr)
{	
    uint16_t u16CRC16 = 0;

    FlashRead(Prog_Status->u8Buf, u32Addr, sizeof(_StProgStatus));        // Read upgrade info from Flash
    u16CRC16 = Modbus_CalCRC(Prog_Status->u8Buf, sizeof(_StProgStatus) - 2); // Recalculate CRC16
    if(u16CRC16 != Prog_Status->StProgStatusMsg.u16ProgMsgCRC16Value)
    {
        HostProgStatus_ApplyFactoryDefaults(Prog_Status->u8Buf);
        SaveWriteUpgradeFileMsg(Prog_Status, u32Addr);
        return 0x00;       
    }
    else
    {
        uint8_t u8SanitizeDirty = HostProgStatus_SanitizeAfterRead(Prog_Status->u8Buf);
        if (Prog_Status->StProgStatusMsg.u16UpgradeState != DF_WAIT_DOWNLOAD_UPGRADR_FILE_STATUS
            && Prog_Status->StProgStatusMsg.u16UpgradeState != DF_WAIT_INTO_UPGRADR_STATUS
            && Prog_Status->StProgStatusMsg.u16UpgradeState != DF_UPGRADR_FINISH_STATUS
            && Prog_Status->StProgStatusMsg.u16UpgradeState != DF_UPGRADE_FAIL_STATUS
            && Prog_Status->StProgStatusMsg.u16UpgradeState != DF_NOT_UPGRADE_STATUS)
        {
            Prog_Status->StProgStatusMsg.u16UpgradeState = DF_WAIT_DOWNLOAD_UPGRADR_FILE_STATUS;
        }

        // Check if block A bin file status is legal, otherwise set invalid
        if (Prog_Status->StProgStatusMsg.u8BlockABinFileValidFlag != DF_UPGRADE_FILE_INVAILE_STATUS 
            && Prog_Status->StProgStatusMsg.u8BlockABinFileValidFlag != DF_UPGRADE_FILE_VAILE_STATUS)
        {
            Prog_Status->StProgStatusMsg.u8BlockABinFileValidFlag = DF_UPGRADE_FILE_INVAILE_STATUS;
        }
        // Check if block B bin file status is legal, otherwise set invalid
        if (Prog_Status->StProgStatusMsg.u8BlockBBinFileValidFlag != DF_UPGRADE_FILE_INVAILE_STATUS 
            && Prog_Status->StProgStatusMsg.u8BlockBBinFileValidFlag != DF_UPGRADE_FILE_VAILE_STATUS)
        {
            Prog_Status->StProgStatusMsg.u8BlockBBinFileValidFlag = DF_UPGRADE_FILE_INVAILE_STATUS;
        }
        
        // If block A length is 0xFFFFFFFF or 0, mark as invalid and reset length
        if (Prog_Status->StProgStatusMsg.u32BlockABinFileLength == 0xFFFFFFFF ||
            Prog_Status->StProgStatusMsg.u32BlockABinFileLength == 0x0)
        {
            Prog_Status->StProgStatusMsg.u8BlockABinFileValidFlag = DF_UPGRADE_FILE_INVAILE_STATUS;
            Prog_Status->StProgStatusMsg.u32BlockABinFileLength = 0x0000;
        }
        // If block B length is 0xFFFFFFFF or 0, mark as invalid and reset length
        if (Prog_Status->StProgStatusMsg.u32BlockBBinFileLength == 0xFFFFFFFF ||
            Prog_Status->StProgStatusMsg.u32BlockBBinFileLength == 0x0)
        {
            Prog_Status->StProgStatusMsg.u8BlockBBinFileValidFlag = DF_UPGRADE_FILE_INVAILE_STATUS;
            Prog_Status->StProgStatusMsg.u32BlockBBinFileLength = 0x0000;
        }
        
        // If user block field value is not A or B, force both blocks to invalid and set user block to A
        if (Prog_Status->StProgStatusMsg.u8FlashUserBlock != DF_CHS_BLOCK_A &&
            Prog_Status->StProgStatusMsg.u8FlashUserBlock != DF_CHS_BLOCK_B)
        {
            Prog_Status->StProgStatusMsg.u8BlockABinFileValidFlag = DF_UPGRADE_FILE_INVAILE_STATUS;
            Prog_Status->StProgStatusMsg.u8BlockBBinFileValidFlag = DF_UPGRADE_FILE_INVAILE_STATUS;
            Prog_Status->StProgStatusMsg.u8FlashUserBlock = DF_CHS_BLOCK_A;
        }       
        if (u8SanitizeDirty != 0U)
        {
            SaveWriteUpgradeFileMsg(Prog_Status, u32Addr);
        }
        return 0x01;
    }
}
//-----------------------------------------------------------
// Description    : System software reset
// Input          : None
// Output         : None
// Author         : C7 20190612
// Note(s)        : None
//-----------------------------------------------------------
void Sys_software_reset(void)
{
    __set_FAULTMASK(1); 
    NVIC_SystemReset();
}
//----------------------------------------------------------------------
// Description    : Disable global interrupts
// Input          : None
// Output         : None
// Author         : any at 2023/07/20
// Note(s)        : None
//----------------------------------------------------------------------
void DisenabledInterrupt(void)
{
    __set_FAULTMASK(1);
}
//----------------------------------------------------------------------
// Description    : Enable global interrupts
// Input          : None
// Output         : None
// Author         : C7 20190612
// Note(s)        : None
//----------------------------------------------------------------------
void EnabledInterrupt(void)
{
    __set_FAULTMASK(0);
}

#else /* OTA_UPGRADE_ENABLE */

#include "UpgradeMsgSaveFunctionDf.h"

_StProgStatus  StMasterProgStatus;
_StProgStatus  StSlave1ProgStatus;
_utSysStatusMsg utSysStatusMsg;

void SaveWriteUpgradeFileMsg(_StProgStatus *Prog_Status, uint32_t u32Addr)
{
    (void)Prog_Status;
    (void)u32Addr;
}
uint16_t ReadWriteUpgradeFileMsg(_StProgStatus *Prog_Status, uint32_t u32Addr)
{
    (void)Prog_Status;
    (void)u32Addr;
    return 0U;
}
void Sys_software_reset(void) {}
void DisenabledInterrupt(void) {}
void EnabledInterrupt(void) {}

#endif /* OTA_UPGRADE_ENABLE */
