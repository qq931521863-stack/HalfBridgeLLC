/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file         stm32g4xx_hal_msp.c
  * @brief        This file provides code for the MSP Initialization
  *               and de-Initialization codes.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */
extern DMA_HandleTypeDef hdma_adc1;

extern DMA_HandleTypeDef hdma_adc2;

extern DMA_HandleTypeDef hdma_usart3_rx;

extern DMA_HandleTypeDef hdma_usart3_tx;

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN Define */

/* USER CODE END Define */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN Macro */

/* USER CODE END Macro */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* External functions --------------------------------------------------------*/
/* USER CODE BEGIN ExternalFunctions */

/* USER CODE END ExternalFunctions */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

void HAL_HRTIM_MspPostInit(HRTIM_HandleTypeDef *hhrtim);

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);
                                        /**
  * Initializes the Global MSP.
  */
void HAL_MspInit(void)
{

  /* USER CODE BEGIN MspInit 0 */

  /* USER CODE END MspInit 0 */

  __HAL_RCC_SYSCFG_CLK_ENABLE();
  __HAL_RCC_PWR_CLK_ENABLE();

  /* System interrupt init*/

  /** Disable the internal Pull-Up in Dead Battery pins of UCPD peripheral
  */
  HAL_PWREx_DisableUCPDDeadBattery();

  /* USER CODE BEGIN MspInit 1 */

  /* USER CODE END MspInit 1 */
}

static uint32_t HAL_RCC_ADC12_CLK_ENABLED=0;

/**
  * @brief ADC MSP Initialization
  * This function configures the hardware resources used in this example
  * @param hadc: ADC handle pointer
  * @retval None
  */
void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
  if(hadc->Instance==ADC1)
  {
    /* USER CODE BEGIN ADC1_MspInit 0 */

    /* USER CODE END ADC1_MspInit 0 */

  /** Initializes the peripherals clocks
  */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC12;
    PeriphClkInit.Adc12ClockSelection = RCC_ADC12CLKSOURCE_SYSCLK;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }

    /* Peripheral clock enable */
    HAL_RCC_ADC12_CLK_ENABLED++;
    if(HAL_RCC_ADC12_CLK_ENABLED==1){
      __HAL_RCC_ADC12_CLK_ENABLE();
    }

    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**ADC1 GPIO Configuration
    PC2     ------> ADC1_IN8
    PC3     ------> ADC1_IN9
    PA0     ------> ADC1_IN1
    PA2     ------> ADC1_IN3
    PB1     ------> ADC1_IN12
    */
    GPIO_InitStruct.Pin = Vout1_adc_Pin|Vout2_adc_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = Iout_adc_Pin|S12V_adc_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = TEMP_S3_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(TEMP_S3_GPIO_Port, &GPIO_InitStruct);

    /* ADC1 DMA Init */
    /* ADC1 Init */
    hdma_adc1.Instance = DMA1_Channel1;
    hdma_adc1.Init.Request = DMA_REQUEST_ADC1;
    hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_adc1.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_adc1.Init.MemInc = DMA_MINC_ENABLE;
    hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_adc1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_adc1.Init.Mode = DMA_CIRCULAR;
    hdma_adc1.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    if (HAL_DMA_Init(&hdma_adc1) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(hadc,DMA_Handle,hdma_adc1);

    /* USER CODE BEGIN ADC1_MspInit 1 */

    /* USER CODE END ADC1_MspInit 1 */
  }
  else if(hadc->Instance==ADC2)
  {
    /* USER CODE BEGIN ADC2_MspInit 0 */

    /* USER CODE END ADC2_MspInit 0 */

  /** Initializes the peripherals clocks
  */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC12;
    PeriphClkInit.Adc12ClockSelection = RCC_ADC12CLKSOURCE_SYSCLK;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }

    /* Peripheral clock enable */
    HAL_RCC_ADC12_CLK_ENABLED++;
    if(HAL_RCC_ADC12_CLK_ENABLED==1){
      __HAL_RCC_ADC12_CLK_ENABLE();
    }

    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**ADC2 GPIO Configuration
    PC0     ------> ADC2_IN6
    PA4     ------> ADC2_IN17
    PA5     ------> ADC2_IN13
    PA6     ------> ADC2_IN3
    PA7     ------> ADC2_IN4
    PC4     ------> ADC2_IN5
    PC5     ------> ADC2_IN11
    PB11     ------> ADC2_IN14
    */
    GPIO_InitStruct.Pin = FAN1_ADC_Pin|FAN4_ADC_Pin|FAN3_ADC_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = TEMP_S0_Pin|TEMP_S1_Pin|TEMP_S2_Pin|REF_0V3_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = FAN2_ADC_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(FAN2_ADC_GPIO_Port, &GPIO_InitStruct);

    /* ADC2 DMA Init */
    /* ADC2 Init */
    hdma_adc2.Instance = DMA1_Channel2;
    hdma_adc2.Init.Request = DMA_REQUEST_ADC2;
    hdma_adc2.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_adc2.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_adc2.Init.MemInc = DMA_MINC_ENABLE;
    hdma_adc2.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_adc2.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_adc2.Init.Mode = DMA_CIRCULAR;
    hdma_adc2.Init.Priority = DMA_PRIORITY_LOW;
    if (HAL_DMA_Init(&hdma_adc2) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(hadc,DMA_Handle,hdma_adc2);

    /* USER CODE BEGIN ADC2_MspInit 1 */

    /* USER CODE END ADC2_MspInit 1 */
  }

}

