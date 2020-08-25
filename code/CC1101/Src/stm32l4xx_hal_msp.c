/**
  ******************************************************************************
  * @file    stm32l4xx_hal_msp.c
  * @author  MCD Application Team
  * @brief   msp file for HAL
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2018 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @brief This function configures the source of the time base.
  * @brief  don't enable systick
  * @param TickPriority: Tick interrupt priority.
  * @retval HAL status
  */
//HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority)
//{
//  /* Return function status */
//  return HAL_OK;
//}

/**
  * @brief This function provides delay (in ms)
  * @param Delay: specifies the delay time length, in milliseconds.
  * @retval None
  */
//void HAL_Delay(__IO uint32_t Delay)
//{
//  HW_RTC_DelayMs(Delay);   /* based on RTC */
//}

/**
  * @brief  Initializes the MSP.
  * @retval None
  */
void HAL_MspInit(void)
{
  /* Enable Power Clock */
//  __HAL_RCC_PWR_CLK_ENABLE();

  /* Ensure that MSI is wake-up system clock */
//  __HAL_RCC_WAKEUPSTOP_CLK_CONFIG(RCC_STOP_WAKEUPCLOCK_MSI);

  /* Configure all IOs in analog input              */
  /* Except PA143 and PA14 (SWCLK and SWD) for debug*/
  /* PA13 and PA14 are configured in debug_init     */
//  HW_GpioInit();
}




/**
  * @brief RTC MSP De-Initialization
  *        This function freeze the hardware resources used in this example:
  *          - Disable the Peripheral's clock
  * @param hrtc: RTC handle pointer
  * @retval None
  */
void HAL_RTC_MspDeInit(RTC_HandleTypeDef *hrtc)
{
  /* Reset peripherals */
  __HAL_RCC_RTC_DISABLE();
}


/**
  * @brief  Alarm A callback.
  * @param  hrtc: RTC handle
  * @retval None
  */
//void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc)
//{
//  TimerIrqHandler();
//}

/**
  * @brief  EXTI line detection callbacks.
  * @param  GPIO_Pin: Specifies the pins connected to the EXTI line.
  * @retval None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  
}

/**
  * @brief  Gets IRQ number as a function of the GPIO_Pin.
  * @param  GPIO_Pin: Specifies the pins connected to the EXTI line.
  * @retval IRQ number
  */
IRQn_Type MSP_GetIRQn(uint16_t GPIO_Pin)
{

}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
