/******************************************************************************
    ?????:   Flash_spi.c
    ??? :   v1.0
    ????  :   ?????? 
    ????????20290608
    ??????????https://shop252056293.taobao.com/
******************************************************************************/
#include "main.h"
#include "stm32g4xx_hal_rcc.h"
#include "OtaPortal.h"
extern SPI_HandleTypeDef hspi1;
#define g_spi1_handler hspi1

extern volatile uint8_t g_Mx25FlashReadDebug;

static void Flash_SPI_PeriphHardReset(void);

/*
 * ???? Flash ??? HAL_SPI_TransmitReceive ????????????? IT/DMA??
 * ??????? HAL_SPI_Abort???? CR2 ???? TXEIE/RXNEIE/ERRIE ???Abort ???????????
 * ????????? CR2 ????/DMA ????? SPE ??????? FIFO???????????
 */
void Flash_SPI_BusRecover(void)
{
    uint32_t cr2 = READ_REG(g_spi1_handler.Instance->CR2);
    CLEAR_BIT(g_spi1_handler.Instance->CR2,
              SPI_CR2_TXEIE | SPI_CR2_RXNEIE | SPI_CR2_ERRIE | SPI_CR2_TXDMAEN | SPI_CR2_RXDMAEN);

    {
        uint32_t spin;
        uint32_t n;

        __HAL_SPI_DISABLE(&g_spi1_handler);
        spin = 200000U;
        while ((__HAL_SPI_GET_FLAG(&g_spi1_handler, SPI_FLAG_BSY) != RESET) && (spin > 0U))
        {
            spin--;
        }
        for (n = 0U; n < 32U; n++)
        {
            if (__HAL_SPI_GET_FLAG(&g_spi1_handler, SPI_FLAG_RXNE) == RESET)
            {
                break;
            }
            (void)(*(__IO uint8_t *)&g_spi1_handler.Instance->DR);
        }
        __HAL_SPI_CLEAR_OVRFLAG(&g_spi1_handler);
        __HAL_SPI_CLEAR_FREFLAG(&g_spi1_handler);
        if (__HAL_SPI_GET_FLAG(&g_spi1_handler, SPI_FLAG_CRCERR))
        {
            __HAL_SPI_CLEAR_CRCERRFLAG(&g_spi1_handler);
        }
        if (__HAL_SPI_GET_FLAG(&g_spi1_handler, SPI_FLAG_MODF))
        {
            __HAL_SPI_CLEAR_MODFFLAG(&g_spi1_handler);
        }
        __HAL_SPI_ENABLE(&g_spi1_handler);
        /* ????????��??????? leave SR ???????? 0x3A????APB ???��???ؕ??? CR2/??? */
        {
            uint32_t sr16 = READ_REG(g_spi1_handler.Instance->SR) & 0xFFFFUL;

            if ((sr16 & (SPI_SR_OVR | SPI_SR_MODF | SPI_SR_CRCERR | SPI_SR_FRE)) != 0U)
            {
                Flash_SPI_PeriphHardReset();
            }
        }
    }

    __HAL_UNLOCK(&g_spi1_handler);
    g_spi1_handler.State = HAL_SPI_STATE_READY;
    g_spi1_handler.ErrorCode = HAL_SPI_ERROR_NONE;
    if ((g_spi1_handler.Instance->CR1 & SPI_CR1_SPE) == 0U)
    {
        __HAL_SPI_ENABLE(&g_spi1_handler);
    }
}

//-----------------------------------------------------------
//Description    : ????SPI???
//Input          : None
//Output         : None
//Author         : C7 20190612
//Note(s)        : None
//-----------------------------------------------------------
void Flash_SPI_SetSpeed(uint8_t SPI_BaudRatePrescaler)
{
    assert_param(IS_SPI_BAUDRATE_PRESCALER(SPI_BaudRatePrescaler)); 				/* ?????????? */
    __HAL_SPI_DISABLE(&g_spi1_handler);            	 								/* ???SPI */
    g_spi1_handler.Instance->CR1 &= 0XFFC7;        		 							/* ??3-5?????????????????? */
    g_spi1_handler.Instance->CR1 |= SPI_BaudRatePrescaler << 3;     /* ????SPI??? */
    __HAL_SPI_ENABLE(&g_spi1_handler);             								  /* ???SPI */
}

