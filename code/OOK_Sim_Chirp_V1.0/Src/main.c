/*!
 * \file      main.c
 *
 * \brief     Ping-Pong implementation
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \code
 *                ______                              _
 *               / _____)             _              | |
 *              ( (____  _____ ____ _| |_ _____  ____| |__
 *               \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 *               _____) ) ____| | | || |_| ____( (___| | | |
 *              (______/|_____)_|_|_| \__)_____)\____)_| |_|
 *              (C)2013-2017 Semtech
 *
 * \endcode
 *
 * \author    Miguel Luis ( Semtech )
 *
 * \author    Gregory Cristian ( Semtech )
 */
/**
  ******************************************************************************
  * @file    main.c
  * @author  MCD Application Team
  * @brief   this is the main!
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2018 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
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


#define RF_FREQUENCY                                470000000 // Hz
#define TX_OUTPUT_POWER                             14        // dBm

#define LoRa_BW																			125000		// Hz
#define LoRa_SF																			12				// spread factor
#define LoRa_Base_Freq															(RF_FREQUENCY - (LoRa_BW >> 1)) // Hz
#define LoRa_Max_Freq																(RF_FREQUENCY + (LoRa_BW >> 1)) // Hz
#define LoRa_Freq_Step															(LoRa_BW >> LoRa_SF)

typedef enum
{
  LOWPOWER,
  RX,
  RX_TIMEOUT,
  RX_ERROR,
  TX,
  TX_TIMEOUT,
} States_t;

#define RX_TIMEOUT_VALUE                            1000
#define BUFFER_SIZE                                 64 // Define the payload size here
#define LED_PERIOD_MS               50

#define LEDS_OFF   do{ \
                   LED_Off( LED_BLUE ) ;   \
                   LED_Off( LED_RED ) ;    \
                   LED_Off( LED_GREEN1 ) ; \
                   LED_Off( LED_GREEN2 ) ; \
                   } while(0) ;						 
									 
									
const uint8_t PingMsg[] = "PING";
const uint8_t PongMsg[] = "PONG";

uint16_t BufferSize = BUFFER_SIZE;
uint8_t Buffer[BUFFER_SIZE];

States_t State = LOWPOWER;

int8_t RssiValue = 0;
int8_t SnrValue = 0;

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
void Generate_chip( uint32_t freq );
void SetChannel( uint32_t freq );
/**
 * Main application entry point.
 */
int main(void)
{
//  bool isMaster = true;
  uint8_t i;
	uint32_t fdev = 0;
	uint32_t datarate = 25000;
	uint8_t m,n,count;
	uint8_t paConfig = 0;
  uint8_t paDac = 0;
	
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
	
  GPIO_InitStruct.Pin   = GPIO_PIN_5;
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	HW_GPIO_Write(GPIOB,GPIO_PIN_5,GPIO_PIN_SET);
	
	GPIO_InitStruct.Pin   = GPIO_PIN_9;
  GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	
//	SX1276Reset();
////	HAL_TIM_Base_Start_IT(&TIM3_Handler);
//	
////	SX1276SetTxContinuousWave(RF_FREQUENCY,TX_OUTPUT_POWER,3000);
//	
//	SX1276SetChannel( RF_FREQUENCY );
//	
//	SX1276Write( REG_PACKETCONFIG2, ( SX1276Read( REG_PACKETCONFIG2 ) & RF_PACKETCONFIG2_DATAMODE_CONTINUOUS ) );
//	
//	SX1276Write( REG_DIOMAPPING1, RF_DIOMAPPING1_DIO0_11 | RF_DIOMAPPING1_DIO1_00 );
//  SX1276Write( REG_DIOMAPPING2, RF_DIOMAPPING2_DIO4_01 | RF_DIOMAPPING2_DIO5_01 );
//	
//	SX1276Write( REG_OPMODE, ( SX1276Read( REG_OPMODE ) & RF_OPMODE_MODULATIONTYPE_MASK ) | RF_OPMODE_MODULATIONTYPE_OOK );	
//	
//	SX1276Write( REG_PARAMP, ( SX1276Read( REG_PARAMP ) & RF_PARAMP_MASK ) | RF_PARAMP_0010_US );
//	
////	SX1276Write( REG_PARAMP, ( SX1276Read( REG_PARAMP ) & RF_PARAMP_MODULATIONSHAPING_MASK ) | RF_PARAMP_MODULATIONSHAPING_10 );
//	
////	SX1276Write( REG_PACONFIG, ( SX1276Read( REG_PACONFIG ) & RF_PACONFIG_MAX_POWER_MASK ) | 0x0f );
//	SX1276SetRfTxPower( 15);
//	
//	SX1276Write( REG_PLL, ( SX1276Read( REG_PLL ) & RF_PLL_BANDWIDTH_MASK ) | RF_PLL_BANDWIDTH_75 );

//	SX1276Write( REG_PLLHOP, ( SX1276Read( REG_PLLHOP ) & RF_PLLHOP_FASTHOP_MASK ) | RF_PLLHOP_FASTHOP_ON );	

//	fdev = ( uint16_t )( ( double )fdev / ( double )FREQ_STEP );
//  SX1276Write( REG_FDEVMSB, ( uint8_t )( fdev >> 8 ) );
//  SX1276Write( REG_FDEVLSB, ( uint8_t )( fdev & 0xFF ) );
//	
//	datarate = ( uint16_t )( ( double )XTAL_FREQ / ( double )datarate );
//	SX1276Write( REG_BITRATEMSB, ( uint8_t )( datarate >> 8 ) );
//	SX1276Write( REG_BITRATELSB, ( uint8_t )( datarate & 0xFF ) );
	
	Radio.Init(&RadioEvents);

  Radio.SetChannel(RF_FREQUENCY);

	Radio.SetTxContinuousWave(RF_FREQUENCY,TX_OUTPUT_POWER,3);

	SX1276Write( REG_PLLHOP, ( SX1276Read( REG_PLLHOP ) & RF_PLLHOP_FASTHOP_MASK ) | RF_PLLHOP_FASTHOP_ON );
	
	
//	HAL_TIM_Base_Start_IT(&TIM3_Handler);
	SX1276SetOpMode( RF_OPMODE_TRANSMITTER );
	DelayMs(100);
  while (1)
  {
		SX1276SetOpMode( RF_OPMODE_TRANSMITTER );
		for(m=0;m<8;m++)
		{
			LoRa_upChirp();
		}
//		SX1276SetOpMode (RF_OPMODE_SYNTHESIZER_TX);
//		for(n=0;n<2;n++)
//		{
//			LoRa_downChirp();
//		}
		SX1276SetOpMode( RF_OPMODE_SYNTHESIZER_TX );
		DelayMs(2000);
		
//		SX1276SetChannel( RF_FREQUENCY );
// 		for(i = 0; i < 2 ; i++)
//		{
//			HW_GPIO_Write(GPIOB,GPIO_PIN_5,GPIO_PIN_SET);
//			DelayMs(LED_PERIOD_MS);
////			HW_GPIO_Write(GPIOB,GPIO_PIN_5,GPIO_PIN_RESET);
////			DelayMs(200);
//		}
//		SX1276SetChannel( RF_FREQUENCY + LoRa_BW  );
//		for(i = 0; i < 2 ; i++)
//		{
//			HW_GPIO_Write(GPIOB,GPIO_PIN_5,GPIO_PIN_SET);
//			DelayMs(LED_PERIOD_MS);
////			HW_GPIO_Write(GPIOB,GPIO_PIN_5,GPIO_PIN_RESET);
////			DelayMs(200);
//		}
  }
}

