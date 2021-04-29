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

#include "Timer_Calibration.h"

#include "Timer_Calibration_From_SX1276.h"



static RadioEvents_t RadioEvents;
																											
extern TIM_HandleTypeDef TIM2_Handler;
extern uint32_t time_count;
extern uint32_t Time_temp;

const static uint8_t whitening[] = {
		0xFF, 0xFE, 0xFC, 0xF8, 0xF0, 0xE1, 0xC2, 0x85, 0x0B, 0x17, 0x2F, 0x5E, 0xBC, 0x78, 0xF1, 0xE3,
		0xC6, 0x8D, 0x1A, 0x34, 0x68, 0xD0, 0xA0, 0x40, 0x80, 0x01, 0x02, 0x04, 0x08, 0x11, 0x23, 0x47,
		0x8E, 0x1C, 0x38, 0x71, 0xE2, 0xC4, 0x89, 0x12, 0x25, 0x4B, 0x97, 0x2E, 0x5C, 0xB8, 0x70, 0xE0,
		0xC0, 0x81, 0x03, 0x06, 0x0C, 0x19, 0x32, 0x64, 0xC9, 0x92, 0x24, 0x49, 0x93, 0x26, 0x4D, 0x9B,
		0x37, 0x6E, 0xDC, 0xB9, 0x72, 0xE4, 0xC8, 0x90, 0x20, 0x41, 0x82, 0x05, 0x0A, 0x15, 0x2B, 0x56,
		0xAD, 0x5B, 0xB6, 0x6D, 0xDA, 0xB5, 0x6B, 0xD6, 0xAC, 0x59, 0xB2, 0x65, 0xCB, 0x96, 0x2C, 0x58,
		0xB0, 0x61, 0xC3, 0x87, 0x0F, 0x1F, 0x3E, 0x7D, 0xFB, 0xF6, 0xED, 0xDB, 0xB7, 0x6F, 0xDE, 0xBD,
		0x7A, 0xF5, 0xEB, 0xD7, 0xAE, 0x5D, 0xBA, 0x74, 0xE8, 0xD1, 0xA2, 0x44, 0x88, 0x10, 0x21, 0x43,
		0x86, 0x0D, 0x1B, 0x36, 0x6C, 0xD8, 0xB1, 0x63, 0xC7, 0x8F, 0x1E, 0x3C, 0x79, 0xF3, 0xE7, 0xCE,
		0x9C, 0x39, 0x73, 0xE6, 0xCC, 0x98, 0x31, 0x62, 0xC5, 0x8B, 0x16, 0x2D, 0x5A, 0xB4, 0x69, 0xD2,
		0xA4, 0x48, 0x91, 0x22, 0x45, 0x8A, 0x14, 0x29, 0x52, 0xA5, 0x4A, 0x95, 0x2A, 0x54, 0xA9, 0x53,
		0xA7, 0x4E, 0x9D, 0x3B, 0x77, 0xEE, 0xDD, 0xBB, 0x76, 0xEC, 0xD9, 0xB3, 0x67, 0xCF, 0x9E, 0x3D,
		0x7B, 0xF7, 0xEF, 0xDF, 0xBF, 0x7E, 0xFD, 0xFA, 0xF4, 0xE9, 0xD3, 0xA6, 0x4C, 0x99, 0x33, 0x66,
		0xCD, 0x9A, 0x35, 0x6A, 0xD4, 0xA8, 0x51, 0xA3, 0x46, 0x8C, 0x18, 0x30, 0x60, 0xC1, 0x83, 0x07,
		0x0E, 0x1D, 0x3A, 0x75, 0xEA, 0xD5, 0xAA, 0x55, 0xAB, 0x57, 0xAF, 0x5F, 0xBE, 0x7C, 0xF9, 0xF2,
		0xE5, 0xCA, 0x94, 0x28, 0x50, 0xA1, 0x42, 0x84, 0x09, 0x13, 0x27, 0x4F, 0x9F, 0x3F, 0x7F
};

#define BUFFER_SIZE                                 255 // Define the payload size here



uint8_t Tx_Buffer[BUFFER_SIZE]={
		0xFF
};

uint16_t BufferSize = BUFFER_SIZE;


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


uint32_t airtime_cal(int bw, int sf, int cr, int pktLen, int crc, int ih, int ldo)
{
	uint32_t airTime = 0;
	// Symbol rate : time for one symbol (secs)
	double rs = bw / ( 1 << sf );
	double ts = 1 / rs;
	// time of preamble
	double tPreamble = ( 8 + 4.25 ) * ts;
	// Symbol length of payload and time
	double tmp = ceil( ( 8 * pktLen - 4 * sf +
											 28 + 16 * crc -
											 ( ih ? 20 : 0 ) ) /
											 ( double )( 4 * ( sf -
											 ( ( ldo > 0 ) ? 2 : 0 ) ) ) ) *
											 ( cr + 4 );
	double nPayload = 8 + ( ( tmp > 0 ) ? tmp : 0 );
	double tPayload = nPayload * ts;
	// Time on air
	double tOnAir = tPreamble + tPayload;
	// return ms secs
	airTime = (uint32_t) floor( tOnAir * 1000 + 0.999 );
	return airTime;
}





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
	uart_init(115200);

	Control_GPIO_Init();
