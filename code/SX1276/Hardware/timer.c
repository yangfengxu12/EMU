#include "timer.h"

TIM_HandleTypeDef TIM1_Handler;      //��ʱ����� 

uint32_t time_count = 0;

//ͨ�ö�ʱ��3�жϳ�ʼ��
//arr���Զ���װֵ��
//psc��ʱ��Ԥ��Ƶ��
//��ʱ�����ʱ����㷽��:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=��ʱ������Ƶ��,��λ:Mhz
//����ʹ�õ��Ƕ�ʱ��3!(��ʱ��3����APB1�ϣ�ʱ��ΪHCLK/2)
void TIM1_Init(u32 arr,u16 psc)
{  
    TIM1_Handler.Instance=TIM1;                          //ͨ�ö�ʱ��3
    TIM1_Handler.Init.Prescaler=psc;                     //��Ƶϵ��
    TIM1_Handler.Init.CounterMode=TIM_COUNTERMODE_UP;    //���ϼ�����
    TIM1_Handler.Init.Period=arr;                        //�Զ�װ��ֵ
    TIM1_Handler.Init.ClockDivision=TIM_CLOCKDIVISION_DIV1;//ʱ�ӷ�Ƶ����
    HAL_TIM_Base_Init(&TIM1_Handler);
    
//    HAL_TIM_Base_Start_IT(&TIM1_Handler); //ʹ�ܶ�ʱ��3�Ͷ�ʱ��3�����жϣ�TIM_IT_UPDATE   
		HAL_TIM_Base_Stop_IT(&TIM1_Handler);
		time_count = 0;
}


//��ʱ���ײ�����������ʱ�ӣ������ж����ȼ�
//�˺����ᱻHAL_TIM_Base_Init()��������
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
    if(htim->Instance==TIM1)
	{
		__HAL_RCC_TIM1_CLK_ENABLE();            //ʹ��TIM1ʱ��
		HAL_NVIC_SetPriority(TIM1_UP_TIM16_IRQn,0,0);    //�����ж����ȼ�����ռ���ȼ�1�������ȼ�3
		HAL_NVIC_EnableIRQ(TIM1_UP_TIM16_IRQn);          //����ITM3�ж�   
	}
}


//��ʱ��3�жϷ�����
void TIM1_IRQHandler(void)
{
//    HAL_TIM_IRQHandler(&TIM1_Handler);

}

//�ص���������ʱ���жϷ���������
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if(htim==(&TIM1_Handler))
    {
//			time_count += 5;
//			HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_2);
    }
}