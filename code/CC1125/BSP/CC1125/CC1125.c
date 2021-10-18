#include "CC1125.h"

/* Parameters header files */
#include "CC1125_Regs.h"
#include "CC1125_Init_Regs.h"

void CC1125_Init( void )
{
	CC1125_Reset();
	CC1125_Reg_Init();
}

void CC1125_Reset( void )
{
	// manually reset by using GPIO
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_0,GPIO_PIN_RESET);
	delay_ms(1);
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_0,GPIO_PIN_SET);
	delay_ms(1);
	
	//strobe reset by using strobe commond, POWER ON does not work.
//	CC1125_Set_OpMode(SRES);
//	delay_ms(1);
}

void CC1125_Set_OpMode( uint8_t opMode )
{
	SPI1_CS_LOW;
	SPI1_ReadWriteByte_u8( opMode );
	SPI1_CS_HIGH;
}
 
void CC1125_Set_TxPower( void )
{
	SPI1_CS_LOW;
//	CC1125_Burst_Write( 0x3E, CC1125_PA_Table_Settings, 8);
	SPI1_CS_HIGH;
}

uint16_t CC1125_Single_Read(uint16_t reg)
{
	uint16_t value;
	SPI1_CS_LOW;
	while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6) == GPIO_PIN_SET);
	if( reg >= 0x2F00)
	{
		value = SPI1_ReadWriteByte_u16( ((reg&0xFF)<<8) | ( 0x2F | 0x80 ));
		value = SPI1_ReadWriteByte_u8( 0xFF );
	}
	else
	{
		value = SPI1_ReadWriteByte_u16( (reg&0xFF) | 0x80 );
	}
	SPI1_CS_HIGH;
	return value;
}
	
uint16_t CC1125_Single_Write(uint16_t reg, uint8_t TxData)
{
	uint16_t value;
	SPI1_CS_LOW;
	while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6) == GPIO_PIN_SET);
	if( reg >= 0x2F00)
	{
		value = SPI1_ReadWriteByte_u16( ((reg&0xFF)<<8) | ( 0x2F | 0x00 ));
		value = SPI1_ReadWriteByte_u8( TxData );
	}
	else
	{
		value = SPI1_ReadWriteByte_u16( (TxData<<8) | (reg&0xFF) | 0x00 );
	}
	SPI1_CS_HIGH;
	return 1;
}

