/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32g4xx_it.c
  * @brief   Interrupt Service Routines.
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
#include "stm32g4xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "HwConfig.h"
#include "MODBUS_SLAVE.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern DMA_HandleTypeDef hdma_adc1;
extern DMA_HandleTypeDef hdma_adc2;
extern COMP_HandleTypeDef hcomp1;
extern COMP_HandleTypeDef hcomp2;
extern COMP_HandleTypeDef hcomp3;
extern FDCAN_HandleTypeDef hfdcan1;
extern HRTIM_HandleTypeDef hhrtim1;
extern UART_HandleTypeDef huart3;
extern DMA_HandleTypeDef hdma_usart3_rx;
extern DMA_HandleTypeDef hdma_usart3_tx;
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M4 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
   while (1)
  {
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */

  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
  * @brief This function handles Prefetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */

  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */

  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
  /* USER CODE BEGIN SVCall_IRQn 0 */

  /* USER CODE END SVCall_IRQn 0 */
  /* USER CODE BEGIN SVCall_IRQn 1 */

  /* USER CODE END SVCall_IRQn 1 */
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
  /* USER CODE BEGIN PendSV_IRQn 0 */

  /* USER CODE END PendSV_IRQn 0 */
  /* USER CODE BEGIN PendSV_IRQn 1 */

  /* USER CODE END PendSV_IRQn 1 */
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  /* USER CODE BEGIN SysTick_IRQn 1 */

  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32G4xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32g4xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles DMA1 channel1 global interrupt.
  */
void DMA1_Channel1_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Channel1_IRQn 0 */
  uint32_t flag_it = hdma_adc1.DmaBaseAddress->ISR;
  if (0U != (flag_it & ((uint32_t)DMA_FLAG_TC1 << (hdma_adc1.ChannelIndex & 0x1FU))))           
  {
    if((hdma_adc1.Instance->CCR & DMA_CCR_CIRC) == 0U)
    {
      /* Disable the transfer complete and error interrupt */
      __HAL_DMA_DISABLE_IT(&hdma_adc1, DMA_IT_TE | DMA_IT_TC);

      /* Change the DMA state */
      hdma_adc1.State = HAL_DMA_STATE_READY;
    }
    /* Clear the transfer complete flag */
    hdma_adc1.DmaBaseAddress->IFCR = ((uint32_t)DMA_ISR_TCIF1 << (hdma_adc1.ChannelIndex & 0x1FU));
//    HAL_ADC_Stop_DMA(&hadc1);

		interrupt_ADC1();			
//		HAL_ADC_Start_DMA(&hadc1,(uint32_t *)ADC0_Buffer,4);
  }  
	#ifdef PINGBI
  /* USER CODE END DMA1_Channel1_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_adc1);
  /* USER CODE BEGIN DMA1_Channel1_IRQn 1 */
  #endif
  /* USER CODE END DMA1_Channel1_IRQn 1 */
}

/**
  * @brief This function handles DMA1 channel2 global interrupt.
  */
void DMA1_Channel2_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Channel2_IRQn 0 */
  uint32_t flag_it = hdma_adc2.DmaBaseAddress->ISR;
  if (0U != (flag_it & ((uint32_t)DMA_FLAG_TC1 << (hdma_adc2.ChannelIndex & 0x1FU))))           
  {
    if ((hdma_adc2.Instance->CCR & DMA_CCR_CIRC) == 0U)
    {
      /* Disable the transfer complete and error interrupt */
      __HAL_DMA_DISABLE_IT(&hdma_adc2, DMA_IT_TE | DMA_IT_TC);

      /* Change the DMA state */
      hdma_adc2.State = HAL_DMA_STATE_READY;
    }
    /* Clear the transfer complete flag */
    hdma_adc2.DmaBaseAddress->IFCR = ((uint32_t)DMA_ISR_TCIF1 << (hdma_adc2.ChannelIndex & 0x1FU));
//    HAL_ADC_Stop_DMA(&hadc2);
		interrupt_ADC2();	
//		HAL_ADC_Start_DMA(&hadc2,(uint32_t *)ADC2_Buffer,4);
  }  
	
  #ifdef PINGBI
  /* USER CODE END DMA1_Channel2_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_adc2);
  /* USER CODE BEGIN DMA1_Channel2_IRQn 1 */
  #endif
  /* USER CODE END DMA1_Channel2_IRQn 1 */
}

/**
  * @brief This function handles FDCAN1 interrupt 0.
  */
void FDCAN1_IT0_IRQHandler(void)
{
  /* USER CODE BEGIN FDCAN1_IT0_IRQn 0 */

  /* USER CODE END FDCAN1_IT0_IRQn 0 */
  HAL_FDCAN_IRQHandler(&hfdcan1);
  /* USER CODE BEGIN FDCAN1_IT0_IRQn 1 */

  /* USER CODE END FDCAN1_IT0_IRQn 1 */
}



/**
  * @brief This function handles DMA1 channel1 global interrupt.
  */
void DMA1_Channel3_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Channel1_IRQn 0 */

  /* USER CODE END DMA1_Channel1_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_usart3_rx);
  /* USER CODE BEGIN DMA1_Channel1_IRQn 1 */

  /* USER CODE END DMA1_Channel1_IRQn 1 */
}

/**
  * @brief This function handles DMA1 channel2 global interrupt.
  */
void DMA1_Channel4_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Channel2_IRQn 0 */

  /* USER CODE END DMA1_Channel2_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_usart3_tx);
  /* USER CODE BEGIN DMA1_Channel2_IRQn 1 */

  /* USER CODE END DMA1_Channel2_IRQn 1 */
}

/**
  * @brief This function handles USART3 global interrupt / USART3 wake-up interrupt through EXTI line 28.
  */
