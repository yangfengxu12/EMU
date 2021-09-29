/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: Target board general functions implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
/**
  ******************************************************************************
  * @file    stm32l4xx_hw.c
  * @author  MCD Application Team
  * @brief   system hardware driver
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

#include "hw.h"
#include "radio.h"
#include "debug.h"
#include "vcom.h"

/*!
 * Unique Devices IDs register set ( STM32L4xxx )
 */
#define         ID1                                 ( 0x1FFF7590 )
#define         ID2                                 ( 0x1FFF7594 )
#define         ID3                                 ( 0x1FFF7598 )

/* Internal voltage reference, parameter VREFINT_CAL*/
#define VREFINT_CAL       ((uint16_t*) ((uint32_t) 0x1FFF75AA))


/* Internal temperature sensor: constants data used for indicative values in  */
/* this example. Refer to device datasheet for min/typ/max values.            */

/* Internal temperature sensor, parameter TS_CAL1: TS ADC raw data acquired at
 *a temperature of 110 DegC (+-5 DegC), VDDA = 3.3 V (+-10 mV). */
#define TEMP30_CAL_ADDR   ((uint16_t*) ((uint32_t) 0x1FFF75A8))

/* Internal temperature sensor, parameter TS_CAL2: TS ADC raw data acquired at
 *a temperature of  30 DegC (+-5 DegC), VDDA = 3.3 V (+-10 mV). */
#define TEMP110_CAL_ADDR  ((uint16_t*) ((uint32_t) 0x1FFF75CA))

/* Vdda value with which temperature sensor has been calibrated in production
   (+-10 mV). */
#define VDDA_TEMP_CAL                  ((uint32_t) 3000)


#define COMPUTE_TEMPERATURE(TS_ADC_DATA, VDDA_APPLI)                           \
  ((((( ((int32_t)((TS_ADC_DATA * VDDA_APPLI) / VDDA_TEMP_CAL)                  \
        - (int32_t) *TEMP30_CAL_ADDR)                                          \
     ) * (int32_t)(110 - 30)                                                   \
    )<<8) / (int32_t)(*TEMP110_CAL_ADDR - *TEMP30_CAL_ADDR)                        \
   ) + (30<<8)                                                                      \
  )

static ADC_HandleTypeDef hadc;
/*!
 * Flag to indicate if the ADC is Initialized
 */
static bool AdcInitialized = false;

/*!
 * Flag to indicate if the MCU is Initialized
 */
static bool McuInitialized = false;

/**
  * @brief This function initializes the hardware
  * @param None
  * @retval None
  */
void HW_Init(void)
{
  if (McuInitialized == false)
  {
#if defined( USE_BOOTLOADER )
    // Set the Vector Table base location at 0x3000
    NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x3000);
#endif

    HW_AdcInit();

    Radio.IoInit();

    HW_SPI_Init();

//    HW_RTC_Init();

//    TraceInit();

    McuInitialized = true;
  }
}

/**
  * @brief This function Deinitializes the hardware
  * @param None
  * @retval None
  */
void HW_DeInit(void)
{
  HW_SPI_DeInit();

  Radio.IoDeInit();

  vcom_DeInit();

  McuInitialized = false;
}

/**
  * @brief This function Initializes the hardware Ios
  * @param None
  * @retval None
  */
static void HW_IoInit(void)
{
  HW_SPI_IoInit();

  Radio.IoInit();

  vcom_IoInit();
}

/**
  * @brief This function Deinitializes the hardware Ios
  * @param None
  * @retval None
  */
static void HW_IoDeInit(void)
{
  HW_SPI_IoDeInit();

  Radio.IoDeInit();

  vcom_IoDeInit();
}


void HW_GpioInit(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Configure all GPIO as analog to reduce current consumption on non used IOs */
  /* Enable GPIOs clock */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();

  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  /* All GPIOs except debug pins (SWCLK and SWD) */
  GPIO_InitStruct.Pin = GPIO_PIN_All & (~(GPIO_PIN_13 | GPIO_PIN_14));
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* All GPIOs */
  GPIO_InitStruct.Pin = GPIO_PIN_All;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);
  HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

  /* Disable GPIOs clock */
  __HAL_RCC_GPIOA_CLK_DISABLE();
  __HAL_RCC_GPIOB_CLK_DISABLE();
  __HAL_RCC_GPIOC_CLK_DISABLE();
  __HAL_RCC_GPIOD_CLK_DISABLE();
  __HAL_RCC_GPIOE_CLK_DISABLE();
  __HAL_RCC_GPIOF_CLK_DISABLE();
  __HAL_RCC_GPIOG_CLK_DISABLE();
  __HAL_RCC_GPIOH_CLK_DISABLE();
}
/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follows :
  *            System Clock source            = PLL (MSI)
  *            SYSCLK(Hz)                     = 80000000
  *            HCLK(Hz)                       = 80000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 1
  *            APB2 Prescaler                 = 1
  *            MSI Frequency(Hz)              = 4000000
  *            PLL_M                          = 1
  *            PLL_N                          = 40
  *            PLL_R                          = 2
  *            PLL_P                          = 7
  *            PLL_Q                          = 4
  *            Flash Latency(WS)              = 4
  * @param  None
  * @retval None
  */



