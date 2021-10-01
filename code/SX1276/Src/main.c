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

//#define CODING

#define BUFFER_SIZE                                 255// Define the payload size here
#define PACKET_COUNT																1000	//
#define INTERVAL_TIME																150  // ms


#define MODE  																			2 // #1 LOOK, LOOK run as normal lora
																											// #2 LOOK_BLANK, insert blank in symbols
																											// #3 LOOK_DOUBLE, transmit two packets in diffenert channels at the same time

#if (MODE==1) 																			
	#define LOOK
#elif (MODE==2)
	#define LOOK_BLANK
	#define LOOK_BLANK_RATIO													0.8  // This para means x% turn on PA and (1-x)% turn off PA.
#elif (MODE==3)
	#define LOOK_DOUBLE
#endif

#define ENABLE_USART

int PC_center_freq = 433000000;
int PC_tx_power = 14;
int PC_bandwidth = 125;
int PC_preamble_length = 8;
int PC_invert_iq = 0;
		
int PC_sync_words = 1234;
int PC_spread_factor = 7;
int PC_coding_rate = 1;
int PC_CRC = 0;
int PC_implicit_header =  0;
int PC_lowdatarateoptimize = 0;
		
int PC_packets_times = 0;
int PC_payload_length = 0;




static RadioEvents_t RadioEvents;
																											
extern TIM_HandleTypeDef TIM2_Handler;
extern uint32_t time_count;
extern uint32_t Time_temp;

uint8_t data_1[21] = {0x80, 0x37, 0x00, 0x1e, 0x00, 0x00, 0x01, 0x00, 0x02, 0x79, 0x91, 0x05, 0x55, 0x29, 0x3b, 0x6b, 0xab, 0xd7, 0x47, 0xc0, 0x49};
uint8_t data_2[21] = {0x80, 0x2c, 0x00, 0x42, 0x00, 0x00, 0x01, 0x00, 0x02, 0xbe, 0x52, 0xf5, 0x2c, 0xe3, 0xf6, 0xc0, 0xe4, 0x11, 0xd1, 0xb1, 0x6b};

uint8_t Tx_Buffer[BUFFER_SIZE]={0};

uint16_t BufferSize = BUFFER_SIZE;

