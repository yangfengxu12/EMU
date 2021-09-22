#include "timer.h"

TIM_HandleTypeDef TIM2_Handler;      //定时器句柄 
TIM_HandleTypeDef TIM3_Handler;      //定时器句柄 

uint32_t Time;

_Bool Chip_flag = 0;

//通用定时器3中断初始化
//arr：自动重装值。
//psc：时钟预分频数
//定时器溢出时间计算方法:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=定时器工作频率,单位:Mhz
//这里使用的是定时器3!(定时器3挂在APB1上，时钟为HCLK/2)
void TIM2_Init(u32 arr,u16 psc)
{  
    TIM2_Handler.Instance=TIM2;                          //通用定时器3
    TIM2_Handler.Init.Prescaler=psc;                     //分频系数
    TIM2_Handler.Init.CounterMode=TIM_COUNTERMODE_UP;    //向下计数器
    TIM2_Handler.Init.Period=arr;                        //自动装载值
    TIM2_Handler.Init.ClockDivision=TIM_CLOCKDIVISION_DIV1;//时钟分频因子
    HAL_TIM_Base_Init(&TIM2_Handler);
    
//    HAL_TIM_Base_Start_IT(&TIM2_Handler); //使能定时器3和定时器3更新中断：TIM_IT_UPDATE   
		HAL_TIM_Base_Stop_IT(&TIM2_Handler);
}


//定时器底册驱动，开启时钟，设置中断优先级
//此函数会被HAL_TIM_Base_Init()函数调用
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
  if(htim->Instance==TIM2)
	{
		__HAL_RCC_TIM2_CLK_ENABLE();            //使能TIM2时钟
		HAL_NVIC_SetPriority(TIM2_IRQn,0,0);    //设置中断优先级，抢占优先级0，子优先级0
		HAL_NVIC_EnableIRQ(TIM2_IRQn);          //开启ITM2中断   
	}
	if(htim->Instance==TIM3)
	{
		__HAL_RCC_TIM3_CLK_ENABLE();            //使能TIM3时钟
		HAL_NVIC_SetPriority(TIM3_IRQn,0,1);    //设置中断优先级，抢占优先级0，子优先级0
		HAL_NVIC_EnableIRQ(TIM3_IRQn);          //开启ITM3中断   
	}
}



void TIM2_IRQHandler(void)
{
	if(LL_TIM_IsActiveFlag_UPDATE(TIM2) == 1)
  {
		LL_TIM_ClearFlag_UPDATE(TIM2);
  }
}

int Timer_Compensation_Count = 0;

void TIM3_Init(u32 arr)
{  
    TIM3_Handler.Instance=TIM3;                          //通用定时器3
    TIM3_Handler.Init.Prescaler=80-1;                     //分频系数
    TIM3_Handler.Init.CounterMode=TIM_COUNTERMODE_UP;    //向下计数器
    TIM3_Handler.Init.Period=arr;                        //自动装载值
    TIM3_Handler.Init.ClockDivision=TIM_CLOCKDIVISION_DIV1;//时钟分频因子
    HAL_TIM_Base_Init(&TIM3_Handler);
     
		HAL_TIM_Base_Stop_IT(&TIM3_Handler);
		Timer_Compensation_Count = 0;
}

void TIM3_IRQHandler(void)
{
	if(LL_TIM_IsActiveFlag_UPDATE(TIM3) == 1)
  {
		Timer_Compensation_Count++;
		LL_TIM_ClearFlag_UPDATE(TIM3);
  }
}



TIM_HandleTypeDef htim15;
/**
  * @brief TIM15 Initialization Function
  * @param None
  * @retval None
  */
