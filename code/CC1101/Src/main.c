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

#include "LoRa_Channel_Coding.h"

#define BUFFER_SIZE                                 255// Define the payload size here
uint8_t Tx_Buffer[BUFFER_SIZE]={
		0xFF
};

uint16_t BufferSize = BUFFER_SIZE;

int main(void)
{
  uint16_t datarate,i=0;
	
	int *packet_freq_points_No1 = NULL;
	int *packet_freq_points_No2 = NULL;
	int symbol_len_No1 = NULL;
	int symbol_len_No2 = NULL;
	
	int *LoRa_ID_Start_Freq_No1 = NULL;
	int *LoRa_Payload_Start_Freq_No1 = NULL;

	int *LoRa_ID_Start_Freq_No2 = NULL;
	int *LoRa_Payload_Start_Freq_No2 = NULL;
	
	
	HAL_Init();
  SystemClock_Config();
	
	HW_SPI_Init();
	SPI1_Init();
	
	delay_init(80);
	uart_init(115200);
	
	Control_GPIO_Init();
	
	TIM2_Init(0xffffffff,80-1);       //Timer resolution = 1us; auto-reload value = 0xfffff
	
	CC1101_Init();
	
	for (int j = 0; j < BufferSize; j++)
	{
//			Tx_Buffer[j] = rand()%255;
		Tx_Buffer[j] = 0x31;
	}
	
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
	
	printf("Tx CC1101\r\n");
	printf("CR=4/%d, CRC=%s, IMPL_HEAD=%s, LDR=%s\n",4+LORA_CR_NO1,LORA_HAS_CRC_NO1?"ON":"OFF",LORA_IMPL_HEAD_NO1?"ON":"OFF",LORA_LOWDATERATEOPTIMIZE_NO1?"ON":"OFF");
	printf("FREQ1:%d,sf1:%d,\r\nFREQ2:%d,sf2:%d\r\n",RF_FREQUENCY_NO1,LORA_SF_NO1,RF_FREQUENCY_NO2,LORA_SF_NO2);
	
  for(i=0;i<1000;i++)
	{
		packet_freq_points_No1 = LoRa_Channel_Coding(Tx_Buffer, BufferSize, LORA_BW, LORA_SF_NO1, LORA_CR_NO1, LORA_HAS_CRC_NO1, LORA_IMPL_HEAD_NO1, &symbol_len_No1, LORA_LOWDATERATEOPTIMIZE_NO1);
  
		LoRa_Generate_Signal(packet_freq_points_No1,symbol_len_No1);
		
		free(packet_freq_points_No1);
		
		printf("Tx done, Count:%d\r\n",i+1);
		delay_ms(1000);
	}
}


