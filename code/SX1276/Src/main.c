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

#define BUFFER_SIZE                                 255 // Define the payload size here
#define PACKET_COUNT																500	//
#define INTERVAL_TIME																200  // ms


#define MODE  																			2 // #1 LOOK, LOOK run as normal lora
																											// #2 LOOK_BLANK, insert blank in symbols
																											// #3 LOOK_DOUBLE, transmit two packets in diffenert channels at the same time

#if (MODE==1) 																			
	#define LOOK
#elif (MODE==2)
	#define LOOK_BLANK
	#define LOOK_BLANK_RATIO														0.8  // This para means x% turn on PA and (1-x)% turn off PA.
#elif (MODE==3)
	#define LOOK_DOUBLE
#endif

#define ENABLE_USART

static RadioEvents_t RadioEvents;
																											
extern TIM_HandleTypeDef TIM2_Handler;
extern uint32_t time_count;
extern uint32_t Time_temp;

uint8_t data_1[21] = {0x80, 0x37, 0x00, 0x1e, 0x00, 0x00, 0x01, 0x00, 0x02, 0x79, 0x91, 0x05, 0x55, 0x29, 0x3b, 0x6b, 0xab, 0xd7, 0x47, 0xc0, 0x49};
uint8_t data_2[21] = {0x80, 0x2c, 0x00, 0x42, 0x00, 0x00, 0x01, 0x00, 0x02, 0xbe, 0x52, 0xf5, 0x2c, 0xe3, 0xf6, 0xc0, 0xe4, 0x11, 0xd1, 0xb1, 0x6b};

uint8_t Tx_Buffer[BUFFER_SIZE]={0x01,0x02};

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
#ifdef ENABLE_USART
//	printf("Tx\r\n");
//	printf("CR=4/%d, CRC=%s, IMPL_HEAD=%s, LDR=%s\n",4+LORA_CR_NO1,LORA_HAS_CRC_NO1?"ON":"OFF",LORA_IMPL_HEAD_NO1?"ON":"OFF",LORA_LOWDATERATEOPTIMIZE_NO1?"ON":"OFF");
//	printf("FREQ1:%d,sf1:%d,\r\nFREQ2:%d,sf2:%d\r\n",RF_FREQUENCY_NO1,LORA_SF_NO1,RF_FREQUENCY_NO2,LORA_SF_NO2);
#endif
//	for (int j = 0; j < BufferSize; j++)
//	{
////		Tx_Buffer[j] = 0x31;
//		Tx_Buffer[j] =rand()%255;
//	}
	
	printf("id,data 1,data 2,data 3,data 4,data 5,data 6,data 7,data 8,data 9,data 10,data \
	11,data 12,data 13,data 14,data 15,data 16,data 17,data 18,data 19,data 20,data 21,data\
	22,data 23,data 24,data 25,data 26,data 27,data 28,data 29,data 30,data 31,data 32,data \
	33,data 34,data 35,data 36,data 37,data 38,data 39,data 40,data 41,data 42,data 43,data \
	44,data 45,data 46,data 47,data 48,data 49,data 50,data 51,data 52,data 53,data 54,data \
	55,data 56,data 57,data 58,data 59,data 60,data 61,data 62,data 63,data 64,data 65,data \
	66,data 67,data 68,data 69,data 70,data 71,data 72,data 73,data 74,data 75,data 76,data \
	77,data 78,data 79,data 80,data 81,data 82,data 83,data 84,data 85,data 86,data 87,data \
	88,data 89,data 90,data 91,data 92,data 93,data 94,data 95,data 96,data 97,data 98,data \
	99,data 100,data 101,data 102,data 103,data 104,data 105,data 106,data 107,data 108,data \
	109,data 110,data 111,data 112,data 113,data 114,data 115,data 116,data 117,data 118,data \
	119,data 120,data 121,data 122,data 123,data 124,data 125,data 126,data 127,data 128,data \
	129,data 130,data 131,data 132,data 133,data 134,data 135,data 136,data 137,data 138,data \
	139,data 140,data 141,data 142,data 143,data 144,data 145,data 146,data 147,data 148,data \
	149,data 150,data 151,data 152,data 153,data 154,data 155,data 156,data 157,data 158,data \
	159,data 160,data 161,data 162,data 163,data 164,data 165,data 166,data 167,data 168,data \
	169,data 170,data 171,data 172,data 173,data 174,data 175,data 176,data 177,data 178,data \
	179,data 180,data 181,data 182,data 183,data 184,data 185,data 186,data 187,data 188,data \
	189,data 190,data 191,data 192,data 193,data 194,data 195,data 196,data 197,data 198,data \
	199,data 200,data 201,data 202,data 203,data 204,data 205,data 206,data 207,data 208,data \
	209,data 210,data 211,data 212,data 213,data 214,data 215,data 216,data 217,data 218,data \
	219,data 220,data 221,data 222,data 223,data 224,data 225,data 226,data 227,data 228,data \
	229,data 230,data 231,data 232,data 233,data 234,data 235,data 236,data 237,data 238,data \
	239,data 240,data 241,data 242,data 243,data 244,data 245,data 246,data 247,data 248,data \
	249,data 250,data 251,data 252,data 253,data 254,data 255\n");
	
	for(i=0;i<PACKET_COUNT;i++)
	{
		printf("%d",i);
		for (int j = 0; j < BufferSize; j++)
		{
//			Tx_Buffer[j] = 0x31;
			Tx_Buffer[j] =rand()%255;
			printf(",");
			printf("%x",Tx_Buffer[j]);
		}
		printf("\n");
		packet_freq_points_No1 = LoRa_Channel_Coding(Tx_Buffer, BufferSize, LORA_BW, LORA_SF_NO1, LORA_CR_NO1, LORA_HAS_CRC_NO1, LORA_IMPL_HEAD_NO1, &symbol_len_No1, LORA_LOWDATERATEOPTIMIZE_NO1);

		#ifdef LOOK_DOUBLE
		packet_freq_points_No2 = LoRa_Channel_Coding(Tx_Buffer, BufferSize, LORA_BW, LORA_SF_NO2, LORA_CR_NO2, LORA_HAS_CRC_NO2, LORA_IMPL_HEAD_NO2, &symbol_len_No2, LORA_LOWDATERATEOPTIMIZE_NO2);
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
//		printf("Tx done, Count:%d\r\n",i+1);
		#endif
		delay_ms(INTERVAL_TIME);
	}
#ifdef ENABLE_USART
//	printf("finish!!\r\n");
#endif

}