void TIM15_Init(void)
{
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_IC_InitTypeDef sConfigIC = {0};
	
  htim15.Instance = TIM15;
  htim15.Init.Prescaler = 79;
  htim15.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim15.Init.Period = 0xffff;
  htim15.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim15.Init.RepetitionCounter = 0;
  htim15.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
	
  if (HAL_TIM_IC_Init(&htim15) != HAL_OK)
  {
//    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim15, &sMasterConfig) != HAL_OK)
  {
//    Error_Handler();
  }
  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 0;
  if (HAL_TIM_IC_ConfigChannel(&htim15, &sConfigIC, TIM_CHANNEL_1) != HAL_OK)
  {
//    Error_Handler();
  }
  if (HAL_TIMEx_RemapConfig(&htim15, TIM_TIM15_TI1_LSE) != HAL_OK)
  {
//    Error_Handler();
  }

	HAL_TIM_IC_Start_IT(&htim15,TIM_CHANNEL_1);   //开启TIM5的捕获通道1，并且开启捕获中断
	__HAL_TIM_ENABLE_IT(&htim15,TIM_IT_UPDATE);
}

void HAL_TIM_IC_MspInit(TIM_HandleTypeDef* htim_ic)
{
  if(htim_ic->Instance==TIM15)
  {
    /* Peripheral clock enable */
    __HAL_RCC_TIM15_CLK_ENABLE();
    /* TIM15 interrupt Init */
    HAL_NVIC_SetPriority(TIM1_BRK_TIM15_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM1_BRK_TIM15_IRQn);
  }
}

//定时器5中断服务函数
void TIM1_BRK_TIM15_IRQHandler(void)
{
	HAL_TIM_IRQHandler(&htim15);//定时器共用处理函数
}

u8  TIM15CH1_CAPTURE_STA = 0;	//输入捕获状态		    				
u32 Rising_Edge_Count = 0;
int Input_Captured_Record[ Calibration_Times ][ 4 ] = {0};
u16 Input_Captured_Count = 0;
u8 Timer_Calibration_Done_Flag = 0; //0:doing 1:done

void HAL_TIM_PeriodElapsedCallback( TIM_HandleTypeDef *htim )
{
	if( TIM15CH1_CAPTURE_STA )//
	{
		TIM15CH1_CAPTURE_STA++; 
	}	
}



void HAL_TIM_IC_CaptureCallback( TIM_HandleTypeDef *htim )//捕获中断发生时执行
{
	if(( TIM15CH1_CAPTURE_STA & 0x80 ) == 0 )//第一个上升沿
	{
		Input_Captured_Record[ Input_Captured_Count ][ 0 ] = HAL_TIM_ReadCapturedValue( &htim15, TIM_CHANNEL_1 );
		TIM15CH1_CAPTURE_STA = 0x80;
		Rising_Edge_Count = 1;
	}
	else //接下来的上升沿
	{
		Rising_Edge_Count++;
		if( Rising_Edge_Count == 32768 )
		{
			Input_Captured_Record[ Input_Captured_Count ][ 1 ] = HAL_TIM_ReadCapturedValue( &htim15, TIM_CHANNEL_1 );
			Input_Captured_Record[ Input_Captured_Count ][ 2 ] = TIM15CH1_CAPTURE_STA&0x7F;
			
			Input_Captured_Count++;
			TIM15CH1_CAPTURE_STA = 0;
			Rising_Edge_Count = 0;
			if( Input_Captured_Count >= Calibration_Times )
			{
				HAL_TIM_IC_Stop_IT( &htim15, TIM_CHANNEL_1 );
 				
				
				Input_Captured_Count = 0;
				Timer_Calibration_Done_Flag = 1;
			}
		}
	}
	
}



void MX_TIM3_Init(void)
{
	
  LL_TIM_InitTypeDef TIM_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM3);

  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOC);
  /**TIM3 GPIO Configuration
  PC7   ------> TIM3_CH2
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_7;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_2;
  LL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  TIM_InitStruct.Prescaler = 0;
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
  TIM_InitStruct.Autoreload = 65535;
  TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
  LL_TIM_Init(TIM3, &TIM_InitStruct);
  LL_TIM_DisableARRPreload(TIM3);
  LL_TIM_SetTriggerInput(TIM3, LL_TIM_TS_TI2FP2);
  LL_TIM_SetClockSource(TIM3, LL_TIM_CLOCKSOURCE_EXT_MODE1);
  LL_TIM_CC_DisableChannel(TIM3, LL_TIM_CHANNEL_CH2);
  LL_TIM_IC_SetFilter(TIM3, LL_TIM_CHANNEL_CH2, LL_TIM_IC_FILTER_FDIV1);
  LL_TIM_IC_SetPolarity(TIM3, LL_TIM_CHANNEL_CH2, LL_TIM_IC_POLARITY_RISING);
  LL_TIM_DisableIT_TRIG(TIM3);
  LL_TIM_DisableDMAReq_TRIG(TIM3);
  LL_TIM_SetTriggerOutput(TIM3, LL_TIM_TRGO_UPDATE);
  LL_TIM_EnableMasterSlaveMode(TIM3);
	
	LL_TIM_EnableIT_CC1(TIM3);
//	LL_TIM_EnableCounter(TIM3);

}

void MX_TIM4_Init(void)
{

  LL_TIM_InitTypeDef TIM_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM4);

  TIM_InitStruct.Prescaler = 0;
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
  TIM_InitStruct.Autoreload = 65535;
  TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
  LL_TIM_Init(TIM4, &TIM_InitStruct);
  LL_TIM_DisableARRPreload(TIM4);
  LL_TIM_SetTriggerInput(TIM4, LL_TIM_TS_ITR2);
  LL_TIM_SetClockSource(TIM4, LL_TIM_CLOCKSOURCE_EXT_MODE1);
  LL_TIM_DisableIT_TRIG(TIM4);
  LL_TIM_DisableDMAReq_TRIG(TIM4);
  LL_TIM_SetTriggerOutput(TIM4, LL_TIM_TRGO_RESET);
  LL_TIM_DisableMasterSlaveMode(TIM4);
	
	LL_TIM_EnableIT_CC1(TIM4);
//	LL_TIM_EnableCounter(TIM4);


}