uint16_t CC1125_Burst_Read(uint16_t reg,uint8_t *pBuf,uint8_t len)
{
	uint8_t uint8_t_ctr;
	uint16_t value;
	SPI1_CS_LOW;          //使能SPI传输
	while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6) == GPIO_PIN_SET);
	if( reg < 0x2F00 )
	{
		if( len & 1 ) // len = odd  
		{
			value = SPI1_ReadWriteByte_u16( 0xFF00 | (reg&0xFF) | 0xC0 );
//			value = SPI1_ReadWriteByte_u16(( reg << 8 ) | ( reg>>8 )| 0x80 );
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
			SPI1_ReadWriteByte_u8( (reg&0xFF) | 0xC0 );
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
	}
	else // register address > 0x2F00
	{
		value = SPI1_ReadWriteByte_u16( ((reg&0xFF)<<8) | 0x2F | 0xC0 );
		
		if( len & 1 ) // len = odd  
		{
			if( len > 1 ) // len = 3,5,7.....
			{
				for(uint8_t_ctr = 0; uint8_t_ctr < (( len - 1 ) >> 1 ); uint8_t_ctr++ )
				{
					value = SPI1_ReadWriteByte_u16( 0xFFFF ); //写入数据
					pBuf[ uint8_t_ctr * 2] = value;
					pBuf[ uint8_t_ctr * 2 + 1 ] = value >> 8;
				}
				pBuf[len-1]=SPI1_ReadWriteByte_u8( 0xFF );
			}
			else
			{
				pBuf[0]=SPI1_ReadWriteByte_u8( 0xFF );
			}
		}
		else	// len = even
		{
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
	}
	SPI1_CS_HIGH;       //关闭SPI传输
	return 1;          //返回读到的状态值
}

uint16_t CC1125_Burst_Write(uint16_t reg, uint8_t *pBuf, uint8_t len)
{
	uint8_t uint8_t_ctr;
	uint16_t value;
	SPI1_CS_LOW;          //使能SPI传输
	while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6) == GPIO_PIN_SET);
	if( reg < 0x2F00 )
	{
		if( len & 1 ) // len = odd  
		{
			SPI1_ReadWriteByte_u16( (pBuf[0]<<8) | (reg&0xFF) | 0x40 );
			if( len > 1 ) // len = 3,5,7.....
			{
				for(uint8_t_ctr = 0; uint8_t_ctr < (( len - 1 ) >> 1 ); uint8_t_ctr++ )
				{
					SPI1_ReadWriteByte_u16( (pBuf[ uint8_t_ctr * 2 + 2 ]<<8) | pBuf[ uint8_t_ctr * 2 + 1 ] ); //写入数据
				}
			}
		}
		else	// len = even
		{
			SPI1_ReadWriteByte_u8( (pBuf[0]<<8) | 0x40 );
			if( len >= 1 )	// len = 2,4,6...
			{
				for(uint8_t_ctr = 0; uint8_t_ctr < ( len >> 1 ); uint8_t_ctr++ )
				{
					SPI1_ReadWriteByte_u16( (pBuf[ uint8_t_ctr * 2 + 1 ]<<8) | pBuf[ uint8_t_ctr * 2 ] ); //写入数据
				}				
			}
		}
	}
	else // register address > 0x2F00
	{
		value = SPI1_ReadWriteByte_u16( ((reg&0xFF)<<8) | 0x2F | 0x40 );
		
		if( len & 1 ) // len = odd  
		{
			if( len > 1 ) // len = 3,5,7.....
			{
				for(uint8_t_ctr = 0; uint8_t_ctr < (( len - 1 ) >> 1 ); uint8_t_ctr++ )
				{
					value = SPI1_ReadWriteByte_u16( ( pBuf[ uint8_t_ctr * 2 + 1 ]<<8 )| pBuf[ uint8_t_ctr * 2] ); //写入数据
				}
				SPI1_ReadWriteByte_u8( pBuf[ len - 1 ] );
			}
			else
			{
				SPI1_ReadWriteByte_u8( pBuf[0] );
			}
		}
		else	// len = even
		{
			if( len >= 1 )	// len = 2,4,6...
			{
				for(uint8_t_ctr = 0; uint8_t_ctr < ( len >> 1 ); uint8_t_ctr++ )
				{
					SPI1_ReadWriteByte_u16( ( pBuf[ uint8_t_ctr * 2 + 1 ]<<8 )| pBuf[ uint8_t_ctr * 2] ); //写入数据
				}				
			}
		}
	}
	SPI1_CS_HIGH;       //关闭SPI传输
	return 1;          //返回读到的状态值
}