void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Configure LSE Drive Capability 
  */
//  HAL_PWR_EnableBkUpAccess();
//  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
//	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 40;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
	/** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
//	HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_MSI, RCC_MCODIV_4);
  /** Configure the main internal regulator output voltage 
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Enable MSI Auto calibration 
  */
  HAL_RCCEx_EnableMSIPLLMode();


	
}
/**
  * @brief This function return a random seed
  * @note based on the device unique ID
  * @param None
  * @retval see
  */
uint32_t HW_GetRandomSeed(void)
{
  return ((*(uint32_t *)ID1) ^ (*(uint32_t *)ID2) ^ (*(uint32_t *)ID3));
}

/**
  * @brief This function return a unique ID
  * @param unique ID
  * @retval none
  */
void HW_GetUniqueId(uint8_t *id)
{
  id[7] = ((*(uint32_t *)ID1) + (*(uint32_t *)ID3)) >> 24;
  id[6] = ((*(uint32_t *)ID1) + (*(uint32_t *)ID3)) >> 16;
  id[5] = ((*(uint32_t *)ID1) + (*(uint32_t *)ID3)) >> 8;
  id[4] = ((*(uint32_t *)ID1) + (*(uint32_t *)ID3));
  id[3] = ((*(uint32_t *)ID2)) >> 24;
  id[2] = ((*(uint32_t *)ID2)) >> 16;
  id[1] = ((*(uint32_t *)ID2)) >> 8;
  id[0] = ((*(uint32_t *)ID2));
}

uint16_t HW_GetTemperatureLevel(void)
{
  uint16_t measuredLevel = 0;
  uint32_t batteryLevelmV;
  uint16_t temperatureDegreeC;

  measuredLevel = HW_AdcReadChannel(ADC_CHANNEL_VREFINT);

  if (measuredLevel == 0)
  {
    batteryLevelmV = 0;
  }
  else
  {
    batteryLevelmV = (((uint32_t) VDDA_VREFINT_CAL * (*VREFINT_CAL)) / measuredLevel);
  }
#if 0
  PRINTF("VDDA= %d\n\r", batteryLevelmV);
#endif

  measuredLevel = HW_AdcReadChannel(ADC_CHANNEL_TEMPSENSOR);

  temperatureDegreeC = COMPUTE_TEMPERATURE(measuredLevel, batteryLevelmV);

#if 0
  {
    uint16_t temperatureDegreeC_Int = (temperatureDegreeC) >> 8;
    uint16_t temperatureDegreeC_Frac = ((temperatureDegreeC - (temperatureDegreeC_Int << 8)) * 100) >> 8;
    PRINTF("temp= %d, %d,%d\n\r", temperatureDegreeC, temperatureDegreeC_Int, temperatureDegreeC_Frac);
  }
#endif

  return (uint16_t) temperatureDegreeC;
}
/**
  * @brief This function return the battery level
  * @param none
  * @retval the battery level in mV
  */
uint16_t HW_GetBatteryLevel(void)
{
  uint16_t measuredLevel = 0;
  uint32_t batteryLevelmV;

  measuredLevel = HW_AdcReadChannel(ADC_CHANNEL_VREFINT);

  if (measuredLevel == 0)
  {
    batteryLevelmV = 0;
  }
  else
  {
    batteryLevelmV = (((uint32_t) VDDA_VREFINT_CAL * (*VREFINT_CAL)) / measuredLevel);
  }

  return batteryLevelmV;
}

/**
  * @brief This function initializes the ADC
  * @param none
  * @retval none
  */
