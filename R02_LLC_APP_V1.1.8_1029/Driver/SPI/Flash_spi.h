/******************************************************************************
    �ļ���:   Flash_spi.h
    �汾 :   v1.0
    ����  :   ��ŵ�Ƽ� 
    �޸����ڣ�20290608
    �Ա������ӣ�https://shop252056293.taobao.com/
******************************************************************************/
#ifndef _flash_spi_H
#define _flash_spi_H
#include "main.h"

extern void Flash_SPI_Init(void);			 //��ʼ��SPI��
extern void Flash_SPI_SetSpeed(uint8_t SPI_BaudRatePrescaler);
extern uint8_t Flash_SPI_ReadWriteByte(uint8_t TxData);//SPI���߶�дһ���ֽ�
#define FLASH_CS_High()   	  HAL_GPIO_WritePin(GPIOB,GPIO_PIN_6,GPIO_PIN_SET);
#define FLASH_CS_Low()     	  HAL_GPIO_WritePin(GPIOB,GPIO_PIN_6,GPIO_PIN_RESET);
/* CS ���������ⲿ���ߺ��ٵ��ã�Abort + ������־ + ǿ�ƽ������������ڶ��� FlashRead HAL_BUSY */
extern void Flash_SPI_BusRecover(void);
#endif