void CC1125_Reg_Init()
{
    // Address Config = No address check 
    // Bit Rate = 100 
    // Carrier Frequency = 435.000000 
    // Deviation = 82.702637 
    // Device Address = 0 
    // Manchester Enable = false 
    // Modulation Format = ASK/OOK 
    // PA Ramping = false 
    // Packet Bit Length = 0 
    // Packet Length = 3 
    // Packet Length Mode = Variable 
    // Performance Mode = High Performance 
    // RX Filter BW = 250.000000 
    // Symbol rate = 100 
    // TX Power = 15 
    // Whitening = false 
    CC1125_Single_Write(REG_IOCFG3,0x31);            //GPIO3 IO Pin Configuration
    CC1125_Single_Write(REG_IOCFG2,0x08);            //GPIO2 IO Pin Configuration
    CC1125_Single_Write(REG_IOCFG1,0xB0);            //GPIO1 IO Pin Configuration
    CC1125_Single_Write(REG_IOCFG0,0x09);            //GPIO0 IO Pin Configuration
    CC1125_Single_Write(REG_SYNC_CFG1,0x0B);         //Sync Word Detection Configuration Reg. 1
    CC1125_Single_Write(REG_DEVIATION_M,0x0F);       //Frequency Deviation Configuration
    CC1125_Single_Write(REG_MODCFG_DEV_E,0x1F);      //Modulation Format and Frequency Deviation Configur..
    CC1125_Single_Write(REG_DCFILT_CFG,0x04);        //Digital DC Removal Configuration
    CC1125_Single_Write(REG_PREAMBLE_CFG1,0x00);     //Preamble Length Configuration Reg. 1
    CC1125_Single_Write(REG_FREQ_IF_CFG,0x00);       //RX Mixer Frequency Configuration
    CC1125_Single_Write(REG_IQIC,0x00);              //Digital Image Channel Compensation Configuration
    CC1125_Single_Write(REG_CHAN_BW,0x81);           //Channel Filter Configuration
    CC1125_Single_Write(REG_MDMCFG1,0x06);           //General Modem Parameter Configuration Reg. 1
    CC1125_Single_Write(REG_MDMCFG0,0x45);           //General Modem Parameter Configuration Reg. 0
    CC1125_Single_Write(REG_SYMBOL_RATE2,0xA4);      //Symbol Rate Configuration Exponent and Mantissa [1..
    CC1125_Single_Write(REG_SYMBOL_RATE1,0x7A);      //Symbol Rate Configuration Mantissa [15:8]
    CC1125_Single_Write(REG_SYMBOL_RATE0,0xE1);      //Symbol Rate Configuration Mantissa [7:0]
    CC1125_Single_Write(REG_AGC_REF,0x3C);           //AGC Reference Level Configuration
    CC1125_Single_Write(REG_AGC_CS_THR,0xEC);        //Carrier Sense Threshold Configuration
    CC1125_Single_Write(REG_AGC_CFG3,0x83);          //Automatic Gain Control Configuration Reg. 3
    CC1125_Single_Write(REG_AGC_CFG2,0x60);          //Automatic Gain Control Configuration Reg. 2
    CC1125_Single_Write(REG_AGC_CFG1,0xA9);          //Automatic Gain Control Configuration Reg. 1
    CC1125_Single_Write(REG_AGC_CFG0,0xC0);          //Automatic Gain Control Configuration Reg. 0
    CC1125_Single_Write(REG_FIFO_CFG,0x00);          //FIFO Configuration
    CC1125_Single_Write(REG_FS_CFG,0x14);            //Frequency Synthesizer Configuration
    CC1125_Single_Write(REG_PKT_CFG2,0x07);          //Packet Configuration Reg. 2
    CC1125_Single_Write(REG_PKT_CFG1,0x00);          //Packet Configuration Reg. 1
    CC1125_Single_Write(REG_PKT_CFG0,0x20);          //Packet Configuration Reg. 0
    CC1125_Single_Write(REG_PA_CFG2,0x3F);           //Power Amplifier Configuration Reg. 2
    CC1125_Single_Write(REG_PA_CFG0,0x79);           //Power Amplifier Configuration Reg. 0
    CC1125_Single_Write(REG_IF_MIX_CFG,0x00);        //IF Mix Configuration
    CC1125_Single_Write(REG_TOC_CFG,0x0A);           //Timing Offset Correction Configuration
    CC1125_Single_Write(REG_ECG_CFG,0x0C);           //External Clock Frequency Configuration
    CC1125_Single_Write(REG_FREQ2,0x56);             //Frequency Configuration [23:16]
		CC1125_Single_Write(REG_FREQ1,0xFC);             //Frequency Configuration [23:16]
		CC1125_Single_Write(REG_FREQ0,0xCC);             //Frequency Configuration [23:16]
    CC1125_Single_Write(REG_IF_ADC0,0x05);           //Analog to Digital Converter Configuration Reg. 0
    CC1125_Single_Write(REG_FS_DIG1,0x00);           //Frequency Synthesizer Digital Reg. 1
    CC1125_Single_Write(REG_FS_DIG0,0x5F);           //Frequency Synthesizer Digital Reg. 0
    CC1125_Single_Write(REG_FS_CAL0,0x0E);           //Frequency Synthesizer Calibration Reg. 0
    CC1125_Single_Write(REG_FS_DIVTWO,0x03);         //Frequency Synthesizer Divide by 2
    CC1125_Single_Write(REG_FS_DSM0,0x33);           //FS Digital Synthesizer Module Configuration Reg. 0
    CC1125_Single_Write(REG_FS_DVC0,0x17);           //Frequency Synthesizer Divider Chain Configuration ..
    CC1125_Single_Write(REG_FS_PFD,0x50);            //Frequency Synthesizer Phase Frequency Detector Con..
    CC1125_Single_Write(REG_FS_PRE,0x6E);            //Frequency Synthesizer Prescaler Configuration
    CC1125_Single_Write(REG_FS_REG_DIV_CML,0x14);    //Frequency Synthesizer Divider Regulator Configurat..
    CC1125_Single_Write(REG_FS_SPARE,0xAC);          //Frequency Synthesizer Spare
    CC1125_Single_Write(REG_XOSC5,0x0E);             //Crystal Oscillator Configuration Reg. 5
    CC1125_Single_Write(REG_XOSC3,0xC7);             //Crystal Oscillator Configuration Reg. 3
    CC1125_Single_Write(REG_XOSC1,0x07);             //Crystal Oscillator Configuration Reg. 1
    CC1125_Single_Write(REG_SERIAL_STATUS,0x08);     //Serial Status



}
