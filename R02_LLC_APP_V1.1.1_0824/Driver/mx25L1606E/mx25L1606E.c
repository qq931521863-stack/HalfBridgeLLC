#include "OtaPortal.h"
#include "main.h"
#include "HostFlashMemoryMap.h"
#include "Flash_spi.h"
#include "UpgradeMsgSaveFunctionDf.h"
#include "mx25L1606E.h"
#define SPI_FLASH_PageSize              256
#define SPI_FLASH_PerWritePageSize      256


#define DUMMY_BYTE  0XFF
void FlashGet_Info(flashInfoTypedef * pFlashInfo);
void FlashGet_ElectronicInfo(flashInfoTypedef * pFlashInfo);

//-----------------------------------------------------------
//Description    : MX25L1606E?????????
//Input          : None
//Output         : None
//Author         : C7 20190612
//Note(s)        : None
//-----------------------------------------------------------
void MX25L1606E_Flash_Init(void)
{
	flashInfoTypedef flashInfo;   
	GPIO_InitTypeDef gpio_init_struct;
	 /*????LED????GPIO???????*/
    __HAL_RCC_GPIOB_CLK_ENABLE();
	//MX25L1606E��? CS?????
	gpio_init_struct.Pin = GPIO_PIN_6;
	gpio_init_struct.Mode = GPIO_MODE_OUTPUT_PP;
	gpio_init_struct.Pull = GPIO_PULLUP;
	gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOB, &gpio_init_struct);	
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_SET);

	Flash_SPI_Init();                         						 //SPI1??????????
	Flash_SPI_SetSpeed(SPI_BAUDRATEPRESCALER_32);					 //?????18M???,??????
	//???Flash��?ID??
	FlashGet_ElectronicInfo(&flashInfo);
	if((flashInfo.ManufacturerID==0x20)&&(flashInfo.DeviceID[0]==0x14))
	{ 

	}
	FlashGet_Info(&flashInfo);
	if((flashInfo.DeviceID[0]==0x40)&&(flashInfo.DeviceID[1]==0x16))
	{

	}
}
//-----------------------------------------------------------
//Description    : ???MX25L1606E��??????????��????��?????
//Input          : None
//Output         : None
//Author         : C7 20190612
//Note(s)        : None
//-----------------------------------------------------------
void SPI_FLASH_WaitForWriteEnd(void)
{
    uint8_t FLASH_Status = 0;
    FLASH_CS_Low(); /* ??? FLASH: CS ?? */
    Flash_SPI_ReadWriteByte(RDSD_CMD);/* ???? ????????? ???? */
    do/* ??FLASH???????? */
    {	
        FLASH_Status = Flash_SPI_ReadWriteByte(DUMMY_BYTE);/* ???FLASH��?????????? */	 
    }
    while ((FLASH_Status & 0x01) == SET);  /* ????��???? */
    FLASH_CS_High(); /* ?????  FLASH: CS ?? */
}
//-----------------------------------------------------------
//Description    : ???MX25L1606E��?ID??
//Input          : None
//Output         : None
//Author         : C7 20190612
//Note(s)        : None
//-----------------------------------------------------------
void FlashGet_Info(flashInfoTypedef * pFlashInfo)
{
//	    FlashWait_Busy();
    FLASH_CS_Low();
    Flash_SPI_ReadWriteByte(RDID_CMD);
    pFlashInfo->ManufacturerID = Flash_SPI_ReadWriteByte(DUMMY_BYTE);
    pFlashInfo->DeviceID[0] = Flash_SPI_ReadWriteByte(DUMMY_BYTE);
    pFlashInfo->DeviceID[1] = Flash_SPI_ReadWriteByte(DUMMY_BYTE);
    FLASH_CS_High();    
}
//-----------------------------------------------------------
//Description    : ???MX25L1606E��???????ID??
//Input          : None
//Output         : None
//Author         : C7 20190612
//Note(s)        : None
//-----------------------------------------------------------
void FlashGet_ElectronicInfo(flashInfoTypedef * pFlashInfo)
{
//	    FlashWait_Busy();
    FLASH_CS_Low();
    Flash_SPI_ReadWriteByte(REMS_CMD);
    Flash_SPI_ReadWriteByte(DUMMY_BYTE);
    Flash_SPI_ReadWriteByte(DUMMY_BYTE);
    Flash_SPI_ReadWriteByte(0x00);  //manufacturer's ID first
    pFlashInfo->ManufacturerID = Flash_SPI_ReadWriteByte(DUMMY_BYTE);
    pFlashInfo->DeviceID[0] = Flash_SPI_ReadWriteByte(DUMMY_BYTE);   
    FLASH_CS_High();    
}
//-----------------------------------------------------------
//Description    : ????MX25L1606E????????
//Input          : None
//Output         : None
//Author         : C7 20190612
//Note(s)        : None
//-----------------------------------------------------------
void FlashEnter_DeepPowerDown(void)
{
    FLASH_CS_Low();
    Flash_SPI_ReadWriteByte(DP_CMD);   
    FLASH_CS_High();    
}
//-----------------------------------------------------------
//Description    : ??MX25L1606E???????��???
//Input          : None
//Output         : None
//Author         : C7 20190612
//Note(s)        : None
//-----------------------------------------------------------
void FlashReleaseFrom_DeepPowerDown(void)
{
    FLASH_CS_Low();
    Flash_SPI_ReadWriteByte(RDP_CMD);   
    FLASH_CS_High();    
}
uint8_t FlashReleaseFrom_DeepPowerDownE(void)
{   
    uint8_t ElectronicSignature;
    FLASH_CS_Low();
    Flash_SPI_ReadWriteByte(RES_CMD);
    Flash_SPI_ReadWriteByte(DUMMY_BYTE);
    Flash_SPI_ReadWriteByte(DUMMY_BYTE);
    Flash_SPI_ReadWriteByte(DUMMY_BYTE);
    ElectronicSignature = Flash_SPI_ReadWriteByte(DUMMY_BYTE);
    FLASH_CS_High();    
    return ElectronicSignature;
}
//-----------------------------------------------------------
//Description    : MX25L1606E��???
//Input          : None
//Output         : None
//Author         : C7 20190612
//Note(s)        : None
//-----------------------------------------------------------
void FlashWriteEnable(void)
{
    FLASH_CS_Low();
    Flash_SPI_ReadWriteByte(WREN_CMD);   
    FLASH_CS_High();    
}
//-----------------------------------------------------------
//Description    : MX25L1606E��????
//Input          : None
//Output         : None
//Author         : C7 20190612
//Note(s)        : None
//-----------------------------------------------------------
void FlashWriteDisable(void)
{
    FLASH_CS_Low();
    Flash_SPI_ReadWriteByte(WRDI_CMD);   
    FLASH_CS_High();    
}
//-----------------------------------------------------------
//Description    : MX25L1606E��??????
//Input          : None
//Output         : None
//Author         : C7 20190612
//Note(s)        : None
//-----------------------------------------------------------
void FlashWrite_StaRegister(uint8_t flashSta)
{
    FLASH_CS_Low();
    Flash_SPI_ReadWriteByte(WRSR_CMD); 
    Flash_SPI_ReadWriteByte(flashSta); 
    FLASH_CS_High();    
}