//	/*Disbale Stand-by mode*/
	LPM_SetOffMode(LPM_APPLI_Id, LPM_Disable);

	Radio.Init(&RadioEvents);
  Radio.SetChannel(RF_FREQUENCY);
	Radio.SetTxContinuousWave(RF_FREQUENCY,TX_OUTPUT_POWER,3);

	SX1276Write( REG_OSC, RF_OSC_CLKOUT_1_MHZ );
	
	SX1276Write( REG_PLLHOP, ( SX1276Read( REG_PLLHOP ) & RF_PLLHOP_FASTHOP_MASK ) | RF_PLLHOP_FASTHOP_ON );
	SX1276Write( REG_PARAMP, ( SX1276Read( REG_PARAMP ) & RF_PARAMP_MASK ) | RF_PARAMP_0010_US );
	SX1276Write( REG_OCP, ( SX1276Read( REG_OCP ) & RF_OCP_MASK ) | RF_OCP_OFF );
	
	datarate = ( uint16_t )( ( double )XTAL_FREQ / ( double )DATA_RATE );
	SX1276Write( REG_BITRATEMSB, ( uint8_t )( datarate >> 8 ) );
  SX1276Write( REG_BITRATELSB, ( uint8_t )( datarate & 0xFF ) );

//	packet_freq_points_No1 = LoRa_Channel_Coding(Tx_Buffer, BufferSize, LORA_BW, LORA_SF_NO1, LORA_CR_NO1, LORA_HAS_CRC_NO1, LORA_IMPL_HEAD_NO1, &symbol_len_No1, LORA_LOWDATERATEOPTIMIZE_NO1);

	
//	printf("Tx\r\n");
//	printf("CR=4/%d, CRC=%s, IMPL_HEAD=%s, LDR=%s\n",4+LORA_CR_NO1,LORA_HAS_CRC_NO1?"ON":"OFF",LORA_IMPL_HEAD_NO1?"ON":"OFF",LORA_LOWDATERATEOPTIMIZE_NO1?"ON":"OFF");
//	printf("FREQ1:%d,sf1:%d,\r\nFREQ2:%d,sf2:%d\r\n",RF_FREQUENCY,LORA_SF_NO1,RF_FREQUENCY+FREQ_OFFSET_1_2,LORA_SF_NO2);
//	
//	for(i=0;i<1000;i++)
//	{
//		LoRa_Generate_Signal(packet_freq_points_No1,symbol_len_No1);
//		
//		printf("Tx done, Count:%d\r\n",i+1);
//		delay_ms(8000);
//	}
	printf("init finish!!\r\n");
	memset(USART_RX_BUF, 0, USART_REC_LEN);
	while(1)
	{
		while(1)
		{
			printf("Tx:waiting connection\n");
			if(USART_RX_STA&0x8000)
			{
				len=USART_RX_STA&0x3fff;
				if(strstr((char*)USART_RX_BUF,"PC:Hello") != NULL)
				{
					printf("Tx:Hi!\n");
					USART_RX_STA=0;
					break;
				}
				else
				{
					printf("There is no Hello!\n");
				}
				USART_RX_STA=0;
			}
			delay_ms(2000);
		}
		
		delay_ms(500);
		memset(USART_RX_BUF, 0, USART_REC_LEN);
		USART_RX_STA=0;
		while(1)
		{
			printf("Tx:waiting settings...\n");
			if(USART_RX_STA&0x8000)
			{
				len=USART_RX_STA&0x3fff;
				if(strstr((char*)USART_RX_BUF,"PL") != NULL)
				{
					printf((char*)USART_RX_BUF);
					USART_RX_STA=0;
					break;
				}
				USART_RX_STA=0;
			}
			delay_ms(2000);
		}
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
		
	//	while(1){
		printf("\nPayload length:%d,SF:%d,CR:%d,CRC:%d,IH:%d,LDO:%d\n",PC_payload_length,PC_spread_factor,PC_coding_rate,PC_CRC,PC_implicit_header,PC_lowdatarateoptimize);
	//		delay_ms(2000);
	//	}
		int packets_count = 0;
		memset(USART_RX_BUF, 0, USART_REC_LEN);
		USART_RX_STA=0;
		while(1)
		{
			printf("Tx:waiting payload data...\n");
			if(USART_RX_STA&0x8000)
			{
				len=USART_RX_STA&0x3fff;
				if(strstr((char*)USART_RX_BUF,"END") != NULL)
				{
					break;
				}
				else if(strstr((char*)USART_RX_BUF,"PD") != NULL)
				{
					printf((char*)USART_RX_BUF);
					
					str_header = strstr((char*)USART_RX_BUF,"PD");
					str_header += 2;
					printf("\n");
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
						printf("%d\t",Tx_Buffer[j]);
					}
					printf("\n");
					printf("Tx:transmiting packets...\n");
					packet_freq_points_No1 = LoRa_Channel_Coding(Tx_Buffer, PC_payload_length, 125000, PC_spread_factor, \
																											PC_coding_rate, PC_CRC?true:false, PC_implicit_header?true:false, \
																												&symbol_len_No1, PC_lowdatarateoptimize?true:false);
																											
					LoRa_Generate_Signal(packet_freq_points_No1,symbol_len_No1,PC_spread_factor);
					
					delay_ms(1000+airtime_cal(125000, PC_spread_factor, PC_coding_rate, PC_payload_length, PC_CRC, PC_implicit_header, PC_lowdatarateoptimize));
					/////
					//	
					//	send packet
					//
					///
					packets_count++;
					printf("Tx done, Count:%d\n",packets_count);
					USART_RX_STA=0;
				}
			}
			delay_ms(2000);
		}
		
		printf("tx packets:%d\n",packets_count);
	
	}
}


