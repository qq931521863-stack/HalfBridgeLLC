#include "UpgradeFeatureConfig.h"
#if OTA_UPGRADE_ENABLE

#include "HostX1Gate.h"
#include "main.h"
extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim15;
static void Ota_StopSysTick(void)
{
    HAL_SuspendTick();
    SysTick->CTRL = 0U;
    SysTick->LOAD = 0U;
    SysTick->VAL = 0U;
}

static void Ota_StopTim(void)
{
    (void)HAL_TIM_Base_Stop(&htim1);
    (void)HAL_TIM_Base_Stop(&htim3);
    (void)HAL_TIM_Base_Stop(&htim4);
    (void)HAL_TIM_Base_Stop(&htim15);
}

static void Ota_StopUart(void)
{
    (void)HAL_UART_DMAStop(&huart3);
    __HAL_DMA_DISABLE(&hdma_usart3_rx);
    __HAL_DMA_DISABLE(&hdma_usart3_tx);
    (void)HAL_UART_DeInit(&huart3);
}

static void Ota_StopSpi(void)
{
    (void)HAL_SPI_DeInit(&hspi1);
    __HAL_RCC_SPI1_CLK_DISABLE();
}

static void Ota_StopAdcPwm(void)
{
    (void)HAL_ADC_Stop_DMA(&hadc1);
    (void)HAL_ADC_Stop_DMA(&hadc2);
    (void)HAL_HRTIM_WaveformOutputStop(&hhrtim1,
        HRTIM_OUTPUT_TC1 | HRTIM_OUTPUT_TC2 | HRTIM_OUTPUT_TD1 | HRTIM_OUTPUT_TD2);
    (void)HAL_HRTIM_WaveformCountStop(&hhrtim1, HRTIM_TIMERID_TIMER_C);
    (void)HAL_HRTIM_WaveformCountStop(&hhrtim1, HRTIM_TIMERID_TIMER_D);
    (void)HAL_HRTIM_WaveformCountStop(&hhrtim1, HRTIM_TIMERID_MASTER);
}

static void Ota_StopCan(void)
{
    (void)HAL_FDCAN_DeactivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE);
    (void)HAL_FDCAN_Stop(&hfdcan1);
    (void)HAL_FDCAN_DeInit(&hfdcan1);
    HAL_NVIC_DisableIRQ(FDCAN1_IT0_IRQn);
}

static void Ota_DisableAllNvic(void)
{
    uint32_t i;

    for (i = 0U; i < 8U; i++)
    {
        NVIC->ICER[i] = 0xFFFFFFFFU;
        NVIC->ICPR[i] = 0xFFFFFFFFU;
    }
}

void HostX1Gate_PlatformShutdown(void)
{
    Ota_StopSysTick();
    Ota_StopAdcPwm();
    Ota_StopTim();
    Ota_StopCan();
    Ota_StopUart();
    Ota_StopSpi();
    Ota_DisableAllNvic();
    __disable_irq();
}

#endif /* OTA_UPGRADE_ENABLE */