int main(void)
{
  uint16_t datarate,i,len,t;
	uint8_t  USART_temp[USART_REC_LEN];
	int temp=0;
	char * str_header,* substr;
	char *delim = "_";
	
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
#ifdef ENABLE_USART
	uart_init(115200);
#endif
//	
	Control_GPIO_Init();
	LPM_SetOffMode(LPM_APPLI_Id, LPM_Disable);

	Radio.Init(&RadioEvents);
  Radio.SetChannel(RF_FREQUENCY - LORA_BW);
	Radio.SetTxContinuousWave(RF_FREQUENCY,TX_OUTPUT_POWER,3);

	SX1276Write( REG_OSC, RF_OSC_CLKOUT_1_MHZ );
	
	SX1276Write( REG_PLLHOP, ( SX1276Read( REG_PLLHOP ) & RF_PLLHOP_FASTHOP_MASK ) | RF_PLLHOP_FASTHOP_ON );
	SX1276Write( REG_PLL, ( SX1276Read( REG_PLL ) & RF_PLL_BANDWIDTH_MASK ) | RF_PLL_BANDWIDTH_150 );
	SX1276Write( REG_PARAMP, ( SX1276Read( REG_PARAMP ) & RF_PARAMP_MASK ) | RF_PARAMP_0010_US );
	SX1276Write( REG_PARAMP, ( SX1276Read( REG_PARAMP ) & RF_PARAMP_MODULATIONSHAPING_MASK ) | RF_PARAMP_MODULATIONSHAPING_00 );
	SX1276Write( REG_OCP, ( SX1276Read( REG_OCP ) & RF_OCP_MASK ) | RF_OCP_OFF );
	
	datarate = ( uint16_t )( ( double )XTAL_FREQ / ( double )DATA_RATE );
	SX1276Write( REG_BITRATEMSB, ( uint8_t )( datarate >> 8 ) );
  SX1276Write( REG_BITRATELSB, ( uint8_t )( datarate & 0xFF ) );
	memset(USART_RX_BUF, 0, USART_REC_LEN);
	
	printf("init finish!!\r\n");
	while(1)
	{
		USART_RX_STA=0;
		memset(USART_RX_BUF, 0, USART_REC_LEN);
		while(1)
		{
//			printf("Tx:waiting settings...\n");
			if(USART_RX_STA&0x8000)
			{
				if(strstr((char*)USART_RX_BUF,"PL") != NULL)
				{
					printf((char*)USART_RX_BUF);
					USART_RX_STA=0;
					break;
				}
				USART_RX_STA=0;
			}
			delay_ms(1000);
		}
//		printf("Mode:%d\n",MODE);
		delay_ms(1000);
		temp = 0;
		str_header = strstr((char*)USART_RX_BUF,"PL");
		substr = strtok(str_header, delim);
		len = strlen(substr);
		for(int i=0;i<len-2;i++)
		{
			temp += (int)((substr[len-1-i] - '0') * pow(10,i));
		}
		PC_payload_length = temp;
		
		temp = 0;
		str_header = strstr((char*)substr,"SF");
		substr = strtok(str_header, delim);
		len = strlen(substr);
		for(int i=0;i<len-2;i++)
		{
			temp += (int)((substr[len-1-i] - '0') * pow(10,i));
		}
		PC_spread_factor = temp;
		
		temp = 0;
		str_header = strstr((char*)substr,"CR");
		substr = strtok(str_header, delim);
		len = strlen(substr);
		for(int i=0;i<len-2;i++)
		{
			temp += (int)((substr[len-1-i] - '0') * pow(10,i));
		}
		PC_coding_rate = temp;
		
		temp = 0;
		str_header = strstr((char*)substr,"CRC");
		substr = strtok(str_header, delim);
		len = strlen(substr);
		for(int i=0;i<len-3;i++)
		{
			temp += (int)((substr[len-1-i] - '0') * pow(10,i));
		}
		PC_CRC = temp;
		
		temp = 0;
		str_header = strstr((char*)substr,"IH");
		substr = strtok(str_header, delim);
		len = strlen(substr);
		for(int i=0;i<len-3;i++)
		{
			temp += (int)((substr[len-1-i] - '0') * pow(10,i));
		}
		PC_implicit_header = temp;
		
		temp = 0;
		str_header = strstr((char*)substr,"LDO");
		substr = strtok(str_header, delim);
		len = strlen(substr);
		for(int i=0;i<len-3;i++)
		{
			temp += (int)((substr[len-1-i] - '0') * pow(10,i));
		}
		PC_lowdatarateoptimize = temp;
		if(PC_spread_factor>=11)
			PC_lowdatarateoptimize = 1;
		else
			PC_lowdatarateoptimize = 0;
		
//		printf("\nPayload length:%d,SF:%d,CR:%d,CRC:%d,IH:%d,LDO:%d\n",PC_payload_length,PC_spread_factor,PC_coding_rate,PC_CRC,PC_implicit_header,PC_lowdatarateoptimize);

		int packets_count = 0;
	
		USART_RX_STA=0;
		while(1)
		{
//			printf("Tx:waiting payload data...\n");
			if(USART_RX_STA&0x8000)
			{
				len=USART_RX_STA&0x3fff;
				if(strstr((char*)USART_RX_BUF,"END") != NULL)
				{
					printf("reset\r\n");
					break;
				}
				else if(strstr((char*)USART_RX_BUF,"PD") != NULL)
				{
//					printf((char*)USART_RX_BUF);
					
					str_header = strstr((char*)USART_RX_BUF,"PD");
					str_header += 2;
//					printf("\n");
					for(int j=0;j<PC_payload_length;j++)
					{
						str_header = strtok(str_header, delim);
						len = strlen(str_header);
						temp = 0;
						for(int i=0;i<len;i++)
						{
							temp += (int)((str_header[len-1-i] - '0') * pow(10,i));
						}
						Tx_Buffer[j] = temp;
						str_header = str_header+len+1;
						
					}
					USART_RX_STA=0;
				}
				else if(strstr((char*)USART_RX_BUF,"CONFIRMED") != NULL)
				{

				
					packet_freq_points_No1 = LoRa_Channel_Coding(Tx_Buffer, PC_payload_length, LORA_BW, PC_spread_factor, PC_coding_rate, \
																												PC_CRC?true:false, PC_implicit_header?true:false, \
																													&symbol_len_No1, PC_lowdatarateoptimize?true:false);
			
					#ifdef LOOK_DOUBLE
					packet_freq_points_No2 = LoRa_Channel_Coding(Tx_Buffer, PC_payload_length, LORA_BW, PC_spread_factor, PC_coding_rate, \
																												PC_CRC?true:false, PC_implicit_header?true:false, \
																													&symbol_len_No1, PC_lowdatarateoptimize?true:false);
					#endif
					
					#ifdef LOOK
					LoRa_Generate_Signal(packet_freq_points_No1,symbol_len_No1,PC_spread_factor);
					#endif
																												
					#ifdef LOOK_BLANK
					LoRa_Generate_Signal_With_Blank(packet_freq_points_No1,symbol_len_No1,LOOK_BLANK_RATIO);
					#endif
					#ifdef LOOK_DOUBLE
					LoRa_Generate_Double_Packet(packet_freq_points_No1,symbol_len_No1,packet_freq_points_No2,symbol_len_No2,PC_spread_factor);
					#endif
					
					free(packet_freq_points_No1);
					#ifdef LOOK_DOUBLE
					free(packet_freq_points_No2);
					#endif
																										
					delay_ms(200);

					packets_count++;
					printf("Tx:done, count:%d\n",packets_count);
					USART_RX_STA=0;
				}
			}
		}

	}

}


