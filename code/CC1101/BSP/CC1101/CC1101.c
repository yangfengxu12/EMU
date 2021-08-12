#include "CC1101.h"

void CC1101_Init( void )
{
	CC1101_Reset();
	CC1101_Burst_Write( REG_IOCFG2, CC1101_Init_Setting, sizeof( CC1101_Init_Setting ));
}

void CC1101_Reset( void )
{
	SPI1_CS_LOW;
	SPI1_ReadWriteByte_u8(REG_SRES);
	SPI1_CS_HIGH;
	delay_ms(1); // CC1101 spend 30~50us to reset
}

void CC1101_Set_OpMode( uint8_t opMode )
{
	SPI1_CS_LOW;
	SPI1_ReadWriteByte_u8( opMode );
	SPI1_CS_HIGH;
}
 
void CC1101_Set_TxPower( void )
{
	SPI1_CS_LOW;
	CC1101_Burst_Write( 0x3E, CC1101_PA_Table_Settings, 8);
	SPI1_CS_HIGH;
}

uint8_t CC1101_Single_Read(uint8_t reg)
{
	uint16_t value;
	SPI1_CS_LOW;
	while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6) == GPIO_PIN_SET);
	value = SPI1_ReadWriteByte_u16(( 0xFF << 8 ) | ( reg | 0x80 ));
	SPI1_CS_HIGH;
	return ( value >> 8 );
}
	
uint8_t CC1101_Single_Write(uint8_t reg, uint8_t TxData)
{
	SPI1_CS_LOW;
	while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6) == GPIO_PIN_SET);
	SPI1_ReadWriteByte_u16(( TxData << 8)| reg );
	SPI1_CS_HIGH;
	return 1;
}

uint8_t CC1101_Burst_Read(uint8_t reg,uint8_t *pBuf,uint8_t len)
{
	uint8_t uint8_t_ctr;
	uint16_t value;
	if( reg <= 0x2F )
	{
		reg += 0xC0;
	}
	else
	{
		return 0;
	}
 	SPI1_CS_LOW;          //使能SPI传输
	while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6) == GPIO_PIN_SET);
	if( len & 1 ) // len = odd  
	{
		value = SPI1_ReadWriteByte_u16(( 0xFF << 8 ) | reg );
		pBuf[0] = value >> 8;
		if( len > 1 ) // len = 3,5,7.....
		{
			for(uint8_t_ctr = 0; uint8_t_ctr < (( len - 1 ) >> 1 ); uint8_t_ctr++ )
			{
				value = SPI1_ReadWriteByte_u16( 0xFFFF ); //写入数据
				pBuf[ uint8_t_ctr * 2 + 1 ] = value;
				pBuf[ uint8_t_ctr * 2 + 2 ] = value >> 8;
			}
		}
	}
	else	// len = even
	{
		SPI1_ReadWriteByte_u8( reg );
		if( len >= 1 )	// len = 2,4,6...
		{
			for(uint8_t_ctr = 0; uint8_t_ctr < ( len >> 1 ); uint8_t_ctr++ )
			{
				value = SPI1_ReadWriteByte_u16( 0xFFFF ); //写入数据
				pBuf[ uint8_t_ctr * 2 ] = value;
				pBuf[ uint8_t_ctr * 2 + 1 ] = value >> 8;
			}				
		}
	}
	SPI1_CS_HIGH;       //关闭SPI传输
	return 1;          //返回读到的状态值
}

uint8_t CC1101_Burst_Write(uint8_t reg, uint8_t *pBuf, uint8_t len)
{
	uint8_t uint8_t_ctr;
	if( reg <= 0x2F )
	{
		reg += 0x40;
	}
	else
	{
		return 0;
	}
 	SPI1_CS_LOW;          //使能SPI传输
	while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6) == GPIO_PIN_SET);
	if( len & 1 ) // len = odd  
	{
		SPI1_ReadWriteByte_u16(( pBuf[0] << 8 ) | reg );
		if( len > 1 ) // len = 3,5,7.....
		{
			for(uint8_t_ctr = 0; uint8_t_ctr < (( len - 1 ) >> 1 ); uint8_t_ctr++ )
				SPI1_ReadWriteByte_u16(( pBuf[ uint8_t_ctr * 2 + 2 ] << 8 ) | pBuf[ uint8_t_ctr * 2 + 1 ]); //写入数据	 
		}
	}
	else	// len = even
	{
		SPI1_ReadWriteByte_u8( reg | 0x40 );
		if( len >= 1 )	// len = 2,4,6...
		{
			for(uint8_t_ctr = 0; uint8_t_ctr < ( len >> 1 ); uint8_t_ctr++ )
				SPI1_ReadWriteByte_u16(( pBuf[ uint8_t_ctr * 2 + 1 ] << 8 ) | pBuf[ uint8_t_ctr * 2]); //写入数据	 
		}
	}
	SPI1_CS_HIGH;       //关闭SPI传输
	return 1;          //返回读到的状态值
}

