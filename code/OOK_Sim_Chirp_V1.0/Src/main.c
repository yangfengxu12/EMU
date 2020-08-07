#include "stm32l4xx.h"
#include <string.h>
#include "hw.h"
#include "radio.h"
#include "timeServer.h"
#include "low_power_manager.h"
#include "vcom.h"
#include "sx1276.h"
#include "timer.h"
#include "Freq_Set.h"
#include "sx1276mb1mas.h"

#include "fast_spi.h"



#define FINE_PRECISION


#define RF_FREQUENCY                                470000000 // Hz
#define TX_OUTPUT_POWER                             14        // dBm

#define LoRa_BW																			125000		// Hz
#define LoRa_SF																			11				// spread factor
#define LoRa_Base_Freq															(RF_FREQUENCY - (LoRa_BW >> 1)) // Hz
#define LoRa_Max_Freq																(RF_FREQUENCY + (LoRa_BW >> 1)) // Hz
#define LoRa_Freq_Step															(LoRa_BW / (1 << LoRa_SF))

#define LoRa_Preamble_Length												8
#define LoRa_ID																			2
#define LoRa_SFD_Length															2
#define LoRa_Payload_Length													16

#define LoRa_Symbol_Time														8 * (1 << LoRa_SF)

#define LED_PERIOD_MS               50				


int LoRa_ID_Start_Freq[LoRa_ID] = {-62011,-61523,};

int LoRa_Payload_Start_Freq[LoRa_Payload_Length] = {-7083,
-27345,
-58594,
60295,
-37355,
33441,
-53223,
-35646,
3597,
46746,
8662,
61088,
-60364,
-61706,
-16543,
-52307};

static RadioEvents_t RadioEvents;
																											
extern TIM_HandleTypeDef TIM3_Handler;
extern uint32_t time_count;
/* Private function prototypes -----------------------------------------------*/
/*!
 * Radio events function pointer
 */
//static RadioEvents_t RadioEvents;
void LoRa_upChirp(void);
void LoRa_downChirp(void);
void Generate_Quarter_downChirp();
void LoRa_Payload( int Start_freq);
void Generate_chip( uint32_t freq );
void Fast_SetChannel( uint32_t freq );
/**
 * Main application entry point.
 */
int main(void)
{
//  bool isMaster = true;
  uint8_t i;
	uint32_t fdev = 0 ,reg_value;
	u8 buf[5]={0X12,0X34,0X56,0X78,0X90};
	
  HAL_Init();
	
  SystemClock_Config();

  HW_Init();
	SPI1_Init();
	/*Disbale Stand-by mode*/
  LPM_SetOffMode(LPM_APPLI_Id, LPM_Disable);
	TIM1_Init(0xffff-1,80-1);       //Timer interrupt time 5us;
	
	GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	
  GPIO_InitStruct.Pin   = GPIO_PIN_5 | GPIO_PIN_2 | GPIO_PIN_12;					//PB5 DIO2 DATA
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
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
	
	SX1276_Burst_Write(0x06, buf, 1);
	
	SX1276_Burst_Write(0x06, buf, 2);
	
	SX1276_Burst_Write(0x06, buf, 3);
	
//	while(1);
//	HAL_TIM_Base_Start_IT(&TIM3_Handler);
//	SX1276SetOpMode( RF_OPMODE_TRANSMITTER );
//	DelayMs(100);
  while (1)
  {
		
		SX1276SetOpMode( RF_OPMODE_TRANSMITTER );
		DelayMs(100);
		TIM1->CR1|=0x01;   // start timer
		LoRa_upChirp();

////		for(m=0;m<LoRa_ID;m++)
////		{
////			LoRa_Payload( LoRa_ID_Start_Freq[m]);
////			HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_2);
////		}
////		for(m=0;m<LoRa_SFD_Length;m++)
////		{
////			LoRa_downChirp();
////			HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_2);
////		}
////		Generate_Quarter_downChirp();
////		HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_2);
////		for(m=0;m<LoRa_Payload_Length;m++)
////		{
////			LoRa_Payload( LoRa_Payload_Start_Freq[m]);
////			HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_2);
////		}
		SX1276SetOpMode( RF_OPMODE_STANDBY );
		TIM1->CR1|=0x00;
		DelayMs(2000);
		
  }
}




uint8_t Channel_Freq_MSB_temp = 0;
uint8_t Channel_Freq_MID_temp = 0;
uint8_t Channel_Freq_LSB_temp = 0;

uint16_t Time_temp;
uint32_t Input_Freq;
uint32_t Channel;


uint32_t Input_Freq_temp[1<<LoRa_SF]={0};

uint32_t Time_temp_temp[1<<LoRa_SF]={0};

uint32_t n_temp[1<<LoRa_SF]={0};

//uint32_t Previous_Chip_Time_temp[1<<LoRa_SF]={0};

