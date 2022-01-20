#include "stm32l4xx.h"
#include "stm32l4xx_hal.h"
#include <string.h>
#include "hw.h"

#include "CC1125.h"
#include "CC1125_Regs.h"


#include "timer.h"
#include "fast_spi.h"
#include "rtc.h"
#include "delay.h"
#include "usart.h"

#include "Simulated_LoRa.h"
#include "control_GPIO.h"

#include "LoRa_Channel_Coding.h"

#define BUFFER_SIZE                                 51// Define the payload size here
#define PACKET_COUNT																100	//
#define INTERVAL_TIME																1000 // ms


#define MODE  																			3 // #1 LOOK, LOOK run as normal lora
																											// #2 LOOK_BLANK, insert blank in symbols
																											// #3 LOOK_DOUBLE, transmit two packets in diffenert channels at the same time

#if (MODE==1) 																			
	#define LOOK
#elif (MODE==2)
	#define LOOK_BLANK
	#define LOOK_BLANK_RATIO														0.65  // This para means x% turn on PA and (1-x)% turn off PA.
#elif (MODE==3)
	#define LOOK_DOUBLE
#endif

#define ENABLE_USART
																											
extern TIM_HandleTypeDef TIM2_Handler;
extern uint32_t time_count;
extern uint32_t Time_temp;

uint8_t data_1[21] = {0x80, 0x37, 0x00, 0x1e, 0x00, 0x00, 0x01, 0x00, 0x02, 0x79, 0x91, 0x05, 0x55, 0x29, 0x3b, 0x6b, 0xab, 0xd7, 0x47, 0xc0, 0x49};
uint8_t data_2[21] = {0x80, 0x2c, 0x00, 0x42, 0x00, 0x00, 0x01, 0x00, 0x02, 0xbe, 0x52, 0xf5, 0x2c, 0xe3, 0xf6, 0xc0, 0xe4, 0x11, 0xd1, 0xb1, 0x6b};

uint8_t data_0x001D004E[64]={
0x40, 0x4E, 0x00, 0x1D, 0x00, 0x00, 0x01, 0x00, 0x02, 0x50, 0xEE, 0xEC, 0x5F, 0x44, 0x0C, 0xFA, 0xC3, 0x36, 0xC8, 0xD6, 0x28, 0x90, 0xBC, 0x6E, 0xAC, 0x66, 0x03, 0xEF, 0x19, 0xCC, 0x36, 0x9C, 0x5A, 0xF9, 0xF0, 0x22, 0x02, 0xB3, 0xEE, 0x4D, 0x7B, 0x77, 0x70, 0xBB, 0x12, 0xAE, 0x85, 0x73, 0xA6, 0x5C, 0x4A, 0x46, 0xEA, 0x13, 0x61, 0x8E, 0x19, 0xA2, 0xC5, 0x81, 0x53, 0x65, 0x1F, 0x3F
};
	
uint8_t Tx_Buffer[BUFFER_SIZE]={0};

uint16_t BufferSize = BUFFER_SIZE;

int main(void)
{
	uint16_t i=0;
	
	int *packet_freq_points_No1 = NULL;
	int symbol_len_No1 = NULL;
#ifdef LOOK_DOUBLE
	int *packet_freq_points_No2 = NULL;
	int symbol_len_No2 = NULL;
#endif
	
	HAL_Init();
  SystemClock_Config();
	
	HW_SPI_Init();
	SPI1_Init();
	
	delay_init(80);
#ifdef ENABLE_USART
	uart_init(115200);
#endif
	Control_GPIO_Init();
	
	CC1125_Init();	

#ifdef ENABLE_USART
	printf("Tx\r\n");
	printf("CR=4/%d, CRC=%s, IMPL_HEAD=%s, LDR=%s\n",4+LORA_CR_NO1,LORA_HAS_CRC_NO1?"ON":"OFF",LORA_IMPL_HEAD_NO1?"ON":"OFF",LORA_LOWDATERATEOPTIMIZE_NO1?"ON":"OFF");
	printf("FREQ1:%d,sf1:%d,\r\nFREQ2:%d,sf2:%d\r\n",RF_FREQUENCY_NO1,LORA_SF_NO1,RF_FREQUENCY_NO2,LORA_SF_NO2);
#endif
	for (int j = 0; j < BufferSize; j++)
	{
		Tx_Buffer[j] = 0x31;
	}
	
	for(i=0;i<PACKET_COUNT;i++)
	{
		
		packet_freq_points_No1 = LoRa_Channel_Coding(data_0x001D004E, 64, LORA_BW, LORA_SF_NO1, LORA_CR_NO1, LORA_HAS_CRC_NO1, LORA_IMPL_HEAD_NO1, &symbol_len_No1, LORA_LOWDATERATEOPTIMIZE_NO1);

		#ifdef LOOK_DOUBLE
		packet_freq_points_No2 = LoRa_Channel_Coding(data_0x001D004E, 64, LORA_BW, LORA_SF_NO2, LORA_CR_NO2, LORA_HAS_CRC_NO2, LORA_IMPL_HEAD_NO2, &symbol_len_No2, LORA_LOWDATERATEOPTIMIZE_NO2);
		#endif
		
		#ifdef LOOK
		LoRa_Generate_Signal(packet_freq_points_No1,symbol_len_No1);
		#endif
		#ifdef LOOK_BLANK
		LoRa_Generate_Signal_With_Blank(packet_freq_points_No1,symbol_len_No1,LOOK_BLANK_RATIO);
		#endif
		#ifdef LOOK_DOUBLE
		LoRa_Generate_Double_Packet(packet_freq_points_No1,symbol_len_No1,packet_freq_points_No2,symbol_len_No2);
		#endif
		
		free(packet_freq_points_No1);
		#ifdef LOOK_DOUBLE
		free(packet_freq_points_No2);
		#endif
		
		#ifdef ENABLE_USART
		printf("Tx done, Count:%d\r\n",i+1);
		#endif
		delay_ms(INTERVAL_TIME);
	}
#ifdef ENABLE_USART
	printf("finish!!\r\n");
#endif
  }