//-----------------------------------------------------------
//Description    : ?��????
//Input          : None
//Output         : None
//Author         : C7 20190612
//Note(s)        : None
//-----------------------------------------------------------
void FlashPage_Write(uint8_t *ptr,uint32_t pageAddr,uint32_t pageSize)
{
    uint32_t addr,i;
	  addr=pageAddr;
    FlashWriteEnable();
    FLASH_CS_Low();
    Flash_SPI_ReadWriteByte(PP_CMD); 
    Flash_SPI_ReadWriteByte(addr>>16);
    Flash_SPI_ReadWriteByte(addr>>8);
    Flash_SPI_ReadWriteByte(addr); 
    for(i=0;i<pageSize;i++)
        Flash_SPI_ReadWriteByte(*ptr++); 
    FLASH_CS_High(); 

 /* ???��?????*/
  SPI_FLASH_WaitForWriteEnd();
}
 /**
  * @brief  ??FLASH��????????????????��???????????????????
  * @param	pBuffer???��??????????
  * @param  WriteAddr??��????
  * @param  NumByteToWrite??��?????????
  * @retval ??
  */
void SPI_FLASH_BufferWrite(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
    uint16_t NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;

    /*mod??????????writeAddr??SPI_FLASH_PageSize??????????????Addr??0*/
    Addr = WriteAddr % SPI_FLASH_PageSize;

    /*??count??????????????????????*/
    count = SPI_FLASH_PageSize - Addr;
    /*??????��?????????*/
    NumOfPage =  NumByteToWrite / SPI_FLASH_PageSize;
    /*mod???????????????????????????*/
    NumOfSingle = NumByteToWrite % SPI_FLASH_PageSize;

    /* Addr=0,??WriteAddr ????????? aligned  */
    if (Addr == 0)
    {
        /* NumByteToWrite < SPI_FLASH_PageSize */
        if (NumOfPage == 0) 
        {
            FlashPage_Write(pBuffer, WriteAddr, NumByteToWrite);
        }
        else /* NumByteToWrite > SPI_FLASH_PageSize */
        { 
            /*??????????��??*/
            while (NumOfPage--)
            {
                FlashPage_Write(pBuffer, WriteAddr, SPI_FLASH_PageSize);
                WriteAddr +=  SPI_FLASH_PageSize;
                pBuffer += SPI_FLASH_PageSize;
            }
            /*???��????????????????????��??*/
            FlashPage_Write(pBuffer, WriteAddr, NumOfSingle);
        }
    }
    /* ??????? SPI_FLASH_PageSize ??????  */
    else 
    {
        /* NumByteToWrite < SPI_FLASH_PageSize */
        if (NumOfPage == 0)
        {
            /*????????count??��???NumOfSingle��????��????*/
            if (NumOfSingle > count) 
            {
                temp = NumOfSingle - count;
                /*??��??????*/
                FlashPage_Write(pBuffer, WriteAddr, count);

                WriteAddr +=  count;
                pBuffer += count;
                /*??��????????*/
                FlashPage_Write(pBuffer, WriteAddr, temp);
            }
            else /*????????count??��????��??NumOfSingle??????*/
            {
                FlashPage_Write(pBuffer, WriteAddr, NumByteToWrite);
            }
        }
        else /* NumByteToWrite > SPI_FLASH_PageSize */
        {
            /*?????????????count??????????????????????*/
            NumByteToWrite -= count;
            NumOfPage =  NumByteToWrite / SPI_FLASH_PageSize;
            NumOfSingle = NumByteToWrite % SPI_FLASH_PageSize;

            /* ??��??count????????????????????��???????? */
            FlashPage_Write(pBuffer, WriteAddr, count);

            /* ?????????????????????? */
            WriteAddr +=  count;
            pBuffer += count;
            /*?????????��??*/
            while (NumOfPage--)
            {
                FlashPage_Write(pBuffer, WriteAddr, SPI_FLASH_PageSize);
                WriteAddr +=  SPI_FLASH_PageSize;
                pBuffer += SPI_FLASH_PageSize;
            }
            /*???��????????????????????��??*/
            if (NumOfSingle != 0)
            {
                FlashPage_Write(pBuffer, WriteAddr, NumOfSingle);
            }
        }
    }
}
//-----------------------------------------------------------
//Description    : ????��?????
//Input          : None
//Output         : None
//Author         : C7 20190612
//Note(s)        : None
//-----------------------------------------------------------
void FlashChip_Erase(void)
{
    SPI_FLASH_WaitForWriteEnd();             /* ???��?????*/
    FlashWriteEnable();
    FLASH_CS_Low();
    Flash_SPI_ReadWriteByte(CE_CMD);     
    FLASH_CS_High(); 
    SPI_FLASH_WaitForWriteEnd();             /* ??????????*/
}
//-----------------------------------------------------------
//Description    : ????? ????????????��?64K
//Input          : None
//Output         : None
//Author         : C7 20190612
//Note(s)        : None
//-----------------------------------------------------------
void FlashBlock_Erase(uint8_t blockAddr)
{
    uint32_t addr;//,i;  
    addr=((uint32_t)blockAddr)<<16;//???block??16?????????????????4KByte;
    SPI_FLASH_WaitForWriteEnd();  /* ???????*/
    FlashWriteEnable();
    FLASH_CS_Low();
    Flash_SPI_ReadWriteByte(BE_CMD); 
    Flash_SPI_ReadWriteByte(addr>>16);
    Flash_SPI_ReadWriteByte(addr>>8);
    Flash_SPI_ReadWriteByte(addr); 
    FLASH_CS_High();   
    SPI_FLASH_WaitForWriteEnd();             /* ??????????*/
}

