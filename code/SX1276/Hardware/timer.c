#include "timer.h"

TIM_HandleTypeDef TIM2_Handler;      //定时器句柄 

uint32_t Time;
uint32_t time_count = 0;

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
//		time_count = 0;
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
}


//定时器3中断服务函数
void TIM2_IRQHandler(void)
{
//    HAL_TIM_IRQHandler(&TIM2_Handler);
	if(LL_TIM_IsActiveFlag_UPDATE(TIM2) == 1)
  {
    /* Clear the update interrupt flag*/
//		LL_GPIO_TogglePin(GPIOB,GPIO_PIN_2);
//		time_count++;
		Chip_flag = 1;
    LL_TIM_ClearFlag_UPDATE(TIM2);
  }
}

////回调函数，定时器中断服务函数调用
//void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
//{
//    if(htim==(&TIM2_Handler))
//    {
////			time_count += 5;
//			
//    }
//}

uint32_t LL_TIM_IsActiveFlag_UPDATE(TIM_TypeDef *TIMx)
{
  return ((READ_BIT(TIMx->SR, TIM_SR_UIF) == (TIM_SR_UIF)) ? 1UL : 0UL);
}

void LL_TIM_ClearFlag_UPDATE(TIM_TypeDef *TIMx)
{
  WRITE_REG(TIMx->SR, ~(TIM_SR_UIF));
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
int Input_Captured_Record[5][4] = {0};
u16 Input_Captured_Count = 0;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(TIM15CH1_CAPTURE_STA)//
	{
		TIM15CH1_CAPTURE_STA++; 
	}	
}

u8 i;

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)//捕获中断发生时执行
{
	if((TIM15CH1_CAPTURE_STA&0x80)==0)//第一个上升沿
	{
		Input_Captured_Record[Input_Captured_Count][0] = HAL_TIM_ReadCapturedValue(&htim15,TIM_CHANNEL_1);
		TIM15CH1_CAPTURE_STA = 0x80;
		Rising_Edge_Count = 1;
	}
	else //接下来的上升沿
	{
		Rising_Edge_Count++;
		if(Rising_Edge_Count == 32768)
		{
			Input_Captured_Record[Input_Captured_Count][1] = HAL_TIM_ReadCapturedValue(&htim15,TIM_CHANNEL_1);
			Input_Captured_Record[Input_Captured_Count][2] = TIM15CH1_CAPTURE_STA&0x7F;
			
			Input_Captured_Count++;
			TIM15CH1_CAPTURE_STA = 0;
			Rising_Edge_Count = 0;
			if(Input_Captured_Count >= 5)
			{
				HAL_TIM_IC_Stop_IT(&htim15,TIM_CHANNEL_1);
 				HAL_NVIC_DisableIRQ(TIM1_BRK_TIM15_IRQn);
				for(i=0;i<5;i++)
				Input_Captured_Record[i][3] = 65536 * Input_Captured_Record[i][2] + Input_Captured_Record[i][1] - Input_Captured_Record[i][0];
				Input_Captured_Count = 0;
			}
		}
	}
	
}
