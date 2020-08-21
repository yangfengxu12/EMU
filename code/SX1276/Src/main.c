#include "stm32l4xx.h"
#include <string.h>
#include "hw.h"
#include "radio.h"
#include "timeServer.h"
#include "low_power_manager.h"
#include "vcom.h"
#include "sx1276.h"
#include "sx1276mb1mas.h"

#include "timer.h"
#include "fast_spi.h"
#include "rtc.h"
#include "delay.h"
#include "usart.h"

#define RF_FREQUENCY                                470000000 // Hz
#define TX_OUTPUT_POWER                             14        // dBm

#define LORA_BW																			125000		// Hz
#define LORA_SF																			11				// spread factor
#define LORA_BASE_FREQ															(RF_FREQUENCY - (LORA_BW >> 1)) // Hz
#define LORA_MAX_FREQ																(RF_FREQUENCY + (LORA_BW >> 1)) // Hz


#define LORA_PREAMBLE_LENGTH												8
#define LORA_ID_LENGTH															2
#define LORA_SFD_LENGTH															2
#define LORA_PAYLOAD_LENGTH													16

#define LORA_SYMBOL_TIME														8 * (1 << LORA_SF)

#define LED_PERIOD_MS               50				


int LoRa_ID_Start_Freq[LORA_ID_LENGTH] = {-62011,-61523};