/**
  * @brief ADC MSP De-Initialization
  * This function freeze the hardware resources used in this example
  * @param hadc: ADC handle pointer
  * @retval None
  */
void HAL_ADC_MspDeInit(ADC_HandleTypeDef* hadc)
{
  if(hadc->Instance==ADC1)
  {
    /* USER CODE BEGIN ADC1_MspDeInit 0 */

    /* USER CODE END ADC1_MspDeInit 0 */
    /* Peripheral clock disable */
    HAL_RCC_ADC12_CLK_ENABLED--;
    if(HAL_RCC_ADC12_CLK_ENABLED==0){
      __HAL_RCC_ADC12_CLK_DISABLE();
    }

    /**ADC1 GPIO Configuration
    PC2     ------> ADC1_IN8
    PC3     ------> ADC1_IN9
    PA0     ------> ADC1_IN1
    PA2     ------> ADC1_IN3
    PB1     ------> ADC1_IN12
    */
    HAL_GPIO_DeInit(GPIOC, Vout1_adc_Pin|Vout2_adc_Pin);

    HAL_GPIO_DeInit(GPIOA, Iout_adc_Pin|S12V_adc_Pin);

    HAL_GPIO_DeInit(TEMP_S3_GPIO_Port, TEMP_S3_Pin);

    /* ADC1 DMA DeInit */
    HAL_DMA_DeInit(hadc->DMA_Handle);
    /* USER CODE BEGIN ADC1_MspDeInit 1 */

    /* USER CODE END ADC1_MspDeInit 1 */
  }
  else if(hadc->Instance==ADC2)
  {
    /* USER CODE BEGIN ADC2_MspDeInit 0 */

    /* USER CODE END ADC2_MspDeInit 0 */
    /* Peripheral clock disable */
    HAL_RCC_ADC12_CLK_ENABLED--;
    if(HAL_RCC_ADC12_CLK_ENABLED==0){
      __HAL_RCC_ADC12_CLK_DISABLE();
    }

    /**ADC2 GPIO Configuration
    PC0     ------> ADC2_IN6
    PA4     ------> ADC2_IN17
    PA5     ------> ADC2_IN13
    PA6     ------> ADC2_IN3
    PA7     ------> ADC2_IN4
    PC4     ------> ADC2_IN5
    PC5     ------> ADC2_IN11
    PB11     ------> ADC2_IN14
    */
    HAL_GPIO_DeInit(GPIOC, FAN1_ADC_Pin|FAN4_ADC_Pin|FAN3_ADC_Pin);

    HAL_GPIO_DeInit(GPIOA, TEMP_S0_Pin|TEMP_S1_Pin|TEMP_S2_Pin|REF_0V3_Pin);

    HAL_GPIO_DeInit(FAN2_ADC_GPIO_Port, FAN2_ADC_Pin);

    /* ADC2 DMA DeInit */
    HAL_DMA_DeInit(hadc->DMA_Handle);
    /* USER CODE BEGIN ADC2_MspDeInit 1 */

    /* USER CODE END ADC2_MspDeInit 1 */
  }

}

