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

uint8_t cr = 4;
uint8_t sf = 7;


static RadioEvents_t RadioEvents;
																											
extern TIM_HandleTypeDef TIM2_Handler;
extern uint32_t time_count;
extern uint32_t Time_temp;


int main(void)
{
  uint16_t datarate,i;
	
	char *str = "Hello world!!"; 
	uint8_t *whitened_data;
	uint8_t *add_header_data;
	uint8_t *add_CRC;
	uint8_t *hanmingcode_data;
	uint8_t *interleaver_data;
	uint8_t *gray_data;
	
	uint8_t noutput_add_CRC=0;
	uint8_t noutput_hanmming_coding = 0;
	uint8_t noutput_interleaver = 0;
	uint8_t noutput_gray = 0;
	
	
	HAL_Init();
  SystemClock_Config();

  HW_Init();
	
	SPI1_Init();
	delay_init(80);
	uart_init(115200);

	Control_GPIO_Init();
	
	
	printf("\n------------------Whitening-----------------------\n");
	
	whitened_data = Whitening(str);
	printf("Len of Output:%d\n",2*strlen(str));
	for(i=0;i<2*strlen(str);i++)
	{
		printf("Out[%d]:%x (hex)\n",i,whitened_data[i]);
	}
	printf("\n------------------Add header-----------------------\n");
	
	add_header_data = Add_Header(false, has_crc, cr, str, whitened_data);
	
	printf("Len of Output:%d\n",2*strlen(str)+5);
	for(i=0;i<2*strlen(str)+5;i++)
	{
		printf("Out[%d]:%x (hex)\n",i,add_header_data[i]);
	}
	
	printf("\n------------------Add CRC-----------------------\n");
	
	add_CRC = Add_CRC(has_crc, str, add_header_data, 2*strlen(str)+5, &noutput_add_CRC);
	
	printf("Len of Output:%d\n",noutput_add_CRC);
	for(i=0;i<noutput_add_CRC;i++)
	{
		printf("Out[%d]:%x (hex)\n",i,add_CRC[i]);
	}
	printf("\n------------------Hanmming coding-----------------------\n");
	
	hanmingcode_data = Hanmming_Enc(cr, sf, str, add_CRC, noutput_add_CRC, &noutput_hanmming_coding);
	
	printf("Len of Output:%d\n",noutput_hanmming_coding);
	for(i=0;i<noutput_hanmming_coding;i++)
	{
		printf("Out[%d]:%x (hex)\n",i,hanmingcode_data[i]);
	}
	
	printf("\n------------------Interleaver-----------------------\n");
	
	interleaver_data = Interleaver(cr, sf, str, hanmingcode_data, noutput_hanmming_coding, &noutput_interleaver);
	
	printf("Len of Output:%d\n",noutput_interleaver);
	for(i=0;i<noutput_interleaver;i++)
	{
		printf("Out[%d]:%x (hex)\n",i,interleaver_data[i]);
	}
	
	printf("\n------------------Gray coder-----------------------\n");
	
	gray_data = Gray_Decoder(cr, sf, str, interleaver_data, noutput_interleaver, &noutput_gray);
	
	printf("Len of Output:%d\n",noutput_gray);
	for(i=0;i<noutput_interleaver;i++)
	{
		printf("Out[%d]:%x (hex)\n",i,gray_data[i]);
	}
	
	free(whitened_data);
	free(add_header_data);
	free(add_CRC);
	free(hanmingcode_data);
	free(interleaver_data);
	free(gray_data);
	
	
	/*Disbale Stand-by mode*/
//  LPM_SetOffMode(LPM_APPLI_Id, LPM_Disable);
	
//	TIM2_Init(0xffffffff,80-1);       //Timer resolution = 1us; auto-reload value = 0xfffff
//	
//	Radio.Init(&RadioEvents);
//  Radio.SetChannel(RF_FREQUENCY);
//	Radio.SetTxContinuousWave(RF_FREQUENCY,TX_OUTPUT_POWER,3);
//	SX1276Write( REG_PLLHOP, ( SX1276Read( REG_PLLHOP ) & RF_PLLHOP_FASTHOP_MASK ) | RF_PLLHOP_FASTHOP_ON );
//	SX1276Write( REG_PARAMP, ( SX1276Read( REG_PARAMP ) & RF_PARAMP_MASK ) | RF_PARAMP_0010_US );
//	SX1276Write( REG_OCP, ( SX1276Read( REG_OCP ) & RF_OCP_MASK ) | RF_OCP_OFF );
//	
//	datarate = ( uint16_t )( ( double )XTAL_FREQ / ( double )DATA_RATE );
//	SX1276Write( REG_BITRATEMSB, ( uint8_t )( datarate >> 8 ) );
//  SX1276Write( REG_BITRATELSB, ( uint8_t )( datarate & 0xFF ) );
//	printf("Tx\r\n");
//	printf("FREQ1:%d,sf1:%d,\r\nFREQ2:%d,sf2:%d\r\n",RF_FREQUENCY,LORA_SF_NO1,RF_FREQUENCY+FREQ_OFFSET_1_2,LORA_SF_NO2);
//	
////  while (1)
////  {
////		printf("Start\r\n");
//		for(i=0;i<100;i++)
//		{
//			LoRa_Generate_Signal();
//			delay_ms(500);
//			printf("Tx done, Count:%d\r\n",i+1);
//		}
////		printf("Done\r\n");
////  }
//		printf("finish!!\r\n");
}


