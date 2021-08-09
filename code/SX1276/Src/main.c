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
#include "control_GPIO.h"

//#include "Timer_Calibration.h"
#include "Timer_Calibration_From_SX1276.h"


//1.00014136 = 1      +2us

//#define CODING

static RadioEvents_t RadioEvents;
																											
extern TIM_HandleTypeDef TIM2_Handler;
extern uint32_t time_count;
extern uint32_t Time_temp;


#define BUFFER_SIZE                                 255// Define the payload size here

uint8_t Tx_Buffer[BUFFER_SIZE]={0};

uint16_t BufferSize = BUFFER_SIZE;

int main(void)
{
  uint16_t datarate,i;
	
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
	
	SPI1_Init();
	delay_init(80);
	uart_init(115200);
//	
	Control_GPIO_Init();
//	/*Disbale Stand-by mode*/
	LPM_SetOffMode(LPM_APPLI_Id, LPM_Disable);

	Radio.Init(&RadioEvents);
  Radio.SetChannel(RF_FREQUENCY - LORA_BW);
	Radio.SetTxContinuousWave(RF_FREQUENCY,TX_OUTPUT_POWER,3);

	SX1276Write( REG_OSC, RF_OSC_CLKOUT_1_MHZ );
	
	SX1276Write( REG_PLLHOP, ( SX1276Read( REG_PLLHOP ) & RF_PLLHOP_FASTHOP_MASK ) | RF_PLLHOP_FASTHOP_ON );
	SX1276Write( REG_PLL, ( SX1276Read( REG_PLL ) & RF_PLL_BANDWIDTH_MASK ) | RF_PLL_BANDWIDTH_150 );
	SX1276Write( REG_PARAMP, ( SX1276Read( REG_PARAMP ) & RF_PARAMP_MASK ) | RF_PARAMP_0010_US );
	SX1276Write( REG_PARAMP, ( SX1276Read( REG_PARAMP ) & RF_PARAMP_MODULATIONSHAPING_MASK ) | RF_PARAMP_MODULATIONSHAPING_01 );
	SX1276Write( REG_OCP, ( SX1276Read( REG_OCP ) & RF_OCP_MASK ) | RF_OCP_OFF );
	
	datarate = ( uint16_t )( ( double )XTAL_FREQ / ( double )DATA_RATE );
	SX1276Write( REG_BITRATEMSB, ( uint8_t )( datarate >> 8 ) );
  SX1276Write( REG_BITRATELSB, ( uint8_t )( datarate & 0xFF ) );
	
	printf("Tx\r\n");
	printf("CR=4/%d, CRC=%s, IMPL_HEAD=%s, LDR=%s\n",4+LORA_CR_NO1,LORA_HAS_CRC_NO1?"ON":"OFF",LORA_IMPL_HEAD_NO1?"ON":"OFF",LORA_LOWDATERATEOPTIMIZE_NO1?"ON":"OFF");
	printf("FREQ1:%d,sf1:%d,\r\nFREQ2:%d,sf2:%d\r\n",RF_FREQUENCY_NO1,LORA_SF_NO1,RF_FREQUENCY_NO2,LORA_SF_NO2);
	
	for(i=0;i<1000;i++)
	{
		for (int j = 0; j < BufferSize; j++)
		{
//			Tx_Buffer[j] = rand()%255;
			Tx_Buffer[j] = 0x31;
		}
		
		packet_freq_points_No1 = LoRa_Channel_Coding(Tx_Buffer, BufferSize, LORA_BW, LORA_SF_NO1, LORA_CR_NO1, LORA_HAS_CRC_NO1, LORA_IMPL_HEAD_NO1, &symbol_len_No1, LORA_LOWDATERATEOPTIMIZE_NO1);
		packet_freq_points_No2 = LoRa_Channel_Coding(Tx_Buffer, BufferSize, LORA_BW, LORA_SF_NO2, LORA_CR_NO2, LORA_HAS_CRC_NO2, LORA_IMPL_HEAD_NO2, &symbol_len_No2, LORA_LOWDATERATEOPTIMIZE_NO2);
		
//		LoRa_Generate_Signal(packet_freq_points_No1,symbol_len_No1);
		LoRa_Generate_Double_Packet(packet_freq_points_No1,symbol_len_No1,packet_freq_points_No2,symbol_len_No2);
		
		free(packet_freq_points_No1);
		free(packet_freq_points_No2);
		printf("Tx done, Count:%d\r\n",i+1);
		delay_ms(5000);
	}
	printf("finish!!\r\n");

}