/**
  * @brief COMP MSP Initialization
  * This function configures the hardware resources used in this example
  * @param hcomp: COMP handle pointer
  * @retval None
  */
void HAL_COMP_MspInit(COMP_HandleTypeDef* hcomp)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(hcomp->Instance==COMP1)
  {
    /* USER CODE BEGIN COMP1_MspInit 0 */

    /* USER CODE END COMP1_MspInit 0 */

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**COMP1 GPIO Configuration
    PA1     ------> COMP1_INP
    */
    GPIO_InitStruct.Pin = Iout_OCP_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(Iout_OCP_GPIO_Port, &GPIO_InitStruct);

    /* USER CODE BEGIN COMP1_MspInit 1 */

    /* USER CODE END COMP1_MspInit 1 */
  }
  else if(hcomp->Instance==COMP2)
  {
    /* USER CODE BEGIN COMP2_MspInit 0 */

    /* USER CODE END COMP2_MspInit 0 */

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**COMP2 GPIO Configuration
    PA3     ------> COMP2_INP
    */
    GPIO_InitStruct.Pin = IR1_OCP_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(IR1_OCP_GPIO_Port, &GPIO_InitStruct);

    /* COMP2 interrupt Init */
//    HAL_NVIC_SetPriority(COMP1_2_3_IRQn, 0, 0);
//    HAL_NVIC_EnableIRQ(COMP1_2_3_IRQn);
    /* USER CODE BEGIN COMP2_MspInit 1 */

    /* USER CODE END COMP2_MspInit 1 */
  }
  else if(hcomp->Instance==COMP3)
  {
    /* USER CODE BEGIN COMP3_MspInit 0 */

    /* USER CODE END COMP3_MspInit 0 */

    __HAL_RCC_GPIOC_CLK_ENABLE();
    /**COMP3 GPIO Configuration
    PC1     ------> COMP3_INP
    */
    GPIO_InitStruct.Pin = Vout_OVP_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(Vout_OVP_GPIO_Port, &GPIO_InitStruct);

    /* USER CODE BEGIN COMP3_MspInit 1 */

    /* USER CODE END COMP3_MspInit 1 */
  }

}

/**
  * @brief COMP MSP De-Initialization
  * This function freeze the hardware resources used in this example
  * @param hcomp: COMP handle pointer
  * @retval None
  */
void HAL_COMP_MspDeInit(COMP_HandleTypeDef* hcomp)
{
  if(hcomp->Instance==COMP1)
  {
    /* USER CODE BEGIN COMP1_MspDeInit 0 */

    /* USER CODE END COMP1_MspDeInit 0 */

    /**COMP1 GPIO Configuration
    PA1     ------> COMP1_INP
    */
    HAL_GPIO_DeInit(Iout_OCP_GPIO_Port, Iout_OCP_Pin);

    /* COMP1 interrupt DeInit */
    /* USER CODE BEGIN COMP1:COMP1_2_3_IRQn disable */
    /**
    * Uncomment the line below to disable the "COMP1_2_3_IRQn" interrupt
    * Be aware, disabling shared interrupt may affect other IPs
    */
    /* HAL_NVIC_DisableIRQ(COMP1_2_3_IRQn); */
    /* USER CODE END COMP1:COMP1_2_3_IRQn disable */

    /* USER CODE BEGIN COMP1_MspDeInit 1 */

    /* USER CODE END COMP1_MspDeInit 1 */
  }
  else if(hcomp->Instance==COMP2)
  {
    /* USER CODE BEGIN COMP2_MspDeInit 0 */

    /* USER CODE END COMP2_MspDeInit 0 */

    /**COMP2 GPIO Configuration
    PA3     ------> COMP2_INP
    */
    HAL_GPIO_DeInit(IR1_OCP_GPIO_Port, IR1_OCP_Pin);

    /* COMP2 interrupt DeInit */
    /* USER CODE BEGIN COMP2:COMP1_2_3_IRQn disable */
    /**
    * Uncomment the line below to disable the "COMP1_2_3_IRQn" interrupt
    * Be aware, disabling shared interrupt may affect other IPs
    */
    /* HAL_NVIC_DisableIRQ(COMP1_2_3_IRQn); */
    /* USER CODE END COMP2:COMP1_2_3_IRQn disable */

    /* USER CODE BEGIN COMP2_MspDeInit 1 */

    /* USER CODE END COMP2_MspDeInit 1 */
  }
  else if(hcomp->Instance==COMP3)
  {
    /* USER CODE BEGIN COMP3_MspDeInit 0 */

    /* USER CODE END COMP3_MspDeInit 0 */

    /**COMP3 GPIO Configuration
    PC1     ------> COMP3_INP
    */
    HAL_GPIO_DeInit(Vout_OVP_GPIO_Port, Vout_OVP_Pin);

    /* COMP3 interrupt DeInit */
    /* USER CODE BEGIN COMP3:COMP1_2_3_IRQn disable */
    /**
    * Uncomment the line below to disable the "COMP1_2_3_IRQn" interrupt
    * Be aware, disabling shared interrupt may affect other IPs
    */
    /* HAL_NVIC_DisableIRQ(COMP1_2_3_IRQn); */
    /* USER CODE END COMP3:COMP1_2_3_IRQn disable */

    /* USER CODE BEGIN COMP3_MspDeInit 1 */

    /* USER CODE END COMP3_MspDeInit 1 */
  }

}

