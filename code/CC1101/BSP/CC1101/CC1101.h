#ifndef __CC1101_H__
#define __CC1101_H__


#include "stm32l4xx_hal.h"
#include "fast_spi.h"
#include "delay.h"

/* Parameters header files */
#include "CC1101Regs.h"
#include "CC1101_Init_Regs.h"

#define FREQ_STEP_8                                 15625 /* FREQ_STEP<<8 */
#define SPI1_CS_LOW																	GPIOB->BRR = GPIO_PIN_6
#define SPI1_CS_HIGH																GPIOB->BSRR = GPIO_PIN_6

#define SX_FREQ_TO_CHANNEL( channel, freq )                                                                       \
    do                                                                                                            \
    {                                                                                                             \
        uint32_t initialFreqInt, initialFreqFrac;                                                                 \
        initialFreqInt = freq / FREQ_STEP_8;                                                                      \
        initialFreqFrac = freq - ( initialFreqInt * FREQ_STEP_8 );                                                \
        channel = ( initialFreqInt << 8 ) + ( ( ( initialFreqFrac << 8 ) + ( FREQ_STEP_8 / 2 ) ) / FREQ_STEP_8 ); \
    }while( 0 )


#define	SRES																				0x30
#define	SFSTXON                                     0x31
#define	SXOFF                                       0x32
#define	SCAL                                        0x33
#define	SRX                                         0x34
#define	STX                                         0x35
#define	SIDLE                                       0x36
#define	SWOR                                        0x38
#define	SPWD                                        0x39
#define	SFRX                                        0x3A
#define	SFTX                                        0x3B
#define	SWORRST                                     0x3C
#define	SNOP                                        0x3D

void CC1101_Init( void );
void CC1101_Reset( void );
void CC1101_Set_OpMode( uint8_t opMode );

uint8_t CC1101_Single_Read(uint8_t reg);
uint8_t CC1101_Single_Write(uint8_t reg, uint8_t TxData);

uint8_t CC1101_Burst_Read(uint8_t reg,uint8_t *pBuf,uint8_t len);
uint8_t CC1101_Burst_Write(uint8_t reg, uint8_t *pBuf, uint8_t len);

#endif /* __SX1276_H__ */