void HW_AdcInit(void)
{
  if (AdcInitialized == false)
  {
    AdcInitialized = true;


    hadc.Instance  = ADC1;

    hadc.Init.OversamplingMode      = DISABLE;

    hadc.Init.ClockPrescaler        = ADC_CLOCK_SYNC_PCLK_DIV1;
    /* hadc.Init.LowPowerAutoPowerOff  = DISABLE; */
    /* hadc.Init.LowPowerFrequencyMode = ENABLE; */
    hadc.Init.LowPowerAutoWait      = DISABLE;

    hadc.Init.Resolution            = ADC_RESOLUTION_12B;
    /* hadc.Init.SamplingTime          = ADC_SAMPLETIME_7CYCLES_5; */
    hadc.Init.ScanConvMode          = ADC_SCAN_DISABLE;
    hadc.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
    hadc.Init.ContinuousConvMode    = DISABLE;
    hadc.Init.DiscontinuousConvMode = DISABLE;
    hadc.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc.Init.EOCSelection          = ADC_EOC_SINGLE_CONV;
    hadc.Init.DMAContinuousRequests = DISABLE;

    ADCCLK_ENABLE();

    HAL_ADC_Init(&hadc);

  }
}
/**
  * @brief This function De-initializes the ADC
  * @param none
  * @retval none
  */
void HW_AdcDeInit(void)
{
  AdcInitialized = false;
  HAL_ADC_DeInit(&hadc);
}

/**
  * @brief This function De-initializes the ADC
  * @param Channel
  * @retval Value
  */
uint16_t HW_AdcReadChannel(uint32_t Channel)
{

  ADC_ChannelConfTypeDef adcConf = {0};

  uint16_t adcData = 0;

  HW_AdcInit();

  if (AdcInitialized == true)
  {

    ADCCLK_ENABLE();

    /*calibrate ADC if any calibraiton hardware*/
    HAL_ADCEx_Calibration_Start(&hadc, ADC_SINGLE_ENDED);


    /* configure adc channel */
    adcConf.SamplingTime = ADC_SAMPLETIME_47CYCLES_5;
    adcConf.Channel = Channel;
    adcConf.Rank = ADC_REGULAR_RANK_1;
    HAL_ADC_ConfigChannel(&hadc, &adcConf);

    /* Start the conversion process */
    HAL_ADC_Start(&hadc);

    /* Wait for the end of conversion */
    HAL_ADC_PollForConversion(&hadc, HAL_MAX_DELAY);

    /* Get the converted value of regular channel */
    adcData = HAL_ADC_GetValue(&hadc);

    ADC_Disable(&hadc) ;

    ADCCLK_DISABLE();
  }
  return adcData;
}

/**
  * @brief Enters Low Power Stop Mode
  * @note ARM exists the function when waking up
  * @param none
  * @retval none
  */
void LPM_EnterStopMode(void)
{
  BACKUP_PRIMASK();

  DISABLE_IRQ();

  HW_IoDeInit();

  HW_AdcDeInit();

  RESTORE_PRIMASK();

  /* Enter Stop Mode */
  HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
}
/**
  * @brief Exists Low Power Stop Mode
  * @note Enable the pll at 32MHz
  * @param none
  * @retval none
  */
void LPM_ExitStopMode(void)
{
  /* Disable IRQ while the MCU is not running on PLL */

  BACKUP_PRIMASK();
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  uint32_t pFLatency = 0;

  DISABLE_IRQ();

  /* In case nvic had a pending IT, the arm doesn't enter stop mode
   * Hence the pll is not switched off and will cause HAL_RCC_OscConfig return
    an error*/
  if (__HAL_RCC_GET_SYSCLK_SOURCE() != RCC_CFGR_SWS_PLL)
  {
    /* Enable Power Control clock */
    __HAL_RCC_PWR_CLK_ENABLE();

    /* Get the Oscillators configuration according to the internal RCC registers */
    HAL_RCC_GetOscConfig(&RCC_OscInitStruct);

    /* Enable PLL */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_NONE;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
      while (1);
    }

    /* Get the Clocks configuration according to the internal RCC registers */
    HAL_RCC_GetClockConfig(&RCC_ClkInitStruct, &pFLatency);

    /* Select PLL as system clock source and keep HCLK, PCLK1 and PCLK2 clocks dividers as before */
    RCC_ClkInitStruct.ClockType     = RCC_CLOCKTYPE_SYSCLK;
    RCC_ClkInitStruct.SYSCLKSource  = RCC_SYSCLKSOURCE_PLLCLK;
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, pFLatency) != HAL_OK)
    {
      while (1);
    }
  }
  else
  {
    /*mcu did not enter stop mode beacuse NVIC had a pending IT*/
  }

  HW_IoInit();

  RESTORE_PRIMASK();
}

/**
  * @brief Enters Low Power Sleep Mode
  * @note ARM exits the function when waking up
  * @param none
  * @retval none
  */
void LPM_EnterSleepMode(void)
{
  HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