/**
  * @brief DAC MSP Initialization
  * This function configures the hardware resources used in this example
  * @param hdac: DAC handle pointer
  * @retval None
  */
void HAL_DAC_MspInit(DAC_HandleTypeDef* hdac)
{
  if(hdac->Instance==DAC1)
  {
    /* USER CODE BEGIN DAC1_MspInit 0 */

    /* USER CODE END DAC1_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_DAC1_CLK_ENABLE();
    /* USER CODE BEGIN DAC1_MspInit 1 */

    /* USER CODE END DAC1_MspInit 1 */
  }
  else if(hdac->Instance==DAC3)
  {
    /* USER CODE BEGIN DAC3_MspInit 0 */

    /* USER CODE END DAC3_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_DAC3_CLK_ENABLE();
    /* USER CODE BEGIN DAC3_MspInit 1 */

    /* USER CODE END DAC3_MspInit 1 */
  }

}

/**
  * @brief DAC MSP De-Initialization
  * This function freeze the hardware resources used in this example
  * @param hdac: DAC handle pointer
  * @retval None
  */
void HAL_DAC_MspDeInit(DAC_HandleTypeDef* hdac)
{
  if(hdac->Instance==DAC1)
  {
    /* USER CODE BEGIN DAC1_MspDeInit 0 */

    /* USER CODE END DAC1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_DAC1_CLK_DISABLE();
    /* USER CODE BEGIN DAC1_MspDeInit 1 */

    /* USER CODE END DAC1_MspDeInit 1 */
  }
  else if(hdac->Instance==DAC3)
  {
    /* USER CODE BEGIN DAC3_MspDeInit 0 */

    /* USER CODE END DAC3_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_DAC3_CLK_DISABLE();
    /* USER CODE BEGIN DAC3_MspDeInit 1 */

    /* USER CODE END DAC3_MspDeInit 1 */
  }

}

/**
  * @brief FDCAN MSP Initialization
  * This function configures the hardware resources used in this example
  * @param hfdcan: FDCAN handle pointer
  * @retval None
  */
void HAL_FDCAN_MspInit(FDCAN_HandleTypeDef* hfdcan)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
  if(hfdcan->Instance==FDCAN1)
  {
    /* USER CODE BEGIN FDCAN1_MspInit 0 */

    /* USER CODE END FDCAN1_MspInit 0 */

  /** Initializes the peripherals clocks
  */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_FDCAN;
    PeriphClkInit.FdcanClockSelection = RCC_FDCANCLKSOURCE_PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }

    /* Peripheral clock enable */
    __HAL_RCC_FDCAN_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**FDCAN1 GPIO Configuration
    PA11     ------> FDCAN1_RX
    PA12     ------> FDCAN1_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF9_FDCAN1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* FDCAN1 interrupt Init */
    HAL_NVIC_SetPriority(FDCAN1_IT0_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(FDCAN1_IT0_IRQn);
    /* USER CODE BEGIN FDCAN1_MspInit 1 */

    /* USER CODE END FDCAN1_MspInit 1 */

  }

}

