#include "stm32l4xx.h"
#include "stm32l4xx_hal.h"
#include <string.h>
#include "hw.h"

#include "CC1101.h"

#include "timer.h"
#include "fast_spi.h"
#include "rtc.h"
#include "delay.h"
#include "usart.h"

#include "Simulated_LoRa.h"
#include "Timer_Calibration.h"
#include "control_GPIO.h"

int main(void)
{
  HAL_Init();
  SystemClock_Config();
	
	HW_SPI_Init();
	SPI1_Init();
	
	delay_init(80);
	uart_init(115200);
	printf("123");
	
	Control_GPIO_Init();
	
	TIM2_Init(0xffffffff,80-1);       //Timer resolution = 1us; auto-reload value = 0xfffff
	
	CC1101_Init();
	
//	CC1101_Burst_Read(0x00,buffer,47);
//	CC1101_Set_OpMode( STX );
//	CC1101_Burst_Write( REG_FREQ2,freq1,3);
//	CC1101_Burst_Write( REG_FREQ2,freq2,3);
//	CC1101_Burst_Write( REG_FREQ2,freq3,3);
//	CC1101_Burst_Write( REG_FREQ2,freq4,3);
//	CC1101_Burst_Read(0x00,buffer,47);
//	CC1101_Reset();
//	RTC_Timer_Calibration();
	

//	HAL_TIM_Base_Start_IT(&TIM1_Handler);
//	CC1101_Set_OpMode( STX );
//	DelayMs(100);
  while (1)
  {
		LoRa_Generate_Signal();
  }
}


