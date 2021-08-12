#ifndef __FAST_SPI_H
#define __FAST_SPI_H
#include "sys.h"   
// SPI总线速度设置 
#define SPI_SPEED_2   		0
#define SPI_SPEED_4   		1
#define SPI_SPEED_8   		2
#define SPI_SPEED_16  		3
#define SPI_SPEED_32 			4
#define SPI_SPEED_64 			5
#define SPI_SPEED_128 		6
#define SPI_SPEED_256 		7

#define CS PBout(6)	// DS0



void SPI1_Init(void);			 //初始化SPI1口
void SPI1_SetSpeed(uint8_t SpeedSet); //设置SPI1速度   

//uint8_t SPI1_ReadByte(uint8_t TxData);
//uint8_t SPI1_WriteByte_u8(uint8_t TxData);
//uint8_t SPI1_WriteByte_u16(uint16_t TxData);
uint8_t SPI1_WriteByte_u8(uint8_t TxData);
uint16_t SPI1_WriteByte_u16(uint16_t TxData);

uint8_t CMT2300A_Burst_Write(uint8_t addr, uint8_t *pBuf, uint8_t len);
u8 SPI1_ReadWriteByte(u8 TxData);
#endif