/**
  * @brief FDCAN MSP De-Initialization
  * This function freeze the hardware resources used in this example
  * @param hfdcan: FDCAN handle pointer
  * @retval None
  */
void HAL_FDCAN_MspDeInit(FDCAN_HandleTypeDef* hfdcan)
{
  if(hfdcan->Instance==FDCAN1)
  {
    /* USER CODE BEGIN FDCAN1_MspDeInit 0 */

    /* USER CODE END FDCAN1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_FDCAN_CLK_DISABLE();

    /**FDCAN1 GPIO Configuration
    PA11     ------> FDCAN1_RX
    PA12     ------> FDCAN1_TX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_11|GPIO_PIN_12);

    /* FDCAN1 interrupt DeInit */
    HAL_NVIC_DisableIRQ(FDCAN1_IT0_IRQn);
    /* USER CODE BEGIN FDCAN1_MspDeInit 1 */

    /* USER CODE END FDCAN1_MspDeInit 1 */
  }

}

/**
  * @brief HRTIM MSP Initialization
  * This function configures the hardware resources used in this example
  * @param hhrtim: HRTIM handle pointer
  * @retval None
  */
void HAL_HRTIM_MspInit(HRTIM_HandleTypeDef* hhrtim)
{
  if(hhrtim->Instance==HRTIM1)
  {
    /* USER CODE BEGIN HRTIM1_MspInit 0 */

    /* USER CODE END HRTIM1_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_HRTIM1_CLK_ENABLE();
    /* HRTIM1 interrupt Init */
    HAL_NVIC_SetPriority(HRTIM1_FLT_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(HRTIM1_FLT_IRQn);
    /* USER CODE BEGIN HRTIM1_MspInit 1 */
    hhrtim1.Instance->sCommonRegs.IER |= HRTIM_IT_FLT4 | HRTIM_IT_FLT5 | HRTIM_IT_FLT6;
    /* USER CODE END HRTIM1_MspInit 1 */

  }

}

void HAL_HRTIM_MspPostInit(HRTIM_HandleTypeDef* hhrtim)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(hhrtim->Instance==HRTIM1)
  {
    /* USER CODE BEGIN HRTIM1_MspPostInit 0 */

    /* USER CODE END HRTIM1_MspPostInit 0 */

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**HRTIM1 GPIO Configuration
    PB12     ------> HRTIM1_CHC1
    PB13     ------> HRTIM1_CHC2
    PB14     ------> HRTIM1_CHD1
    PB15     ------> HRTIM1_CHD2
    */
    GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF13_HRTIM1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* USER CODE BEGIN HRTIM1_MspPostInit 1 */

    /* USER CODE END HRTIM1_MspPostInit 1 */
  }

}
/**
  * @brief HRTIM MSP De-Initialization
  * This function freeze the hardware resources used in this example
  * @param hhrtim: HRTIM handle pointer
  * @retval None
  */
void HAL_HRTIM_MspDeInit(HRTIM_HandleTypeDef* hhrtim)
{
  if(hhrtim->Instance==HRTIM1)
  {
    /* USER CODE BEGIN HRTIM1_MspDeInit 0 */

    /* USER CODE END HRTIM1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_HRTIM1_CLK_DISABLE();

    /* HRTIM1 interrupt DeInit */
    HAL_NVIC_DisableIRQ(HRTIM1_FLT_IRQn);
    /* USER CODE BEGIN HRTIM1_MspDeInit 1 */

    /* USER CODE END HRTIM1_MspDeInit 1 */
  }

}

/**
  * @brief SPI MSP Initialization
  * This function configures the hardware resources used in this example
  * @param hspi: SPI handle pointer
  * @retval None
  */
void HAL_SPI_MspInit(SPI_HandleTypeDef* hspi)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(hspi->Instance==SPI1)
  {
    /* USER CODE BEGIN SPI1_MspInit 0 */

    /* USER CODE END SPI1_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_SPI1_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**SPI1 GPIO Configuration
    PB3     ------> SPI1_SCK
    PB4     ------> SPI1_MISO
    PB5     ------> SPI1_MOSI
    */
    GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* USER CODE BEGIN SPI1_MspInit 1 */

    /* USER CODE END SPI1_MspInit 1 */

  }

}

