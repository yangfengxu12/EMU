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

#define FINE_PRECISION


#define RF_FREQUENCY                                470000000 // Hz
#define TX_OUTPUT_POWER                             14        // dBm

#define LoRa_BW																			125000		// Hz
#define LoRa_SF																			11				// spread factor
#define LoRa_Base_Freq															(RF_FREQUENCY - (LoRa_BW >> 1)) // Hz
#define LoRa_Max_Freq																(RF_FREQUENCY + (LoRa_BW >> 1)) // Hz
#define LoRa_Freq_Step															(LoRa_BW >> LoRa_SF)

#define LoRa_Preamble_Length												8
#define LoRa_ID																			2
#define LoRa_SFD_Length															2
#define LoRa_Payload_Length													16

#define LoRa_Symbol_Time														8 * (1 << LoRa_SF)

#define LED_PERIOD_MS               50				


#define interval_time_of_hop												90 //us
#define TS_HOP																			20 //us
#define Fixed_time																	5 //us
#define hop_value 																	(interval_time_of_hop >> 3) * LoRa_Freq_Step

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
	uint32_t fdev = 0;
	uint8_t m;
	
  HAL_Init();

  SystemClock_Config();

  HW_Init();
	/*Disbale Stand-by mode*/
  LPM_SetOffMode(LPM_APPLI_Id, LPM_Disable);
	TIM3_Init(5-1,80-1);       //Timer interrupt time 5us;
	
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
	
//	HAL_TIM_Base_Start_IT(&TIM3_Handler);
//	SX1276SetOpMode( RF_OPMODE_TRANSMITTER );
//	DelayMs(100);
  while (1)
  {

		SX1276SetOpMode( RF_OPMODE_TRANSMITTER );
		DelayMs(100);
		for(m=0;m<LoRa_Preamble_Length;m++)
		{
			LoRa_upChirp();
			HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_2);
		}
//		for(m=0;m<LoRa_ID;m++)
//		{
//			LoRa_Payload( LoRa_ID_Start_Freq[m] + RF_FREQUENCY);
////			HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_2);
//		}
		for(m=0;m<LoRa_SFD_Length;m++)
		{
			LoRa_downChirp();
			HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_2);
		}
		Generate_Quarter_downChirp();
		HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_2);
//		for(m=0;m<LoRa_Payload_Length;m++)
//		{
//			LoRa_Payload( LoRa_Payload_Start_Freq[m] + RF_FREQUENCY);
////			HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_2);
//		}
		SX1276SetOpMode( RF_OPMODE_STANDBY );
		DelayMs(1000);
		
  }
}


uint32_t increased_temp[2<<11]={0};

//void LoRa_upChirp()
//{
//	uint16_t Count = 0;
//	uint32_t time_temp = 0;
//	uint32_t increaed = 0;
//	
//	time_count = 0;
//	HAL_TIM_Base_Start_IT(&TIM3_Handler);
//	Generate_chip(LoRa_Base_Freq);
//	while(1)
//	{
//		if((HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_6) == GPIO_PIN_SET) && \
//			(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_9)  == GPIO_PIN_SET))
//		{
//			HAL_TIM_Base_Stop_IT(&TIM3_Handler);
//			time_temp = time_count;
//			time_count = 0;
//			HAL_TIM_Base_Start_IT(&TIM3_Handler);
//#ifdef  FINE_PRECISION
//			increaed += ((uint32_t)(ceil(time_temp  + Fixed_time)) >> 3) * LoRa_Freq_Step;
//#else
//			increaed += ceil(time_temp >> 3)  * LoRa_Freq_Step;
//#endif
//			if(increaed + hop_value >= LoRa_BW)
//			{
//				HAL_TIM_Base_Stop_IT(&TIM3_Handler);
//				time_count = 0;
//				break;
//			}
//			Generate_chip(LoRa_Base_Freq + increaed);
//			increased_temp[Count] = increaed;
//			Count++;
//		}
//	}
//}

void LoRa_upChirp()
{
	uint16_t Count = 0;
	uint32_t time_temp = 0;
	uint32_t increaed = 0;
	
	time_count = 0;
	HAL_TIM_Base_Start_IT(&TIM3_Handler);
//	Generate_chip(LoRa_Base_Freq);
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
			Generate_chip( LoRa_Base_Freq + (time_temp >> 3) * LoRa_Freq_Step);
			HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_12);
		}
	}
}

//void LoRa_downChirp()
//{
//	uint16_t Count = 0;
//	uint32_t time_temp = 0;
//	uint32_t increaed = 0;
//	
//	time_count = 0;
//	HAL_TIM_Base_Start_IT(&TIM3_Handler);
//	Generate_chip(LoRa_Max_Freq);
//	while(1)
//	{
//		if((HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_6) == GPIO_PIN_SET) && \
//			(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_9)  == GPIO_PIN_SET))
//		{
//			HAL_TIM_Base_Stop_IT(&TIM3_Handler);
//			time_temp = time_count;
//			time_count = 0;
//			HAL_TIM_Base_Start_IT(&TIM3_Handler);
//#ifdef  FINE_PRECISION
//			increaed += ((uint32_t)(ceil(time_temp  + Fixed_time)) >> 3) * LoRa_Freq_Step;
//#else
//			increaed += ceil(time_temp >> 3)  * LoRa_Freq_Step;
//#endif
//			if(increaed + hop_value >= LoRa_BW)
//			{
//				HAL_TIM_Base_Stop_IT(&TIM3_Handler);
//				time_count = 0;
//				break;
//			}
//			Generate_chip(LoRa_Max_Freq - increaed);
//		}
//	}
//}



