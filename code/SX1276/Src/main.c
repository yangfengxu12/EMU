#include "stm32l4xx.h"
#include <string.h>
#include <stdlib.h>
#include "hw.h"
#include "radio.h"
#include "timeServer.h"
#include "low_power_manager.h"

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


#define has_crc   true

static RadioEvents_t RadioEvents;
																											
extern TIM_HandleTypeDef TIM2_Handler;
extern uint32_t time_count;
extern uint32_t Time_temp;





int main(void)
{
  uint16_t datarate,i;
	
	char *str_tx = "1";

	int *packet_freq_points_No1;
	int *packet_freq_points_No2;
	
	int symbol_len_No1;
	int symbol_len_No2;
	
	int *LoRa_ID_Start_Freq_No1;
	int *LoRa_Payload_Start_Freq_No1;

	int *LoRa_ID_Start_Freq_No2;
	int *LoRa_Payload_Start_Freq_No2;
	
	
	HAL_Init();
  SystemClock_Config();

  HW_Init();
	
	SPI1_Init();
	delay_init(80);
	uart_init(115200);

	Control_GPIO_Init();
	
//	packet_freq_points_No1 = LoRa_Channel_Coding(str_tx, LORA_BW, LORA_SF_NO1, LORA_CR_NO1, LORA_HAS_CRC_NO1, LORA_IMPL_HEAD_NO1, &symbol_len_No1 );

////	packet_freq_points_No2 = LoRa_Channel_Coding(str_tx, LORA_BW, LORA_SF_NO2, LORA_CR_NO2, LORA_HAS_CRC_NO2, LORA_IMPL_HEAD_NO2, &symbol_len_No2);
//	
//	LoRa_ID_Start_Freq_No1= calloc(2,sizeof(int));
//	LoRa_Payload_Start_Freq_No1 = calloc((symbol_len_No1-2),sizeof(int));
//	
//	memcpy(LoRa_ID_Start_Freq_No1,packet_freq_points_No1,2*sizeof(int));
//	memcpy(LoRa_Payload_Start_Freq_No1,packet_freq_points_No1+2,(symbol_len_No1-2)*sizeof(int));
//	
//	printf("Len of Output:%d\n",symbol_len_No1);
//	for(i=0;i<symbol_len_No1;i++)
//	{
//		printf("Out[%d]:%d (float)\n",i,packet_freq_points_No1[i]);
//	}
//	
//	
//	
//	printf("Len of Output:%d\n",2);
//	for(i=0;i<2;i++)
//	{
//		printf("Out[%d]:%d (float)\n",i,LoRa_ID_Start_Freq_No1[i]);
//	}

//	printf("Len of Output:%d\n",symbol_len_No1-2);
//	for(i=0;i<symbol_len_No1-2;i++)
//	{
//		printf("Out[%d]:%d (float)\n",i,LoRa_Payload_Start_Freq_No1[i]);
//	}
//	
//	coding_phy_midware(LoRa_ID_Start_Freq_No1,LoRa_Payload_Start_Freq_No1);
	
	
	
	/*Disbale Stand-by mode*/
  LPM_SetOffMode(LPM_APPLI_Id, LPM_Disable);
	
	TIM2_Init(0xffffffff,80-1);       //Timer resolution = 1us; auto-reload value = 0xfffff
	
	Radio.Init(&RadioEvents);
  Radio.SetChannel(RF_FREQUENCY);
	Radio.SetTxContinuousWave(RF_FREQUENCY,TX_OUTPUT_POWER,3);
	SX1276Write( REG_PLLHOP, ( SX1276Read( REG_PLLHOP ) & RF_PLLHOP_FASTHOP_MASK ) | RF_PLLHOP_FASTHOP_ON );
	SX1276Write( REG_PARAMP, ( SX1276Read( REG_PARAMP ) & RF_PARAMP_MASK ) | RF_PARAMP_0010_US );
	SX1276Write( REG_OCP, ( SX1276Read( REG_OCP ) & RF_OCP_MASK ) | RF_OCP_OFF );
	
	datarate = ( uint16_t )( ( double )XTAL_FREQ / ( double )DATA_RATE );
	SX1276Write( REG_BITRATEMSB, ( uint8_t )( datarate >> 8 ) );
  SX1276Write( REG_BITRATELSB, ( uint8_t )( datarate & 0xFF ) );
	printf("Tx\r\n");
	printf("FREQ1:%d,sf1:%d,\r\nFREQ2:%d,sf2:%d\r\n",RF_FREQUENCY,LORA_SF_NO1,RF_FREQUENCY+FREQ_OFFSET_1_2,LORA_SF_NO2);
	
//  while (1)
//  {
//		printf("Start\r\n");
		for(i=0;i<100;i++)
		{
			LoRa_Generate_Signal();
			delay_ms(500);
			printf("Tx done, Count:%d\r\n",i+1);
		}
//		printf("Done\r\n");
//  }
		printf("finish!!\r\n");
}