/**
  * @brief SPI MSP De-Initialization
  * This function freeze the hardware resources used in this example
  * @param hspi: SPI handle pointer
  * @retval None
  */
void HAL_SPI_MspDeInit(SPI_HandleTypeDef* hspi)
{
  if(hspi->Instance==SPI1)
  {
    /* USER CODE BEGIN SPI1_MspDeInit 0 */

    /* USER CODE END SPI1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_SPI1_CLK_DISABLE();

    /**SPI1 GPIO Configuration
    PB3     ------> SPI1_SCK
    PB4     ------> SPI1_MISO
    PB5     ------> SPI1_MOSI
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5);

    /* USER CODE BEGIN SPI1_MspDeInit 1 */

    /* USER CODE END SPI1_MspDeInit 1 */
  }

}

/**
  * @brief TIM_Base MSP Initialization
  * This function configures the hardware resources used in this example
  * @param htim_base: TIM_Base handle pointer
  * @retval None
  */
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim_base)
{
  if(htim_base->Instance==TIM1)
  {
    /* USER CODE BEGIN TIM1_MspInit 0 */

    /* USER CODE END TIM1_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_TIM1_CLK_ENABLE();
    /* USER CODE BEGIN TIM1_MspInit 1 */

    /* USER CODE END TIM1_MspInit 1 */
  }
  else if(htim_base->Instance==TIM3)
  {
    /* USER CODE BEGIN TIM3_MspInit 0 */

    /* USER CODE END TIM3_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_TIM3_CLK_ENABLE();
    /* USER CODE BEGIN TIM3_MspInit 1 */

    /* USER CODE END TIM3_MspInit 1 */
  }
  else if(htim_base->Instance==TIM4)
  {
    /* USER CODE BEGIN TIM4_MspInit 0 */

    /* USER CODE END TIM4_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_TIM4_CLK_ENABLE();
    /* USER CODE BEGIN TIM4_MspInit 1 */

    /* USER CODE END TIM4_MspInit 1 */
  }
  else if(htim_base->Instance==TIM15)
  {
    /* USER CODE BEGIN TIM15_MspInit 0 */

    /* USER CODE END TIM15_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_TIM15_CLK_ENABLE();
    /* USER CODE BEGIN TIM15_MspInit 1 */

    /* USER CODE END TIM15_MspInit 1 */
  }

}

void HAL_TIM_MspPostInit(TIM_HandleTypeDef* htim)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(htim->Instance==TIM1)
  {
    /* USER CODE BEGIN TIM1_MspPostInit 0 */

    /* USER CODE END TIM1_MspPostInit 0 */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**TIM1 GPIO Configuration
    PA8     ------> TIM1_CH1
    */
    GPIO_InitStruct.Pin = FAN4_OUT_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF6_TIM1;
    HAL_GPIO_Init(FAN4_OUT_GPIO_Port, &GPIO_InitStruct);

    /* USER CODE BEGIN TIM1_MspPostInit 1 */

    /* USER CODE END TIM1_MspPostInit 1 */
  }
  else if(htim->Instance==TIM4)
  {
    /* USER CODE BEGIN TIM4_MspPostInit 0 */

    /* USER CODE END TIM4_MspPostInit 0 */

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**TIM4 GPIO Configuration
    PB7     ------> TIM4_CH2
    PB8-BOOT0     ------> TIM4_CH3
    PB9     ------> TIM4_CH4
    */
    GPIO_InitStruct.Pin = FAN3_OUT_Pin|FAN2_OUT_Pin|FAN1_OUT_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM4;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* USER CODE BEGIN TIM4_MspPostInit 1 */

    /* USER CODE END TIM4_MspPostInit 1 */
  }

}
/**
  * @brief TIM_Base MSP De-Initialization
  * This function freeze the hardware resources used in this example
  * @param htim_base: TIM_Base handle pointer
  * @retval None
  */