//------------------------------------------------------------------------------
//Description    : ???????
//Input          : None
//Output         : None
//Author         : C7
//Note(s)        : None
//------------------------------------------------------------------------------
uint8_t W25Q64_64K_Blockerase(uint16_t sect_no, uint8_t flgwait) 
{
    uint32_t addr;	
    FlashWriteEnable();
    addr = sect_no;
    addr = addr << 16;      //  64K ?  1 0000 0000 0000 0000  ???????????16�� 
    FLASH_CS_Low(); 
    Flash_SPI_ReadWriteByte(CMD_BLOCK_ERASE64KB); 
    Flash_SPI_ReadWriteByte(addr >> 16);
    Flash_SPI_ReadWriteByte(addr>>8);
    Flash_SPI_ReadWriteByte(addr); 
    FLASH_CS_High(); 
    SPI_FLASH_WaitForWriteEnd();
    return 0;
}
//------------------------------------------------------------------------------
//Description    : ???��???
//Input          : None
//Output         : None
//Author         : C7
//Note(s)        : None
//------------------------------------------------------------------------------
void FlashSector_Erase(uint32_t sectorAddr)
{
    uint32_t addr;//,i;
    addr=((uint32_t)sectorAddr)/*<<12*/;   //??????��?��?4K???????4K
    SPI_FLASH_WaitForWriteEnd();  /* ???????*/
    FlashWriteEnable();
    FLASH_CS_Low();
    Flash_SPI_ReadWriteByte(SE_CMD); 
    Flash_SPI_ReadWriteByte(addr>>16);
    Flash_SPI_ReadWriteByte(addr>>8);
    Flash_SPI_ReadWriteByte(addr); 
    FLASH_CS_High(); 
    SPI_FLASH_WaitForWriteEnd();             /* ??????????*/
}
/*-------------------------------------------------------------------------------------
  * @brief  ???FLASH???? 
  * @param 	ptr???��????????????
  * @param   addr????????
  * @param   len????????????  (????????)
  * @retval ??
------------------------------------------------------------------------------------------*/
void FlashReadEx(uint8_t *ptr, uint32_t addr, uint32_t len, uint8_t u8SkipBusRecover)
{
    uint32_t addr_tmp,i;
    addr_tmp=addr;
    if (u8SkipBusRecover == 0U)
    {
        FLASH_CS_High();
        Flash_SPI_BusRecover();
    }
    FLASH_CS_Low();
    Flash_SPI_ReadWriteByte(READ_CMD);
    Flash_SPI_ReadWriteByte(addr_tmp>>16);
    Flash_SPI_ReadWriteByte(addr_tmp>>8);
    Flash_SPI_ReadWriteByte(addr_tmp);
    for(i=0;i<len;i++)
    {
        *ptr++=Flash_SPI_ReadWriteByte(DUMMY_BYTE);
    }
    FLASH_CS_High();
}

