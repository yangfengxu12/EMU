#include "rtc.h"
#include "usart.h"
#include "control_GPIO.h"

RTC_HandleTypeDef RTC_Handler;  //RTC句柄
RTC_TimeTypeDef sTimeStampget;

//RTC时间设置
//hour,min,sec:小时,分钟,秒钟
//ampm:@RTC_AM_PM_Definitions:RTC_HOURFORMAT12_AM/RTC_HOURFORMAT12_PM
//返回值:SUCEE(1),成功
//       ERROR(0),进入初始化模式失败 
HAL_StatusTypeDef RTC_Set_Time(uint8_t hour,uint8_t min,uint8_t sec,uint8_t ampm)
{
	RTC_TimeTypeDef RTC_TimeStructure;
	
	RTC_TimeStructure.Hours=hour;
	RTC_TimeStructure.Minutes=min;
	RTC_TimeStructure.Seconds=sec;
	RTC_TimeStructure.TimeFormat=ampm;
	RTC_TimeStructure.DayLightSaving=RTC_DAYLIGHTSAVING_NONE;
    RTC_TimeStructure.StoreOperation=RTC_STOREOPERATION_RESET;
	return HAL_RTC_SetTime(&RTC_Handler,&RTC_TimeStructure,RTC_FORMAT_BIN);	
}

//RTC日期设置
//year,month,date:年(0~99),月(1~12),日(0~31)
//week:星期(1~7,0,非法!)
//返回值:SUCEE(1),成功
//       ERROR(0),进入初始化模式失败 
HAL_StatusTypeDef RTC_Set_Date(uint8_t year,uint8_t month,uint8_t date,uint8_t week)
{
	RTC_DateTypeDef RTC_DateStructure;
    
	RTC_DateStructure.Date=date;
	RTC_DateStructure.Month=month;
	RTC_DateStructure.WeekDay=week;
	RTC_DateStructure.Year=year;
	return HAL_RTC_SetDate(&RTC_Handler,&RTC_DateStructure,RTC_FORMAT_BIN);
}

//RTC初始化
//返回值:0,初始化成功;
//       2,进入初始化模式失败;
uint8_t RTC_Init(void)
{      
	
	__HAL_RTC_RESET_HANDLE_STATE(&RTC_Handler);
  RTC_Handler.Instance = RTC;
  RTC_Handler.Init.HourFormat     = RTC_HOURFORMAT_24;
  RTC_Handler.Init.AsynchPrediv   = 0x7F;
  RTC_Handler.Init.SynchPrediv    = 0x00FF;
  RTC_Handler.Init.OutPut         = RTC_OUTPUT_DISABLE;
  RTC_Handler.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  RTC_Handler.Init.OutPutType     = RTC_OUTPUT_TYPE_PUSHPULL;
	
	if(HAL_RTC_Init(&RTC_Handler) != HAL_OK)
  {
    /* Initialization Error */
    while(1);
  }
	HAL_RTCEx_EnableBypassShadow(&RTC_Handler);
}

