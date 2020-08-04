#include "timer.h"

TIM_HandleTypeDef TIM1_Handler;      //定时器句柄 

uint32_t time_count = 0;

//通用定时器3中断初始化
//arr：自动重装值。
//psc：时钟预分频数
//定时器溢出时间计算方法:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=定时器工作频率,单位:Mhz
//这里使用的是定时器3!(定时器3挂在APB1上，时钟为HCLK/2)
void TIM1_Init(u32 arr,u16 psc)
{  
    TIM1_Handler.Instance=TIM1;                          //通用定时器3
    TIM1_Handler.Init.Prescaler=psc;                     //分频系数
    TIM1_Handler.Init.CounterMode=TIM_COUNTERMODE_UP;    //向上计数器
    TIM1_Handler.Init.Period=arr;                        //自动装载值
    TIM1_Handler.Init.ClockDivision=TIM_CLOCKDIVISION_DIV1;//时钟分频因子
    HAL_TIM_Base_Init(&TIM1_Handler);
    
//    HAL_TIM_Base_Start_IT(&TIM1_Handler); //使能定时器3和定时器3更新中断：TIM_IT_UPDATE   
		HAL_TIM_Base_Stop_IT(&TIM1_Handler);
		time_count = 0;
}


//定时器底册驱动，开启时钟，设置中断优先级
//此函数会被HAL_TIM_Base_Init()函数调用
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
    if(htim->Instance==TIM1)
	{
		__HAL_RCC_TIM1_CLK_ENABLE();            //使能TIM1时钟
		HAL_NVIC_SetPriority(TIM1_UP_TIM16_IRQn,0,0);    //设置中断优先级，抢占优先级1，子优先级3
		HAL_NVIC_EnableIRQ(TIM1_UP_TIM16_IRQn);          //开启ITM3中断   
	}
}


//定时器3中断服务函数
void TIM1_IRQHandler(void)
{
//    HAL_TIM_IRQHandler(&TIM1_Handler);

}

//回调函数，定时器中断服务函数调用
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if(htim==(&TIM1_Handler))
    {
//			time_count += 5;
			HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_2);
    }
}