void USART3_IRQHandler(void)
{
  /* USER CODE BEGIN USART3_IRQn 0 */
	uint32_t tmp_flag = 0;
  uint32_t temp,rx_len;
	temp = USART3->RDR;	
  tmp_flag = __HAL_UART_GET_FLAG(&huart3, UART_FLAG_IDLE); 
  if ((tmp_flag != RESET))                                 
  {
    __HAL_UART_CLEAR_IDLEFLAG(&huart3); //    HAL_UART_DMAStop(&huart3);   
		__HAL_UART_CLEAR_OREFLAG(&huart3);   		
		temp = hdma_usart3_rx.Instance->CNDTR;
		huart3.Instance->CR3 &= 0xFFFFFFBF;	 
		__HAL_DMA_DISABLE(&hdma_usart3_rx);
    rx_len = PFC_LLC_RX_DMA_LEN - temp; 
    if(rx_len >= 10 ){
		  Modbus_Slave_Rx(Rxbuffer_usat3,rx_len);
	  }
		hdma_usart3_rx.Instance->CNDTR = PFC_LLC_RX_DMA_LEN;
		hdma_usart3_rx.Instance->CPAR = (uint32_t)&huart3.Instance->RDR;
		hdma_usart3_rx.Instance->CMAR = (uint32_t)&Rxbuffer_usat3;
	  __HAL_DMA_ENABLE(&hdma_usart3_rx);
		//HAL_DMA_Start(&hdma_usart3_rx, (uint32_t)&huart3.Instance->RDR,(uint32_t)&Rxbuffer_usat3, BUFFER_SIZE);
		huart3.Instance->CR3 |= 0x40;			
  }
  
//  tmp_flag = __HAL_UART_GET_FLAG(&huart3, UART_FLAG_TC); 
//  if ((tmp_flag != RESET))                                 
//  {
//    __HAL_UART_CLEAR_FLAG(&huart3,UART_CLEAR_TCF);
//  }
//  HAL_UART_IRQHandler(&huart3);
	#ifdef PINGBI
  /* USER CODE END USART3_IRQn 0 */
  HAL_UART_IRQHandler(&huart3);
  /* USER CODE BEGIN USART3_IRQn 1 */
  #endif
  /* USER CODE END USART3_IRQn 1 */
}

/**
  * @brief This function handles COMP1, COMP2 and COMP3 interrupts through EXTI lines 21, 22 and 29.
  */
void COMP1_2_3_IRQHandler(void)
{
  /* USER CODE BEGIN COMP1_2_3_IRQn 0 */
//  HAL_COMP_SR();
  #ifdef PINGBI	
  /* USER CODE END COMP1_2_3_IRQn 0 */
  HAL_COMP_IRQHandler(&hcomp1);
  HAL_COMP_IRQHandler(&hcomp2);
  HAL_COMP_IRQHandler(&hcomp3);
  /* USER CODE BEGIN COMP1_2_3_IRQn 1 */
  #endif
  /* USER CODE END COMP1_2_3_IRQn 1 */
}

/**
  * @brief This function handles HRTIM fault global interrupt.
  */
void HRTIM1_FLT_IRQHandler(void)
{
  /* USER CODE BEGIN HRTIM1_FLT_IRQn 0 */
  uint32_t isrflags = READ_REG(hhrtim1.Instance->sCommonRegs.ISR);
  if((uint32_t)(isrflags & HRTIM_FLAG_FLT4) != (uint32_t)RESET)
  {
     __HAL_HRTIM_CLEAR_IT(&hhrtim1, HRTIM_IT_FLT4);
		if(DataFlowFace.RelayOld)
		{
			DataFlowFace.FaultSta.bit.Out_oc = 1;	
	  }
  }
  if((uint32_t)(isrflags & HRTIM_FLAG_FLT5) != (uint32_t)RESET)
  {
     __HAL_HRTIM_CLEAR_IT(&hhrtim1, HRTIM_IT_FLT5);
		DataFlowFace.FaultSta.bit.InterOv = 1;	
  }
  if((uint32_t)(isrflags & HRTIM_FLAG_FLT6) != (uint32_t)RESET)
  {
     __HAL_HRTIM_CLEAR_IT(&hhrtim1, HRTIM_IT_FLT6);
		
		DataFlowFace.FaultSta.bit.ResonOc = 1;	
  }	
	
	if(DataFlowFace.FaultSta.all)
	{
		SarH_Off();
		SarL_Off();
		DrvH_Off();
		DrvL_Off();
		DischargeOn();
		LLC_Disable();
		PwmClose();
		RelayOff();	
    	DriverPwm.Plv = POW_MAX_PLV;
		DriverPwm.Sr_Dtime = 0;
		Ctrl_interFace.Curr_REF = 0;
	}else
	{
		HAL_HRTIM_WaveformOutputStart(&hhrtim1,HRTIM_OUTPUT_TC1 | HRTIM_OUTPUT_TC2 | HRTIM_OUTPUT_TD1 | HRTIM_OUTPUT_TD2);	
		HAL_HRTIM_WaveformCountStart(&hhrtim1, HRTIM_TIMERID_TIMER_C);
		HAL_HRTIM_WaveformCountStart(&hhrtim1, HRTIM_TIMERID_TIMER_D);		
	}
  #ifdef PINGBI	
  /* USER CODE END HRTIM1_FLT_IRQn 0 */
  HAL_HRTIM_IRQHandler(&hhrtim1,HRTIM_TIMERINDEX_COMMON);
  /* USER CODE BEGIN HRTIM1_FLT_IRQn 1 */
  #endif
  /* USER CODE END HRTIM1_FLT_IRQn 1 */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