void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* htim_base)
{
  if(htim_base->Instance==TIM1)
  {
    /* USER CODE BEGIN TIM1_MspDeInit 0 */

    /* USER CODE END TIM1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_TIM1_CLK_DISABLE();
    /* USER CODE BEGIN TIM1_MspDeInit 1 */

    /* USER CODE END TIM1_MspDeInit 1 */
  }
  else if(htim_base->Instance==TIM3)
  {
    /* USER CODE BEGIN TIM3_MspDeInit 0 */

    /* USER CODE END TIM3_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_TIM3_CLK_DISABLE();
    /* USER CODE BEGIN TIM3_MspDeInit 1 */

    /* USER CODE END TIM3_MspDeInit 1 */
  }
  else if(htim_base->Instance==TIM4)
  {
    /* USER CODE BEGIN TIM4_MspDeInit 0 */

    /* USER CODE END TIM4_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_TIM4_CLK_DISABLE();
    /* USER CODE BEGIN TIM4_MspDeInit 1 */

    /* USER CODE END TIM4_MspDeInit 1 */
  }
  else if(htim_base->Instance==TIM15)
  {
    /* USER CODE BEGIN TIM15_MspDeInit 0 */

    /* USER CODE END TIM15_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_TIM15_CLK_DISABLE();
    /* USER CODE BEGIN TIM15_MspDeInit 1 */

    /* USER CODE END TIM15_MspDeInit 1 */
  }

}

/**
  * @brief UART MSP Initialization
  * This function configures the hardware resources used in this example
  * @param huart: UART handle pointer
  * @retval None
  */
void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
  if(huart->Instance==USART3)
  {
    /* USER CODE BEGIN USART3_MspInit 0 */

    /* USER CODE END USART3_MspInit 0 */

  /** Initializes the peripherals clocks
  */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART3;
    PeriphClkInit.Usart3ClockSelection = RCC_USART3CLKSOURCE_PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }

    /* Peripheral clock enable */
    __HAL_RCC_USART3_CLK_ENABLE();

    __HAL_RCC_GPIOC_CLK_ENABLE();
    /**USART3 GPIO Configuration
    PC10     ------> USART3_TX
    PC11     ------> USART3_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* USART3 DMA Init */
    /* USART3_RX Init */
    hdma_usart3_rx.Instance = DMA1_Channel3;
    hdma_usart3_rx.Init.Request = DMA_REQUEST_USART3_RX;
    hdma_usart3_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_usart3_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart3_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart3_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart3_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart3_rx.Init.Mode = DMA_NORMAL;
    hdma_usart3_rx.Init.Priority = DMA_PRIORITY_LOW;
    if (HAL_DMA_Init(&hdma_usart3_rx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(huart,hdmarx,hdma_usart3_rx);

    /* USART3_TX Init */
    hdma_usart3_tx.Instance = DMA1_Channel4;
    hdma_usart3_tx.Init.Request = DMA_REQUEST_USART3_TX;
    hdma_usart3_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_usart3_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart3_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart3_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart3_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart3_tx.Init.Mode = DMA_NORMAL;
    hdma_usart3_tx.Init.Priority = DMA_PRIORITY_LOW;
    if (HAL_DMA_Init(&hdma_usart3_tx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(huart,hdmatx,hdma_usart3_tx);

    /* USART3 interrupt Init */
    HAL_NVIC_SetPriority(USART3_IRQn, 4, 0);
    HAL_NVIC_EnableIRQ(USART3_IRQn);
    /* USER CODE BEGIN USART3_MspInit 1 */

    /* USER CODE END USART3_MspInit 1 */

  }

}

/**
  * @brief UART MSP De-Initialization
  * This function freeze the hardware resources used in this example
  * @param huart: UART handle pointer
  * @retval None
  */
void HAL_UART_MspDeInit(UART_HandleTypeDef* huart)
{
  if(huart->Instance==USART3)
  {
    /* USER CODE BEGIN USART3_MspDeInit 0 */

    /* USER CODE END USART3_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART3_CLK_DISABLE();

    /**USART3 GPIO Configuration
    PC10     ------> USART3_TX
    PC11     ------> USART3_RX
    */
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_10|GPIO_PIN_11);

    /* USART3 DMA DeInit */
    HAL_DMA_DeInit(huart->hdmarx);
    HAL_DMA_DeInit(huart->hdmatx);

    /* USART3 interrupt DeInit */
    HAL_NVIC_DisableIRQ(USART3_IRQn);
    /* USER CODE BEGIN USART3_MspDeInit 1 */

    /* USER CODE END USART3_MspDeInit 1 */
  }

}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