void LoRa_upChirp()
{
	uint8_t Upchirp_Count;
	uint16_t Previous_Chip_Time = 0;
	uint32_t Count = 0;
	uint8_t n=1; // the number of changed registers.
	
//	Fast_SetChannel(LoRa_Base_Freq);
	for(Upchirp_Count=0;Upchirp_Count<LoRa_Preamble_Length;Upchirp_Count++)
	{
		Previous_Chip_Time = 0;
		Count = 0;
		Input_Freq = LoRa_Base_Freq;
		TIM1->CNT = 0;
		n = 1;
		while(1)
		{
			Time_temp = TIM1->CNT;
			if(Time_temp > LoRa_Symbol_Time || (Input_Freq > LoRa_Base_Freq + LoRa_BW))
			{
				break;
			}
			else if( ( ( GPIOB->IDR & GPIO_PIN_6) != 0x00u) && ( ( GPIOA->IDR & GPIO_PIN_9) != 0x00u ) )
			{
				Input_Freq = LoRa_Base_Freq + 2 * Count * LoRa_Freq_Step;
				
				SX_FREQ_TO_CHANNEL( Channel, Input_Freq );
				if(Channel_Freq_MSB_temp != ( uint8_t )( ( Channel >> 16 ) & 0xFF ))
					n+=1;
				if(Channel_Freq_MID_temp != ( uint8_t )( ( Channel >> 8 ) & 0xFF ))
					n+=1;
				if( n == 1)
				{
					do
					{
						Time_temp = TIM1->CNT;
					}
					while( Time_temp - Previous_Chip_Time <= 16 - 4 );
				}
				else if( n == 2)
				{
					do
					{
						Time_temp = TIM1->CNT;
					}
					while( Time_temp - Previous_Chip_Time <= 16 - 8 );
				}
				else if( n == 3)
				{
					do
					{
						Time_temp = TIM1->CNT;
					}
					while( Time_temp - Previous_Chip_Time <= 16 - 12 );
				}
				Fast_SetChannel( Input_Freq );
//				Previous_Chip_Time_temp[Count] = Previous_Chip_Time;
				Previous_Chip_Time = Time_temp;
				Input_Freq_temp[Count] = Input_Freq;
				Time_temp_temp[Count] = Time_temp;
				
				n_temp[Count] = n;
				Count++;
				n=1;
				HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_12);
			}
		}
		HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_2);
	}
	TIM1->CR1|=0x00;   // stop timer
}

void LoRa_downChirp()
{
	uint32_t time_temp = 0;
	
	time_count = 0;
	
	Fast_SetChannel(LoRa_Max_Freq);
	while(1)
	{
		if(time_count >= LoRa_Symbol_Time)
		{
			break;
		}
		else if((HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_6) == GPIO_PIN_SET) && \
			(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_9)  == GPIO_PIN_SET))
		{
			time_temp = time_count;
			Generate_chip(LoRa_Max_Freq - (ceil(time_temp / 8) + 1) * LoRa_Freq_Step);
			HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_12);
		}
	}
}

void Generate_Quarter_downChirp()
{
	uint32_t time_temp = 0;
	
	time_count = 0;

	Fast_SetChannel(LoRa_Max_Freq);
	while(1)
	{
		if(time_count >= LoRa_Symbol_Time >> 2)
		{
			break;
		}
		else if((HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_6) == GPIO_PIN_SET) && \
			(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_9)  == GPIO_PIN_SET))
		{
			time_temp = time_count;
			Generate_chip(LoRa_Max_Freq - (ceil(time_temp / 8) + 1) * LoRa_Freq_Step);
			HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_12);
		}
	}
	
}

void LoRa_Payload( int Start_freq)
{
	uint32_t time_temp = 0;
	int freq = 0;
	
	time_count = 0;
	Fast_SetChannel(RF_FREQUENCY + Start_freq);
	while(1)
	{
		if(time_count >= LoRa_Symbol_Time)
		{
			break;
		}
		else if((HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_6) == GPIO_PIN_SET) && \
			(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_9)  == GPIO_PIN_SET))
		{
			time_temp = time_count;
			freq = Start_freq + (ceil(time_temp / 8)+1) * LoRa_Freq_Step;
			if( freq <= (LoRa_BW>>1))
				Fast_SetChannel( RF_FREQUENCY + freq );
			else
				Fast_SetChannel( RF_FREQUENCY + freq - LoRa_BW);
			HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_12);
		}
	}
}

void Generate_chip( uint32_t freq )
{
	Fast_SetChannel( freq );
}


void Fast_SetChannel( uint32_t freq )
{
//    uint32_t channel;
//			
////		channel = freq / FREQ_STEP;
//		SX_FREQ_TO_CHANNEL( channel, freq );
//		if(Channel_Freq_MSB_temp != ( uint8_t )( ( channel >> 16 ) & 0xFF ))
//			SPI1_WriteByte( REG_FRFMSB, ( uint8_t )( ( channel >> 16 ) & 0xFF ) );
//		
//		if(Channel_Freq_MID_temp != ( uint8_t )( ( channel >> 8 ) & 0xFF ))
//			SPI1_WriteByte( REG_FRFMID, ( uint8_t )( ( channel >> 8 ) & 0xFF ) );
//		
//    SPI1_WriteByte( REG_FRFLSB, ( uint8_t )( channel & 0xFF ) );
//		
//		Channel_Freq_MSB_temp = ( uint8_t )( ( channel >> 16 ) & 0xFF );
//		Channel_Freq_MID_temp = ( uint8_t )( ( channel >> 8 ) & 0xFF );
//		Channel_Freq_LSB_temp = ( uint8_t )( channel & 0xFF );
}