int LoRa_Payload_Start_Freq[LORA_PAYLOAD_LENGTH] = {-7083,
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

#define LORA_FREQ_STEP	( LORA_BW / ( 1 << LORA_SF ))

uint8_t Channel_Freq[3] = {0};  //MSB,MID,LSB
uint8_t Changed_Register_Count = 1;  // the number of changed registers.

//1.00014136 = 1      +2us

uint32_t RTC_Subsecond_Value[3] = {0};

static RadioEvents_t RadioEvents;
																											
extern TIM_HandleTypeDef TIM3_Handler;
extern uint32_t time_count;
/* Private function prototypes -----------------------------------------------*/
/*!
 * Radio events function pointer
 */
//static RadioEvents_t RadioEvents;
void RTC_Timer_Calibration(void);
void LoRa_UpChirp(void);
void LoRa_DownChirp(void);
void Generate_Quarter_DownChirp(void);
void LoRa_Payload( int Start_freq);
void Generate_chip( uint32_t freq );
void Fast_SetChannel( uint8_t *freq );
/**
 * Main application entry point.
 */
int main(void)
{
//  bool isMaster = true;
	uint8_t m;
  HAL_Init();
	
  SystemClock_Config();

  HW_Init();
	
	SPI1_Init();
	delay_init(80);
	uart_init(115200);
	printf("123");
	RTC_Init();
	
	/*Disbale Stand-by mode*/
  LPM_SetOffMode(LPM_APPLI_Id, LPM_Disable);
	TIM1_Init(0xffff-1,80-1);       //Timer resolution = 1us;
	
	GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	
  GPIO_InitStruct.Pin   = GPIO_PIN_2 | GPIO_PIN_5 | GPIO_PIN_12;					//PB5
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pin   = GPIO_PIN_1;
	GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	
	HW_GPIO_Write(GPIOB,GPIO_PIN_5,GPIO_PIN_SET); //PB5 DIO2 DATA HIGH
	HW_GPIO_Write(GPIOB,GPIO_PIN_2,GPIO_PIN_RESET);
	HW_GPIO_Write(GPIOB,GPIO_PIN_12,GPIO_PIN_RESET);
	
	GPIO_InitStruct.Pin   = GPIO_PIN_9;				//PA9 DIO4_a PLL LOCK
  GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	RTC_Timer_Calibration();
	
	while(1)
	{
//		HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_1);
//		HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_1);
		RTC_Subsecond_Value[0] = (uint32_t) RTC_Handler.Instance->SSR;
		delay_ms(100);
//		HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_1);
		RTC_Subsecond_Value[1] = (uint32_t) RTC_Handler.Instance->SSR;
		delay_ms(100);
	}
	
/*
	Radio.Init(&RadioEvents);

  Radio.SetChannel(RF_FREQUENCY);

	Radio.SetTxContinuousWave(RF_FREQUENCY,TX_OUTPUT_POWER,3);

	SX1276Write( REG_PLLHOP, ( SX1276Read( REG_PLLHOP ) & RF_PLLHOP_FASTHOP_MASK ) | RF_PLLHOP_FASTHOP_ON );
	
	SX1276Write( REG_PARAMP, ( SX1276Read( REG_PARAMP ) & RF_PARAMP_MASK ) | RF_PARAMP_0010_US );
	
	SX1276Write( REG_OCP, ( SX1276Read( REG_OCP ) & RF_OCP_MASK ) | RF_OCP_OFF );
	
//	while(1);
//	HAL_TIM_Base_Start_IT(&TIM3_Handler);
//	SX1276SetOpMode( RF_OPMODE_TRANSMITTER );
//	DelayMs(100);
  while (1)
  {
		
		SX1276SetOpMode( RF_OPMODE_TRANSMITTER );
		DelayMs(100);
		TIM1->CR1|=0x01;   // start timer
		
		for(m=0;m<LORA_PREAMBLE_LENGTH;m++)
		{
			LoRa_UpChirp();
			HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_2);
		}
		
//		for(m=0;m<LORA_ID_LENGTH;m++)
//		{
//			LoRa_Payload( LoRa_ID_Start_Freq[m]);
//			HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_2);
//		}
//		for(m=0;m<LORA_SFD_LENGTH;m++)
//		{
//			LoRa_DownChirp();
//			HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_2);
//		}
//		
//		Generate_Quarter_DownChirp();
//		HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_2);
//		for(m=0;m<LORA_PAYLOAD_LENGTH;m++)
//		{
//			LoRa_Payload( LoRa_Payload_Start_Freq[m]);
//			HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_2);
//		}
		SX1276SetOpMode( RF_OPMODE_STANDBY );
		TIM1->CR1|=0x00;
		DelayMs(2000);
		
  }
	*/
}

uint8_t Channel_Freq_MSB_temp = 0;
uint8_t Channel_Freq_MID_temp = 0;
uint8_t Channel_Freq_LSB_temp = 0;

uint16_t Time_temp;
uint32_t Input_Freq;
uint32_t Channel;


uint32_t Input_Freq_temp[1<<LORA_SF]={0};
uint32_t Time_temp_temp[1<<LORA_SF]={0};
uint32_t n_temp[1<<LORA_SF]={0};



void RTC_Timer_Calibration()
{
	float RTC_Time_temp = 0;
	TIM1->CR1|=0x00;
	TIM1->CNT = 0;
	RTC_Subsecond_Value[0] = (uint32_t) RTC_Handler.Instance->SSR;
	while(RTC_Handler.Instance->SSR - RTC_Subsecond_Value[0] == 0);
	TIM1->CR1|=0x01;
	RTC_Subsecond_Value[2] = (uint32_t) RTC_Handler.Instance->SSR;
	while( TIM1->CNT <= LORA_SYMBOL_TIME);  
	Time_temp = TIM1->CNT;
	RTC_Subsecond_Value[1] = (uint32_t) RTC_Handler.Instance->SSR;
	TIM1->CR1|=0x00;
	TIM1->CNT = 0;
	if( RTC_Subsecond_Value[0] < RTC_Subsecond_Value[1] )
	{
		RTC_Subsecond_Value[0] += 32768;
	}
	RTC_Time_temp = ( RTC_Subsecond_Value[0] - RTC_Subsecond_Value[1] ) / 32768;
}

void LoRa_UpChirp()
{
	uint32_t Count = 0;

	Input_Freq = LORA_BASE_FREQ;
	TIM1->CNT = 0;
	while(1)
	{
		Time_temp = TIM1->CNT;
		if( Time_temp > LORA_SYMBOL_TIME || ( Input_Freq > LORA_BASE_FREQ + LORA_BW ))
		{
			break;
		}
		else if((( GPIOB->IDR & GPIO_PIN_6) != 0x00u) && (( GPIOA->IDR & GPIO_PIN_9) != 0x00u ))
		{
			Input_Freq = LORA_BASE_FREQ + Count * LORA_FREQ_STEP;
			
			SX_FREQ_TO_CHANNEL( Channel, Input_Freq );

			Channel_Freq[0] = ( uint8_t )( ( Channel >> 16 ) & 0xFF );
			Channel_Freq[1] = ( uint8_t )( ( Channel >> 8 ) & 0xFF );
			Channel_Freq[2] = ( uint8_t )( ( Channel ) & 0xFF );
			
			if(Channel_Freq_MSB_temp != Channel_Freq[0])
			{
				Changed_Register_Count = 3;
				Channel_Freq_MSB_temp = Channel_Freq[0];
				Channel_Freq_MID_temp = Channel_Freq[1];
				Channel_Freq_LSB_temp = Channel_Freq[2];
			}
			else if(Channel_Freq_MID_temp != Channel_Freq[1])
			{
				Changed_Register_Count = 2;
				Channel_Freq_MID_temp = Channel_Freq[1];
				Channel_Freq_LSB_temp = Channel_Freq[2];
			}
			else if(Channel_Freq_MID_temp != Channel_Freq[2])
			{
				Changed_Register_Count = 1;
				Channel_Freq_LSB_temp = Channel_Freq[2];
			}
			
			do
			{
				Time_temp = TIM1->CNT;
			}while( Time_temp & (8-1));  // 10Mhz spi

			Fast_SetChannel( Channel_Freq );
			Time_temp_temp[Count] = Time_temp;
			Input_Freq_temp[Count] = Input_Freq;
			
			n_temp[Count] = Changed_Register_Count;
			Changed_Register_Count = 1;
			Count++;
			
			HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_12);
		}
	}
}

