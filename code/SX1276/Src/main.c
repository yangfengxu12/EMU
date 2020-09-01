#include "stm32l4xx.h"
#include <string.h>
#include "hw.h"
#include "radio.h"
#include "timeServer.h"
#include "low_power_manager.h"
#include "vcom.h"
#include "sx1276.h"
#include "sx1276mb1mas.h"

#include "timer.h"
#include "fast_spi.h"
#include "rtc.h"
#include "delay.h"
#include "usart.h"

#include "Simulated_LoRa.h"
#include "Timer_Calibration.h"

//1.00014136 = 1      +2us


static RadioEvents_t RadioEvents;
																											
extern TIM_HandleTypeDef TIM2_Handler;
extern uint32_t time_count;
extern uint32_t Time_temp;
/* Private function prototypes -----------------------------------------------*/
/*!
 * Radio events function pointer
 */
//static RadioEvents_t RadioEvents;
void RTC_Timer_Calibration(void);

void Generate_chip( uint32_t freq );

/**
 * Main application entry point.
 */
int main(void)
{
//  bool isMaster = true;
	uint8_t m;
  HAL_Init();
	
  SystemClock_Config();

  HW_Init();
	
	SPI1_Init();
	delay_init(80);
	uart_init(115200);
	printf("123");
	RTC_Init();
	
	/*Disbale Stand-by mode*/
  LPM_SetOffMode(LPM_APPLI_Id, LPM_Disable);
	TIM2_Init(0xfffffff,80-1);       //Timer resolution = 1us; auto-reload value =0
	
	GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	
  GPIO_InitStruct.Pin   = GPIO_PIN_2 | GPIO_PIN_5 | GPIO_PIN_12;					//PB5
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pin   = GPIO_PIN_1;
	GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	
	HW_GPIO_Write(GPIOB,GPIO_PIN_5,GPIO_PIN_SET); //PB5 DIO2 DATA HIGH
	HW_GPIO_Write(GPIOB,GPIO_PIN_2,GPIO_PIN_RESET);
	HW_GPIO_Write(GPIOB,GPIO_PIN_12,GPIO_PIN_RESET);
	
	GPIO_InitStruct.Pin   = GPIO_PIN_9;				//PA9 DIO4_a PLL LOCK
  GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	Radio.Init(&RadioEvents);

  Radio.SetChannel(RF_FREQUENCY);

	Radio.SetTxContinuousWave(RF_FREQUENCY,TX_OUTPUT_POWER,3);

	SX1276Write( REG_PLLHOP, ( SX1276Read( REG_PLLHOP ) & RF_PLLHOP_FASTHOP_MASK ) | RF_PLLHOP_FASTHOP_ON );
	
	SX1276Write( REG_PARAMP, ( SX1276Read( REG_PARAMP ) & RF_PARAMP_MASK ) | RF_PARAMP_0010_US );
	
	SX1276Write( REG_OCP, ( SX1276Read( REG_OCP ) & RF_OCP_MASK ) | RF_OCP_OFF );
	

//	HAL_TIM_Base_Start_IT(&TIM3_Handler);
//	SX1276SetOpMode( RF_OPMODE_TRANSMITTER );
//	DelayMs(100);
  while (1)
  {
		RTC_Timer_Calibration();
		SX1276SetOpMode( RF_OPMODE_TRANSMITTER );
		DelayMs(100);
		TIM1->CR1|=0x01;   // start timer
		
		
		
		
//		for(m=0;m<LORA_PREAMBLE_LENGTH;m++)
//		{
//			LoRa_UpChirp();
//			HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_2);
//		}
		
//		for(m=0;m<LORA_ID_LENGTH;m++)
//		{
//			LoRa_Payload( LoRa_ID_Start_Freq[m]);
//			HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_2);
//		}
//		for(m=0;m<LORA_SFD_LENGTH;m++)
//		{
//			LoRa_DownChirp();
//			HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_2);
//		}
//		
//		Generate_Quarter_DownChirp();
//		HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_2);
//		for(m=0;m<LORA_PAYLOAD_LENGTH;m++)
//		{
//			LoRa_Payload( LoRa_Payload_Start_Freq[m]);
//			HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_2);
//		}
		SX1276SetOpMode( RF_OPMODE_STANDBY );
		TIM1->CR1|=0x00;
		DelayMs(2000);
		
  }
}








void Generate_chip( uint32_t freq )
{
//	Fast_SetChannel( freq );
}