void FlashRead(uint8_t *ptr, uint32_t addr, uint32_t len)
{
    FlashReadEx(ptr, addr, len, 0U);
}
/*------------------------------------------------------------------------------------------
  * @brief  ???FLASH???????
  * @param 	ptr???��????????????
  * @param   pageAddr????????
  * @param   pageSize????????????  ??????????256
  * @retval ??
------------------------------------------------------------------------------------------*/
void FlashPage_Read(uint8_t *ptr,uint32_t pageAddr,uint32_t pageSize)
{
    uint32_t addr,i;
      addr=pageAddr;
    addr=((uint32_t)addr)<<8;
    FLASH_CS_Low();
    Flash_SPI_ReadWriteByte(READ_CMD); 
    Flash_SPI_ReadWriteByte(addr>>16);
    Flash_SPI_ReadWriteByte(addr>>8);
    Flash_SPI_ReadWriteByte(addr); 
    for(i=0;i<pageSize;i++)
        *ptr++=Flash_SPI_ReadWriteByte(DUMMY_BYTE); 
    FLASH_CS_High();    
}

void FlashFastRead(uint32_t addr,uint8_t *ptr,uint32_t len)
{
    uint32_t addr_tmp,i;
    addr_tmp=addr&0x001fffff;
    FLASH_CS_Low();
    Flash_SPI_ReadWriteByte(FREAD_CMD); 
    Flash_SPI_ReadWriteByte(addr_tmp>>16);
    Flash_SPI_ReadWriteByte(addr_tmp>>8);
    Flash_SPI_ReadWriteByte(addr_tmp); 
    Flash_SPI_ReadWriteByte(DUMMY_BYTE); 
    for(i=0;i<len;i++)
        *ptr++=Flash_SPI_ReadWriteByte(DUMMY_BYTE); 
    FLASH_CS_High();    
}
//------------------------------------------------------------------------------
//Description    : ????????????��A??  
//Input          : None
//Output         : None
//Author         : C7
//Note(s)        : None
//------------------------------------------------------------------------------
void EraseMasterBinFileBlockA(void)
{
	uint16_t i;
	uint16_t end = (uint16_t)(DF_HOST_EXT_BANK_A_64K_START + DF_HOST_EXT_BANK_64K_ERASE_BLOCKS);

	for (i = DF_HOST_EXT_BANK_A_64K_START; i < end; i++)
	{
        W25Q64_64K_Blockerase(i, 1);
	}
}

