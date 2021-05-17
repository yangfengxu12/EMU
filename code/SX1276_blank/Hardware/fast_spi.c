#include "fast_spi.h"
//以下是SPI模块的初始化代码，配置成主机模式 						  
//SPI口初始化
//这里针是对SPI1的初始化
void SPI1_Init(void)
{	 
	uint16_t tempreg=0;
	RCC->AHB1ENR|=1<<0;    	//使能PORTA时钟	   
	RCC->APB2ENR|=1<<12;   	//SPI1时钟使能 
	GPIO_Set(GPIOA,7<<5,GPIO_MODE_AF,GPIO_OTYPE_PP,GPIO_SPEED_100M,GPIO_PUPD_PU);	//PA5~7复用功能输出	
//  	GPIO_AF_Set(GPIOB,3,5);	//PB3,AF5
// 	GPIO_AF_Set(GPIOB,4,5);	//PB4,AF5
// 	GPIO_AF_Set(GPIOB,5,5);	//PB5,AF5 

	//这里只针对SPI口初始化
//	RCC->APB2RSTR|=1<<12;	//复位SPI1
//	RCC->APB2RSTR&=~(1<<12);//停止复位SPI1
	tempreg|=0<<10;			//全双工模式	
	tempreg|=1<<9;			//软件nss管理
	tempreg|=1<<8;			 
	tempreg|=1<<2;			//SPI主机  

	tempreg|=0<<1;			// CPOL=0
	tempreg|=0<<0;			//CPHA=0
 	//对SPI1属于APB2的外设.时钟频率最大为80Mhz频率.
	tempreg|=2<<3;			//Fsck=Fpclk1/8 
	tempreg|=0<<7;			//MSB First
	tempreg|=1<<6;			//SPI启动
	
	SPI1->CR2 = 1<<12;  //FIFO reception threshold 8bit
	SPI1->CR2 = 7<<8;		//8位数据格式	
	
	
	SPI1->CR1=tempreg; 		//设置CR1
//	SPI1->I2SCFGR&=~(1<<11);//选择SPI模式

	
	SPI1->CR1&=~(1<<6); 	//SPI设备失能
	SPI1->CR1&=~(1<<1); 	//空闲模式下SCK为0 CPOL=0
	SPI1->CR1&=~(1<<0); 	//数据采样从第1个时间边沿开始,CPHA=0  
	SPI1->CR1|=1<<6; 		//SPI设备使能 
		 
}   
//SPI1速度设置函数
//SpeedSet:0~7
//SPI速度=fAPB2/2^(SpeedSet+1)
//fAPB2时钟一般为84Mhz
void SPI1_SetSpeed(uint8_t SpeedSet)
{
	SpeedSet&=0X07;			//限制范围
	SPI1->CR1&=0XFFC7; 
	SPI1->CR1|=SpeedSet<<3;	//设置SPI1速度  
	SPI1->CR1|=1<<6; 		//SPI设备使能	  
} 


////SPI1 读一个字节
////TxData:要写入的字节
////返回值:读取到的字节
//uint8_t SPI1_ReadByte(uint8_t addr)
//{		 			 
//	uint16_t reg;
//	GPIOB->BRR = GPIO_PIN_6;
//	while((SPI1->SR&1<<1)==0);		//等待发送区空 
//	SPI1->DR=(uint16_t )addr;	 	  		//发送一个byte  
//	while((SPI1->SR&1<<0)==0);		//等待接收完一个byte
//	reg = SPI1->DR;
//	while((SPI1->SR&1<<0)==0);
//	reg = SPI1->DR;
//	GPIOB->BSRR = GPIO_PIN_6;
// 	return reg;        		//返回收到的数据		
//}

uint8_t SPI1_ReadByte(uint8_t TxData)
{		 			 
	while((SPI1->SR&1<<1)==0);		//等待发送区空 	
	*(__IO uint8_t *)&SPI1->DR = TxData;	//发送一个byte 
	while((SPI1->SR&1<<0)==0);		//等待接收完一个byte  
 	return SPI1->DR;          		//返回收到的数据				    
}

uint8_t SPI1_WriteByte_u8(uint8_t TxData)
{		 			 
	while((SPI1->SR&1<<1)==0);		//等待发送区空 	
	*(__IO uint8_t *)&SPI1->DR = TxData;	//发送一个byte 
	while((SPI1->SR&1<<0)==0);		//等待接收完一个byte  
 	return SPI1->DR;          		//返回收到的数据				    
}

uint8_t SPI1_WriteByte_u16(uint16_t TxData)
{		 			 
	uint8_t value;
//	while(!(SPI1->SR&1<<1));		//等待发送区空 	
	SPI1->DR = TxData;	//发送一个byte 
	while(!(SPI1->SR&1<<0));		//等待接收完一个byte
  value = SPI1->DR;
	while(!(SPI1->SR&1<<0));
 	return SPI1->DR;          		//返回收到的数据				    
}



uint8_t SX1276_Burst_Read(uint8_t reg,uint8_t *pBuf,uint8_t len)
{
	uint8_t status,uint8_t_ctr;	       
  	GPIOB->BRR = GPIO_PIN_6;           //使能SPI传输
  	status=SPI1_ReadByte(reg);//发送寄存器值(位置),并读取状态值   	   
 	for(uint8_t_ctr=0;uint8_t_ctr<len;uint8_t_ctr++)pBuf[uint8_t_ctr]=SPI1_ReadByte(0XFF);//读出数据
  	GPIOB->BSRR = GPIO_PIN_6;       //关闭SPI传输
  	return status;        //返回读到的状态值
}

uint8_t SX1276_Burst_Write(uint8_t reg, uint8_t *pBuf, uint8_t len)
{
	uint8_t uint8_t_ctr;	    
 	GPIOB->BRR = GPIO_PIN_6;          //使能SPI传输

	if( len & 1 ) // len = odd  
	{
		SPI1_WriteByte_u16(( pBuf[0] << 8 ) | ( reg | 0x80 ));
		if( len > 1 ) // len = 3,5,7.....
		{
			for(uint8_t_ctr = 0; uint8_t_ctr < (( len - 1 ) >> 1 ); uint8_t_ctr++ )
				SPI1_WriteByte_u16(( pBuf[ uint8_t_ctr + 2 ] << 8 ) | pBuf[ uint8_t_ctr + 1 ]); //写入数据	 
		}
	}
	else	// len = even
	{
		SPI1_WriteByte_u8( reg | 0x80 );
		if( len >= 1 )	// len = 2,4,6...
		{
			for(uint8_t_ctr = 0; uint8_t_ctr < ( len >> 1 ); uint8_t_ctr++ )
				SPI1_WriteByte_u16(( pBuf[ uint8_t_ctr + 1 ] << 8 ) | pBuf[ uint8_t_ctr ]); //写入数据	 
		}
	}
	GPIOB->BSRR = GPIO_PIN_6;       //关闭SPI传输
	return 1;          //返回读到的状态值
}







