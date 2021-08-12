
#ifndef _BSP_H_
#define _BSP_H_

#include "stm32l4xx_hal.h"
#include "si446x.h"
#define GPIO_ResetBits(port,pin) HAL_GPIO_WritePin(port,pin,GPIO_PIN_RESET)
#define GPIO_SetBits(port,pin) HAL_GPIO_WritePin(port,pin,GPIO_PIN_SET)

#define SI_CSN_LOW( )   GPIO_ResetBits( GPIOA, GPIO_PIN_4 );
#define SI_CSN_HIGH( )  GPIO_SetBits( GPIOA, GPIO_PIN_4 );

#define SI_SDN_LOW( )   GPIO_ResetBits( GPIOB, GPIO_PIN_6 );
#define SI_SDN_HIGH( )  GPIO_SetBits( GPIOB, GPIO_PIN_6);









uint8_t SPI_ExchangeByte(uint8_t input);
void SI446x_RX_Test(void);
void SI446x_TX_Test(void);

#endif //_BSP_H_
/*
=================================================================================
------------------------------------End of File----------------------------------
=================================================================================
*/

