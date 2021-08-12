#include "timer.h"

TIM_HandleTypeDef TIM2_Handler;      //��ʱ����� 
TIM_HandleTypeDef TIM3_Handler;      //��ʱ����� 

uint32_t Time;

_Bool Chip_flag = 0;

//ͨ�ö�ʱ��3�жϳ�ʼ��
//arr���Զ���װֵ��
//psc��ʱ��Ԥ��Ƶ��
//��ʱ�����ʱ����㷽��:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=��ʱ������Ƶ��,��λ:Mhz
//����ʹ�õ��Ƕ�ʱ��3!(��ʱ��3����APB1�ϣ�ʱ��ΪHCLK/2)
void TIM2_Init(u32 arr,u16 psc)
{  
    TIM2_Handler.Instance=TIM2;                          //ͨ�ö�ʱ��3
    TIM2_Handler.Init.Prescaler=psc;                     //��Ƶϵ��
    TIM2_Handler.Init.CounterMode=TIM_COUNTERMODE_UP;    //up conuter
    TIM2_Handler.Init.Period=arr;                        //�Զ�װ��ֵ
    TIM2_Handler.Init.ClockDivision=TIM_CLOCKDIVISION_DIV1;//ʱ�ӷ�Ƶ����
    HAL_TIM_Base_Init(&TIM2_Handler);
    
//    HAL_TIM_Base_Start_IT(&TIM2_Handler); //ʹ�ܶ�ʱ��3�Ͷ�ʱ��3�����жϣ�TIM_IT_UPDATE   
		HAL_TIM_Base_Stop_IT(&TIM2_Handler);
}


//��ʱ���ײ�����������ʱ�ӣ������ж����ȼ�
//�˺����ᱻHAL_TIM_Base_Init()��������
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
  if(htim->Instance==TIM2)
	{
		__HAL_RCC_TIM2_CLK_ENABLE();            //ʹ��TIM2ʱ��
		HAL_NVIC_SetPriority(TIM2_IRQn,0,0);    //�����ж����ȼ�����ռ���ȼ�0�������ȼ�0
		HAL_NVIC_EnableIRQ(TIM2_IRQn);          //����ITM2�ж�   
	}
	if(htim->Instance==TIM3)
	{
		__HAL_RCC_TIM3_CLK_ENABLE();            //ʹ��TIM3ʱ��
		HAL_NVIC_SetPriority(TIM3_IRQn,0,1);    //�����ж����ȼ�����ռ���ȼ�0�������ȼ�0
		HAL_NVIC_EnableIRQ(TIM3_IRQn);          //����ITM3�ж�   
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
    TIM3_Handler.Instance=TIM3;                          //ͨ�ö�ʱ��3
    TIM3_Handler.Init.Prescaler=80-1;                     //��Ƶϵ��
    TIM3_Handler.Init.CounterMode=TIM_COUNTERMODE_UP;    //���¼�����
    TIM3_Handler.Init.Period=arr;                        //�Զ�װ��ֵ
    TIM3_Handler.Init.ClockDivision=TIM_CLOCKDIVISION_DIV1;//ʱ�ӷ�Ƶ����
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


//uint32_t LL_TIM_IsActiveFlag_UPDATE(TIM_TypeDef *TIMx)
//{
//  return ((READ_BIT(TIMx->SR, TIM_SR_UIF) == (TIM_SR_UIF)) ? 1UL : 0UL);
//}

//void LL_TIM_ClearFlag_UPDATE(TIM_TypeDef *TIMx)
//{
//  WRITE_REG(TIMx->SR, ~(TIM_SR_UIF));
//}

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

	HAL_TIM_IC_Start_IT(&htim15,TIM_CHANNEL_1);   //����TIM5�Ĳ���ͨ��1�����ҿ��������ж�
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

//��ʱ��5�жϷ�����
void TIM1_BRK_TIM15_IRQHandler(void)
{
	HAL_TIM_IRQHandler(&htim15);//��ʱ�����ô�����
}

u8  TIM15CH1_CAPTURE_STA = 0;	//���벶��״̬		    				
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

void HAL_TIM_IC_CaptureCallback( TIM_HandleTypeDef *htim )//�����жϷ���ʱִ��
{
	if(( TIM15CH1_CAPTURE_STA & 0x80 ) == 0 )//��һ��������
	{
		Input_Captured_Record[ Input_Captured_Count ][ 0 ] = HAL_TIM_ReadCapturedValue( &htim15, TIM_CHANNEL_1 );
		TIM15CH1_CAPTURE_STA = 0x80;
		Rising_Edge_Count = 1;
	}
	else //��������������
	{
		Rising_Edge_Count++;
		if( Rising_Edge_Count == 32767 )
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

//--------------------------------------------------------------

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