//------------------------------------------------------------------------------
//Description    : ????????????��B??  
//Input          : None
//Output         : None
//Author         : C7
//Note(s)        : None
//------------------------------------------------------------------------------
void EraseMasterBinFileBlockB(void)
{
    uint16_t i;
	uint16_t end = (uint16_t)(DF_HOST_EXT_BANK_B_64K_START + DF_HOST_EXT_BANK_64K_ERASE_BLOCKS);

	for (i = DF_HOST_EXT_BANK_B_64K_START; i < end; i++)
	{
        W25Q64_64K_Blockerase(i, 1);
	}
}
//------------------------------------------------------------------------------
//Description    : ???????1????��??   
//Input          : None
//Output         : None
//Author         : C7
//Note(s)        : None
//------------------------------------------------------------------------------
void EraseSlave1BinFileBlock(void)
{
	uint16_t i;
	for(i = 16;i < 24;i++)			   
	{
        W25Q64_64K_Blockerase(i,1);	
	}  
}
//------------------------------------------------------------------------------
//Description    : ???????2????��??   
//Input          : None
//Output         : None
//Author         : C7
//Note(s)        : None
//------------------------------------------------------------------------------
void EraseSlave2BinFileBlock(void)
{
	uint16_t i;
	for(i = 24;i < 32;i++)			   
	{
        W25Q64_64K_Blockerase(i,1);	
	}  
}

void EraseExtFlashPackStore(void)
{
	uint16_t i;
	for (i = 0; i < DF_EXT_FLASH_PACK_STORE_64K_BLOCKS; i++)
	{
		(void)W25Q64_64K_Blockerase((uint16_t)(DF_EXT_FLASH_PACK_STORE_FIRST_64K_BLOCK_IDX + i), 1);
	}
}