void SetChannel( uint32_t freq )
{
    uint32_t channel;
			
		channel = freq / FREQ_STEP;
	
    SX1276Write( REG_FRFMSB, ( uint8_t )( ( channel >> 16 ) & 0xFF ) );
    SX1276Write( REG_FRFMID, ( uint8_t )( ( channel >> 8 ) & 0xFF ) );
    SX1276Write( REG_FRFLSB, ( uint8_t )( channel & 0xFF ) );
}


void LoRa_upChirp()
{
//	uint32_t i;
	uint16_t Count = 0;
	uint32_t time_temp = 0;
	uint32_t increaed = 0;
	
	time_count = 0;
	HAL_TIM_Base_Start_IT(&TIM3_Handler);
	Generate_chip(LoRa_Base_Freq);
//	HAL_TIM_Base_Start_IT(&TIM3_Handler);
	while(1)
	{
//		if()
		if((HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_6) == GPIO_PIN_SET) && \
			(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_9)  == GPIO_PIN_SET))
		{
			HAL_TIM_Base_Stop_IT(&TIM3_Handler);
			time_temp = time_count;
			time_count = 0;
			HAL_TIM_Base_Start_IT(&TIM3_Handler);
			increaed += ceil(time_temp / 8) * LoRa_Freq_Step;
			Generate_chip(LoRa_Base_Freq + increaed);
			if(increaed > LoRa_BW)
			{
				HAL_TIM_Base_Stop_IT(&TIM3_Handler);
				time_count = 0;
				break;
			}
			
//			HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_5);
//			Count ++;
//			Generate_chip(LoRa_Base_Freq + Count * LoRa_Freq_Step * 12);
//			if(Count * LoRa_Freq_Step * 12 > LoRa_BW)
//			{
//				HAL_TIM_Base_Stop_IT(&TIM3_Handler);
//				time_count = 0;
//				break;
//			}
//			Generate_chip(RF_FREQUENCY + Freq_Set[96 * Count]);
//			if(96 * Count >= 16384 - 1)
//			{
//				HAL_TIM_Base_Stop_IT(&TIM3_Handler);
//				time_count = 0;
//				break;
//			}
//			HAL_TIM_Base_Start_IT(&TIM3_Handler);
		}
	}
}


void LoRa_downChirp()
{
//	uint32_t i;
	uint16_t Count = 0;
	
	time_count = 0;
	Generate_chip(LoRa_Max_Freq);
	HAL_TIM_Base_Start_IT(&TIM3_Handler);
	while(1)
	{
		if(time_count > 35 )
		{
			HAL_TIM_Base_Stop_IT(&TIM3_Handler);
			time_count = 0;
			Count ++;
//			Generate_chip(LoRa_Max_Freq - Count * LoRa_Freq_Step * 12);

//			if(Count * LoRa_Freq_Step * 12 > LoRa_BW)
//			{
//				
//				HAL_TIM_Base_Stop_IT(&TIM3_Handler);
//				time_count = 0;
//				break;
//			}
			Generate_chip(RF_FREQUENCY - Freq_Set[96 * Count]);
			if(96 * Count >= 16384 - 1)
			{
				HAL_TIM_Base_Stop_IT(&TIM3_Handler);
				time_count = 0;
				break;
			}
			HAL_TIM_Base_Start_IT(&TIM3_Handler);
		}
	}
}



void Generate_chip( uint32_t freq )
{
	SX1276SetChannel( freq );
}