void LoRa_downChirp()
{
	uint16_t Count = 0;
	uint32_t time_temp = 0;
	uint32_t increaed = 0;
	
	time_count = 0;
	HAL_TIM_Base_Start_IT(&TIM3_Handler);
//	Generate_chip(LoRa_Max_Freq);
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
			Generate_chip(LoRa_Max_Freq - (time_temp >> 3) * LoRa_Freq_Step);

		}
	}
}





//void Generate_Quarter_downChirp()
//{
//	uint16_t Count = 0;
//	uint32_t time_temp = 0;
//	uint32_t increaed = 0;
//	
//	time_count = 0;
//	HAL_TIM_Base_Start_IT(&TIM3_Handler);
//	Generate_chip(LoRa_Max_Freq);
//	while(1)
//	{
//		if((HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_6) == GPIO_PIN_SET) && \
//			(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_9)  == GPIO_PIN_SET))
//		{
//			HAL_TIM_Base_Stop_IT(&TIM3_Handler);
//			time_temp = time_count;
//			time_count = 0;
//			HAL_TIM_Base_Start_IT(&TIM3_Handler);
//#ifdef  FINE_PRECISION
//			increaed += ((uint32_t)(ceil(time_temp  + Fixed_time)) >> 3) * LoRa_Freq_Step;
//#else
//			increaed += ceil(time_temp >> 3)  * LoRa_Freq_Step;
//#endif
//			if(increaed + hop_value >= LoRa_BW >> 2)
//			{
//				HAL_TIM_Base_Stop_IT(&TIM3_Handler);
//				time_count = 0;
//				break;
//			}
//			Generate_chip(LoRa_Max_Freq - increaed);

//		}
//	}
//	
//}

void Generate_Quarter_downChirp()
{
	uint16_t Count = 0;
	uint32_t time_temp = 0;
	uint32_t increaed = 0;
	
	time_count = 0;
	HAL_TIM_Base_Start_IT(&TIM3_Handler);
//	Generate_chip(LoRa_Max_Freq);
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
			Generate_chip(LoRa_Base_Freq + LoRa_BW - (time_temp >> 3) * LoRa_Freq_Step);
		}
	}
	
}






void LoRa_Payload( int Start_freq)
{
	uint16_t Count = 0;
	uint32_t time_temp = 0;
	uint32_t increaed = 0;
	Start_freq = (uint32_t)Start_freq;
	
	time_count = 0;
	HAL_TIM_Base_Start_IT(&TIM3_Handler);
	Generate_chip(Start_freq);
	while(1)
	{
		if((HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_6) == GPIO_PIN_SET) && \
			(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_9)  == GPIO_PIN_SET))
		{
			HAL_TIM_Base_Stop_IT(&TIM3_Handler);
			time_temp = time_count;
			time_count = 0;
			HAL_TIM_Base_Start_IT(&TIM3_Handler);
#ifdef  FINE_PRECISION
			increaed += ((uint32_t)(ceil(time_temp  + Fixed_time)) >> 3) * LoRa_Freq_Step;
#else
			increaed += ceil(time_temp >> 3)  * LoRa_Freq_Step;
#endif

			if(increaed + hop_value >= LoRa_BW)
			{
				HAL_TIM_Base_Stop_IT(&TIM3_Handler);
				time_count = 0;
				break;
			}
			if(Start_freq + increaed >= LoRa_Base_Freq + LoRa_BW)
				Generate_chip(Start_freq + increaed - LoRa_BW);
			else
				Generate_chip(Start_freq + increaed);
		}
	}
}

void Generate_chip( uint32_t freq )
{
//	SX1276SetChannel( freq );
	Fast_SetChannel( freq );
}

uint8_t Channel_Freq_MSB_temp = 0;
uint8_t Channel_Freq_MID_temp = 0;
uint8_t Channel_Freq_LSB_temp = 0;

void Fast_SetChannel( uint32_t freq )
{
    uint32_t channel;
			
//		channel = freq / FREQ_STEP;
		SX_FREQ_TO_CHANNEL( channel, freq );
		if(Channel_Freq_MSB_temp != ( uint8_t )( ( channel >> 16 ) & 0xFF ))
			SX1276Write( REG_FRFMSB, ( uint8_t )( ( channel >> 16 ) & 0xFF ) );
		
		if(Channel_Freq_MID_temp != ( uint8_t )( ( channel >> 8 ) & 0xFF ))
			SX1276Write( REG_FRFMID, ( uint8_t )( ( channel >> 8 ) & 0xFF ) );
		
    SX1276Write( REG_FRFLSB, ( uint8_t )( channel & 0xFF ) );
		
		Channel_Freq_MSB_temp = ( uint8_t )( ( channel >> 16 ) & 0xFF );
		Channel_Freq_MID_temp = ( uint8_t )( ( channel >> 8 ) & 0xFF );
		Channel_Freq_LSB_temp = ( uint8_t )( channel & 0xFF );
}

void GPIO_Pin_Set(GPIO_TypeDef* GPIOx,u16 pinx,u8 status)
{
	if(status&0X01)GPIOx->BSRR=pinx;	//设置GPIOx的pinx为1
	else GPIOx->BSRR=pinx<<16;			//设置GPIOx的pinx为0
}