void LoRa_DownChirp()
{
	uint32_t Count = 0;

	TIM1->CNT = 0;
	while(1)
	{
		Time_temp = TIM1->CNT;
		if(Time_temp > LORA_SYMBOL_TIME || ( Input_Freq < LORA_BASE_FREQ ))
		{
			break;
		}
		else if((( GPIOB->IDR & GPIO_PIN_6) != 0x00u) && ( ( GPIOA->IDR & GPIO_PIN_9 ) != 0x00u ))
		{
			Input_Freq = LORA_MAX_FREQ - Count * LORA_FREQ_STEP;
			
			SX_FREQ_TO_CHANNEL( Channel, Input_Freq );

			Channel_Freq[0] = ( uint8_t )( ( Channel >> 16 ) & 0xFF );
			Channel_Freq[1] = ( uint8_t )( ( Channel >> 8 ) & 0xFF );
			Channel_Freq[2] = ( uint8_t )( ( Channel ) & 0xFF );
			
			if(Channel_Freq_MSB_temp != Channel_Freq[0])
			{
				Changed_Register_Count = 3;
				Channel_Freq_MSB_temp = Channel_Freq[0];
				Channel_Freq_MID_temp = Channel_Freq[1];
				Channel_Freq_LSB_temp = Channel_Freq[2];
			}
			else if(Channel_Freq_MID_temp != Channel_Freq[1])
			{
				Changed_Register_Count = 2;
				Channel_Freq_MID_temp = Channel_Freq[1];
				Channel_Freq_LSB_temp = Channel_Freq[2];
			}
			else if(Channel_Freq_MID_temp != Channel_Freq[2])
			{
				Changed_Register_Count = 1;
				Channel_Freq_LSB_temp = Channel_Freq[2];
			}
			
			do
			{
				Time_temp = TIM1->CNT;
			}while( Time_temp & (8-1));  // 10Mhz spi

			Fast_SetChannel( Channel_Freq );
			Time_temp_temp[Count] = Time_temp;
			Input_Freq_temp[Count] = Input_Freq;
			
			n_temp[Count] = Changed_Register_Count;
			Changed_Register_Count = 1;
			Count++;
			
			HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_12);
		}
	}
}

