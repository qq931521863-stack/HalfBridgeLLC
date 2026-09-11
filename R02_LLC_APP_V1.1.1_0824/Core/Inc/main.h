/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32g4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_HRTIM_MspPostInit(HRTIM_HandleTypeDef *hhrtim);

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define VOUT_FD_Pin GPIO_PIN_13
#define VOUT_FD_GPIO_Port GPIOC
#define FAN1_ADC_Pin GPIO_PIN_0
#define FAN1_ADC_GPIO_Port GPIOC
#define Vout_OVP_Pin GPIO_PIN_1
#define Vout_OVP_GPIO_Port GPIOC
#define Vout1_adc_Pin GPIO_PIN_2
#define Vout1_adc_GPIO_Port GPIOC
#define Vout2_adc_Pin GPIO_PIN_3
#define Vout2_adc_GPIO_Port GPIOC
#define Iout_adc_Pin GPIO_PIN_0
#define Iout_adc_GPIO_Port GPIOA
#define Iout_OCP_Pin GPIO_PIN_1
#define Iout_OCP_GPIO_Port GPIOA
#define S12V_adc_Pin GPIO_PIN_2
#define S12V_adc_GPIO_Port GPIOA
#define IR1_OCP_Pin GPIO_PIN_3
#define IR1_OCP_GPIO_Port GPIOA
#define TEMP_S0_Pin GPIO_PIN_4
#define TEMP_S0_GPIO_Port GPIOA
#define TEMP_S1_Pin GPIO_PIN_5
#define TEMP_S1_GPIO_Port GPIOA
#define TEMP_S2_Pin GPIO_PIN_6
#define TEMP_S2_GPIO_Port GPIOA
#define REF_0V3_Pin GPIO_PIN_7
#define REF_0V3_GPIO_Port GPIOA
#define FAN4_ADC_Pin GPIO_PIN_4
#define FAN4_ADC_GPIO_Port GPIOC
#define FAN3_ADC_Pin GPIO_PIN_5
#define FAN3_ADC_GPIO_Port GPIOC
#define TEMP_S3_Pin GPIO_PIN_1
#define TEMP_S3_GPIO_Port GPIOB
#define RELAY_CTAL_Pin GPIO_PIN_2
#define RELAY_CTAL_GPIO_Port GPIOB
#define FAN2_ADC_Pin GPIO_PIN_11
#define FAN2_ADC_GPIO_Port GPIOB
#define LED_OUT2_Pin GPIO_PIN_6
#define LED_OUT2_GPIO_Port GPIOC
#define LED_OUT1_Pin GPIO_PIN_7
#define LED_OUT1_GPIO_Port GPIOC
#define FAN4_OUT_Pin GPIO_PIN_8
#define FAN4_OUT_GPIO_Port GPIOA
#define LED_OUT3_Pin GPIO_PIN_15
#define LED_OUT3_GPIO_Port GPIOA
#define PFC_OK_Pin GPIO_PIN_12
#define PFC_OK_GPIO_Port GPIOC
#define LLC_OUT_Pin GPIO_PIN_2
#define LLC_OUT_GPIO_Port GPIOD
#define SPI1_CS_Pin GPIO_PIN_6
#define SPI1_CS_GPIO_Port GPIOB
#define FAN3_OUT_Pin GPIO_PIN_7
#define FAN3_OUT_GPIO_Port GPIOB
#define FAN2_OUT_Pin GPIO_PIN_8
#define FAN2_OUT_GPIO_Port GPIOB
#define FAN1_OUT_Pin GPIO_PIN_9
#define FAN1_OUT_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;
extern DMA_HandleTypeDef hdma_adc1;
extern DMA_HandleTypeDef hdma_adc2;

extern COMP_HandleTypeDef hcomp1;
extern COMP_HandleTypeDef hcomp2;
extern COMP_HandleTypeDef hcomp3;

extern DAC_HandleTypeDef hdac1;
extern DAC_HandleTypeDef hdac3;

extern FDCAN_HandleTypeDef hfdcan1;

extern HRTIM_HandleTypeDef hhrtim1;

extern SPI_HandleTypeDef hspi1;

extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim15;

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart3;

extern DMA_HandleTypeDef hdma_usart3_rx;
extern DMA_HandleTypeDef hdma_usart3_tx;
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