/* RCC ��λ SPI1 ������� Init �ؽ��Ĵ�������Ƶ���� MX25L1606E_Flash_Init �� SetSpeed һ�� */
static void Flash_SPI_PeriphHardReset(void)
{
    __HAL_SPI_DISABLE(&g_spi1_handler);
    __HAL_RCC_SPI1_FORCE_RESET();
    __HAL_RCC_SPI1_RELEASE_RESET();
    (void)HAL_SPI_Init(&g_spi1_handler);
    Flash_SPI_SetSpeed(SPI_BAUDRATEPRESCALER_32);
}

//-----------------------------------------------------------
//Description    : Flash_SPI???????
//Input          : None
//Output         : None
//Author         : C7 20190612
//Note(s)        : None
//-----------------------------------------------------------
uint8_t Flash_SPI_ReadWriteByte(uint8_t TxData)
{		
    uint8_t rxdata = 0xFFU;
    HAL_StatusTypeDef st = HAL_SPI_TransmitReceive(&g_spi1_handler, &TxData, &rxdata, 1U, 1000U);
    if (st != HAL_OK)
    {
        __HAL_SPI_CLEAR_OVRFLAG(&g_spi1_handler);
    }
    return rxdata;
}

//-----------------------------------------------------------
//Description    : Flash_SPI?????????
//Input          : None
//Output         : None
//Author         : C7 20190612
//Note(s)        : None
//-----------------------------------------------------------
void Flash_SPI_Init(void)
{
	GPIO_InitTypeDef gpio_init_struct;
	__HAL_RCC_SPI1_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();


	HAL_GPIO_DeInit(GPIOB, GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_3);


	// PB4 - SPI1_MISO
	gpio_init_struct.Pin = GPIO_PIN_4;
	gpio_init_struct.Mode = GPIO_MODE_AF_PP; // ???????????????
	gpio_init_struct.Pull = GPIO_NOPULL;
	gpio_init_struct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	gpio_init_struct.Alternate = GPIO_AF5_SPI1; // ???????STM32??????????????????????????STM32F4??????AF5
	HAL_GPIO_Init(GPIOB, &gpio_init_struct);

	// ????PB5?SPI1??MISO???????????
	gpio_init_struct.Pin = GPIO_PIN_5;
	gpio_init_struct.Alternate = GPIO_AF5_SPI1; // ??????????????????????????
	HAL_GPIO_Init(GPIOB, &gpio_init_struct);

	// PB3 - SPI1_SCK
	gpio_init_struct.Pin = GPIO_PIN_3;
	HAL_GPIO_Init(GPIOB, &gpio_init_struct);

	__HAL_RCC_SPI1_CLK_ENABLE(); 										/* SPI2?????? */

	g_spi1_handler.Instance = SPI1;                                		/* SPI2 */
	g_spi1_handler.Init.Mode = SPI_MODE_MASTER;                        	/* ????SPI????????????????? */
	g_spi1_handler.Init.Direction = SPI_DIRECTION_2LINES;              	/* ????SPI?????????????????:SPI?????????? */
	g_spi1_handler.Init.DataSize = SPI_DATASIZE_8BIT;                  	/* ????SPI?????????:SPI???????8????? */
	g_spi1_handler.Init.CLKPolarity = SPI_POLARITY_HIGH;               	/* ????????????????????? */
	g_spi1_handler.Init.CLKPhase = SPI_PHASE_2EDGE;                    	/* ?????????????????????????????????????????? */
	g_spi1_handler.Init.NSS = SPI_NSS_SOFT;                            	/* NSS??????????NSS?????????????????SSI????????:???NSS?????SSI?????? */
	g_spi1_handler.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4; 	/* ????????????????:????????????256 */
	g_spi1_handler.Init.FirstBit = SPI_FIRSTBIT_MSB;                   	/* ???????????MSB??????LSB?????:????????MSB????? */
	g_spi1_handler.Init.TIMode = SPI_TIMODE_DISABLE;                   	/* ???TI?? */
	g_spi1_handler.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;   	/* ??????CRC???? */
	g_spi1_handler.Init.CRCPolynomial = 10;                             	/* CRC?????????? */
	HAL_SPI_Init(&g_spi1_handler);                                     	/* ????? */

	__HAL_SPI_ENABLE(&g_spi1_handler); /* ???SPI2 */

	Flash_SPI_ReadWriteByte(0Xff); /* ????????, ???????????8?????????, ?????DR??????, ????? */
}