void HAL_RTC_MspInit(RTC_HandleTypeDef *hrtc)
{
  RCC_OscInitTypeDef        RCC_OscInitStruct;
  RCC_PeriphCLKInitTypeDef  PeriphClkInitStruct;
  
  /*##-1- Enables the PWR Clock and Enables access to the backup domain ###################################*/
  /* To change the source clock of the RTC feature (LSE, LSI), You have to:
     - Enable the power clock using __HAL_RCC_PWR_CLK_ENABLE()
     - Enable write access using HAL_PWR_EnableBkUpAccess() function before to 
       configure the RTC clock source (to be done once after reset).
     - Reset the Back up Domain using __HAL_RCC_BACKUPRESET_FORCE() and 
       __HAL_RCC_BACKUPRESET_RELEASE().
     - Configure the needed RTC clock source */
  __HAL_RCC_PWR_CLK_ENABLE();
  HAL_PWR_EnableBkUpAccess();

  /*##-2- Configure LSE/LSI as RTC clock source ###############################*/

  
  RCC_OscInitStruct.OscillatorType =   RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  { 
    while(1);
  }
  
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  if(HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  { 
    while(1);
  }

  
  /*##-3- Enable RTC peripheral Clocks #######################################*/
  /* Enable RTC Clock */ 
  __HAL_RCC_RTC_ENABLE(); 
  
  /*##-4- Configure the NVIC for RTC TimeStamp ###############################*/
  HAL_NVIC_SetPriority(TAMP_STAMP_IRQn, 0x0F, 0);
  HAL_NVIC_EnableIRQ(TAMP_STAMP_IRQn);
}


/**
  * @brief  Configure the current time and date and activate timestamp.
  * @param  None
  * @retval None
  */
void RTC_TimeStampConfig(void)
{
  RTC_DateTypeDef sdatestructure;
  RTC_TimeTypeDef stimestructure;

  /*##-1- Configure the Time Stamp peripheral ################################*/
  /*  RTC TimeStamp generation: TimeStamp Rising Edge on PC.13 Pin */
  HAL_RTCEx_SetTimeStamp_IT(&RTC_Handler, RTC_TIMESTAMPEDGE_FALLING, RTC_TIMESTAMPPIN_DEFAULT);

  /*##-2- Configure the Date #################################################*/
  /* Set Date: Monday April 14th 2014 */
  sdatestructure.Year    = 0x14;
  sdatestructure.Month   = RTC_MONTH_APRIL;
  sdatestructure.Date    = 0x14;
  sdatestructure.WeekDay = RTC_WEEKDAY_MONDAY;
  
  if(HAL_RTC_SetDate(&RTC_Handler,&sdatestructure,RTC_FORMAT_BCD) != HAL_OK)
  {
    /* Initialization Error */
    while(1);
  } 
  
  /*##-3- Configure the Time #################################################*/
  /* Set Time: 08:10:00 */
  stimestructure.Hours          = 0x08;
  stimestructure.Minutes        = 0x10;
  stimestructure.Seconds        = 0x00;
  stimestructure.SubSeconds     = 0x00;
  stimestructure.TimeFormat     = RTC_HOURFORMAT12_AM;
  stimestructure.DayLightSaving = RTC_DAYLIGHTSAVING_NONE ;
  stimestructure.StoreOperation = RTC_STOREOPERATION_RESET;
  
  if(HAL_RTC_SetTime(&RTC_Handler,&stimestructure,RTC_FORMAT_BCD) != HAL_OK)
  {
    /* Initialization Error */
    while(1); 
  }
	
	__HAL_RTC_TIMESTAMP_CLEAR_FLAG(&RTC_Handler, RTC_FLAG_TSF);
}




void TAMP_STAMP_IRQHandler(void)
{
  HAL_RTCEx_TamperTimeStampIRQHandler(&RTC_Handler);
}

void HAL_RTCEx_TimeStampEventCallback(RTC_HandleTypeDef *hrtc)
{
//  RTC_DateTypeDef sTimeStampDateget;

//  HAL_RTCEx_GetTimeStamp(&RTC_Handler, &sTimeStampget, &sTimeStampDateget, RTC_FORMAT_BIN);
}

void RTC_Set_WakeUp(uint32_t wksel,uint16_t cnt)
{ 
  __HAL_RTC_WAKEUPTIMER_CLEAR_FLAG(&RTC_Handler, RTC_FLAG_WUTF);//清除RTC WAKE UP的标志
	
	HAL_RTCEx_SetWakeUpTimer_IT(&RTC_Handler,cnt,wksel);            //设置重装载值和时钟 
	
  HAL_NVIC_SetPriority(RTC_WKUP_IRQn,0x02,0x02); //抢占优先级1,子优先级2
  HAL_NVIC_EnableIRQ(RTC_WKUP_IRQn);
}

void RTC_WKUP_IRQHandler(void)
{    
	if(LL_RTC_IsActiveFlag_WUT(RTC) != 0)//WK_UP中断?
	{ 
		LL_RTC_ClearFlag_WUT(RTC);	//清除中断标志
		LL_GPIO_TogglePin(GPIOB,GPIO_PIN_2);
	}   
	LL_EXTI_ClearFlag_0_31(EXTI_IMR1_IM18);							
}




uint32_t LL_RTC_IsActiveFlag_WUT(RTC_TypeDef *RTCx)
{
  return (READ_BIT(RTCx->ISR, RTC_ISR_WUTF) == (RTC_ISR_WUTF));
}

void LL_RTC_ClearFlag_WUT(RTC_TypeDef *RTCx)
{
  WRITE_REG(RTCx->ISR, (~((RTC_ISR_WUTF | RTC_ISR_INIT) & 0x0000FFFFU) | (RTCx->ISR & RTC_ISR_INIT)));
}

void LL_EXTI_ClearFlag_0_31(uint32_t ExtiLine)
{
  WRITE_REG(EXTI->PR1, ExtiLine);
}