void Generate_Quarter_DownChirp()
{
	uint32_t Count = 0;
	
	Input_Freq = LORA_MAX_FREQ;
	TIM1->CNT = 0;
	while(1)
	{
		Time_temp = TIM1->CNT;
		if(Time_temp > LORA_SYMBOL_TIME / 4 )
		{
			break;
		}
		else if((( GPIOB->IDR & GPIO_PIN_6) != 0x00u) && ( ( GPIOA->IDR & GPIO_PIN_9 ) != 0x00u ))
		{
			Input_Freq = LORA_MAX_FREQ - Count * LORA_FREQ_STEP;
			
			SX_FREQ_TO_CHANNEL( Channel, Input_Freq );

			Channel_Freq[0] = ( uint8_t )( ( Channel >> 16 ) & 0xFF );
			Channel_Freq[1] = ( uint8_t )( ( Channel >> 8 ) & 0xFF );
			Channel_Freq[2] = ( uint8_t )( ( Channel ) & 0xFF );
			
			if(Channel_Freq_MSB_temp != Channel_Freq[0])
			{
				Changed_Register_Count = 3;
				Channel_Freq_MSB_temp = Channel_Freq[0];
				Channel_Freq_MID_temp = Channel_Freq[1];
				Channel_Freq_LSB_temp = Channel_Freq[2];
			}
			else if(Channel_Freq_MID_temp != Channel_Freq[1])
			{
				Changed_Register_Count = 2;
				Channel_Freq_MID_temp = Channel_Freq[1];
				Channel_Freq_LSB_temp = Channel_Freq[2];
			}
			else if(Channel_Freq_MID_temp != Channel_Freq[2])
			{
				Changed_Register_Count = 1;
				Channel_Freq_LSB_temp = Channel_Freq[2];
			}
			
			do
			{
				Time_temp = TIM1->CNT;
			}while( Time_temp & (8-1));  // 10Mhz spi

			Fast_SetChannel( Channel_Freq );
			Time_temp_temp[Count] = Time_temp;
			Input_Freq_temp[Count] = Input_Freq;
			
			n_temp[Count] = Changed_Register_Count;
			Changed_Register_Count = 1;
			Count++;
			
			HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_12);
		}
	}
}

void LoRa_Payload( int Start_freq)
{
	uint32_t Count = 0;
	
	TIM1->CNT = 0;
	while(1)
	{
		Time_temp = TIM1->CNT;
		if( Time_temp > LORA_SYMBOL_TIME )
		{
			break;
		}
		else if((( GPIOB->IDR & GPIO_PIN_6 ) != 0x00u ) && (( GPIOA->IDR & GPIO_PIN_9 ) != 0x00u ) )
		{
			Input_Freq = LORA_BASE_FREQ + Start_freq + Count * LORA_FREQ_STEP;
			
			if( Input_Freq > LORA_MAX_FREQ )
			{
				Input_Freq = Input_Freq - LORA_BW;
			}
			
			SX_FREQ_TO_CHANNEL( Channel, Input_Freq );
				
			Channel_Freq[0] = ( uint8_t )( ( Channel >> 16 ) & 0xFF );
			Channel_Freq[1] = ( uint8_t )( ( Channel >> 8 ) & 0xFF );
			Channel_Freq[2] = ( uint8_t )( ( Channel ) & 0xFF );
			
			if( Channel_Freq_MSB_temp != Channel_Freq[0] )
			{
				Changed_Register_Count = 3;
				Channel_Freq_MSB_temp = Channel_Freq[0];
				Channel_Freq_MID_temp = Channel_Freq[1];
				Channel_Freq_LSB_temp = Channel_Freq[2];
			}
			else if( Channel_Freq_MID_temp != Channel_Freq[1] )
			{
				Changed_Register_Count = 2;
				Channel_Freq_MID_temp = Channel_Freq[1];
				Channel_Freq_LSB_temp = Channel_Freq[2];
			}
			else if( Channel_Freq_MID_temp != Channel_Freq[2] )
			{
				Changed_Register_Count = 1;
				Channel_Freq_LSB_temp = Channel_Freq[2];
			}
			
			do
			{
				Time_temp = TIM1->CNT;
			}while( Time_temp & (8-1));  // 10Mhz spi

			Fast_SetChannel( Channel_Freq );
			Time_temp_temp[Count] = Time_temp;
			Input_Freq_temp[Count] = Channel;
			
			n_temp[Count] = Changed_Register_Count;
			Changed_Register_Count = 1;
			Count++;
			
			HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_12);
		}
	}
}

void Generate_chip( uint32_t freq )
{
//	Fast_SetChannel( freq );
}


void Fast_SetChannel( uint8_t *freq )
{
	uint8_t Reg_Address;
	
	switch(Changed_Register_Count)
	{
		case 1:Reg_Address = REG_FRFLSB;break;
		case 2:Reg_Address = REG_FRFMID;break;
		case 3:Reg_Address = REG_FRFMSB;break;
		default:Reg_Address = REG_FRFMSB;break;
	}
	
	SX1276_Burst_Write( Reg_Address, freq + 3 - Changed_Register_Count, Changed_Register_Count);
	
}
