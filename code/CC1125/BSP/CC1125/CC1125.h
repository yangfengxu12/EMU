#ifndef __CC1125_H__
#define __CC1125_H__


#include "stm32l4xx_hal.h"
#include "fast_spi.h"
#include "delay.h"

#define SPI1_CS_LOW																	GPIOB->BRR = GPIO_PIN_6
#define SPI1_CS_HIGH																GPIOB->BSRR = GPIO_PIN_6


void CC1125_Init( void );
void CC1125_Reg_Init( void );
void CC1125_Reset( void );
void CC1125_Set_OpMode( uint8_t opMode );
void CC1125_Set_TxPower( void );


uint16_t CC1125_Single_Read(uint16_t reg);
uint16_t CC1125_Single_Write(uint16_t reg, uint8_t TxData);

uint16_t CC1125_Burst_Read(uint16_t reg,uint8_t *pBuf,uint8_t len);
uint16_t CC1125_Burst_Write(uint16_t reg, uint8_t *pBuf, uint8_t len);

#endif /* __SX1276_H__ */
