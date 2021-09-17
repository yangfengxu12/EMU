#include "fast_spi.h"

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
	tempreg|=2<<3;			//Fsck=Fpclk1/16
	tempreg|=0<<7;			//MSB First
	tempreg|=1<<6;			//SPI启动
	
	SPI1->CR2 |= 1<<12;  //FIFO reception threshold 8bit
	SPI1->CR2 |= 7<<8;		//8位数据格式	
	
	
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


uint8_t SPI1_ReadWriteByte_u8(uint8_t TxData)
{		 			 
	while((SPI1->SR&1<<1)==0);		//等待发送区空 	
	*(__IO uint8_t *)&SPI1->DR = TxData;	//发送一个byte 
	while((SPI1->SR&1<<0)==0);		//等待接收完一个byte  
 	return SPI1->DR;          		//返回收到的数据				    
}

uint16_t SPI1_ReadWriteByte_u16(uint16_t TxData)
{		 			 
	uint8_t value;
	while((SPI1->SR&1<<1)==0);		//等待发送区空 
	SPI1->DR = TxData;	//发送一个byte 
	while((SPI1->SR&1<<0)==0);		//等待接收完一个byte  
  value = SPI1->DR;
	while(!(SPI1->SR&1<<0));
 	return ((SPI1->DR)<<8) | value;          		//返回收到的数据				    
}










