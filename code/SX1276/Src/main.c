#include "stm32l4xx.h"
#include <string.h>
#include <stdlib.h>
#include "hw.h"
#include "radio.h"
#include "timeServer.h"
#include "low_power_manager.h"
//#include "malloc.h" 
#include "sx1276.h"
#include "sx1276mb1mas.h"

#include "timer.h"
#include "fast_spi.h"
#include "rtc.h"
#include "delay.h"
#include "usart.h"

#include "Simulated_LoRa.h"
#include "LoRa_Channel_Coding.h"
#include "Timer_Calibration.h"
#include "control_GPIO.h"

//1.00014136 = 1      +2us

//#define CODING

static RadioEvents_t RadioEvents;
																											
extern TIM_HandleTypeDef TIM2_Handler;
extern uint32_t time_count;
extern uint32_t Time_temp;



#define BUFFER_SIZE                                 1 // Define the payload size here

uint8_t Tx_Buffer[BUFFER_SIZE]={0xDE};

uint16_t BufferSize = BUFFER_SIZE;

int main(void)
{
  uint16_t datarate,i;
	
	for (i = 0; i < BufferSize; i++)
	{
		if(i == 0)	
			Tx_Buffer[i] = 0x31 ;
		else
			Tx_Buffer[i] = 0x31 ;
	}
	
	
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

  HW_Init();
//	my_mem_init(SRAMIN);
	
	SPI1_Init();
	delay_init(80);
	uart_init(115200);

	Control_GPIO_Init();
	/*Disbale Stand-by mode*/
	LPM_SetOffMode(LPM_APPLI_Id, LPM_Disable);
	
	TIM2_Init(0xffffffff,80-1);       //Timer resolution = 1us; auto-reload value = 0xffffffff
	
	Radio.Init(&RadioEvents);
  Radio.SetChannel(RF_FREQUENCY);
	Radio.SetTxContinuousWave(RF_FREQUENCY,TX_OUTPUT_POWER,3);
	SX1276Write( REG_PLLHOP, ( SX1276Read( REG_PLLHOP ) & RF_PLLHOP_FASTHOP_MASK ) | RF_PLLHOP_FASTHOP_ON );
	SX1276Write( REG_PARAMP, ( SX1276Read( REG_PARAMP ) & RF_PARAMP_MASK ) | RF_PARAMP_0010_US );
	SX1276Write( REG_OCP, ( SX1276Read( REG_OCP ) & RF_OCP_MASK ) | RF_OCP_OFF );
	
	datarate = ( uint16_t )( ( double )XTAL_FREQ / ( double )DATA_RATE );
	SX1276Write( REG_BITRATEMSB, ( uint8_t )( datarate >> 8 ) );
  SX1276Write( REG_BITRATELSB, ( uint8_t )( datarate & 0xFF ) );

	

	packet_freq_points_No1 = LoRa_Channel_Coding(Tx_Buffer, BufferSize, LORA_BW, LORA_SF_NO1, LORA_CR_NO1, LORA_HAS_CRC_NO1, LORA_IMPL_HEAD_NO1, &symbol_len_No1 );

	
	printf("Tx\r\n");
	printf("FREQ1:%d,sf1:%d,\r\nFREQ2:%d,sf2:%d\r\n",RF_FREQUENCY,LORA_SF_NO1,RF_FREQUENCY+FREQ_OFFSET_1_2,LORA_SF_NO2);
	
		for(i=0;i<100;i++)
		{
			LoRa_Generate_Signal(packet_freq_points_No1,symbol_len_No1);
			
			printf("Tx done, Count:%d\r\n",i+1);
			delay_ms(500);
		}
		printf("finish!!\r\n");
}


