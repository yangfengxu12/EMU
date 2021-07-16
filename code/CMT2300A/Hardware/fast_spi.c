#include "fast_spi.h"
//以下是SPI模块的初始化代码，配置成主机模式 						  
//SPI口初始化
//这里针是对SPI1的初始化
//void SPI1_Init(void)
//{	 
//	uint16_t tempreg=0;
//	RCC->AHB1ENR|=1<<0;    	//使能PORTA时钟	   
//	RCC->APB2ENR|=1<<12;   	//SPI1时钟使能 
//	GPIO_Set(GPIOA,7<<5,GPIO_MODE_AF,GPIO_OTYPE_PP,GPIO_SPEED_100M,GPIO_PUPD_PU);	//PA5~7复用功能输出	
////  	GPIO_AF_Set(GPIOB,3,5);	//PB3,AF5
//// 	GPIO_AF_Set(GPIOB,4,5);	//PB4,AF5
//// 	GPIO_AF_Set(GPIOB,5,5);	//PB5,AF5 

//	//这里只针对SPI口初始化
////	RCC->APB2RSTR|=1<<12;	//复位SPI1
////	RCC->APB2RSTR&=~(1<<12);//停止复位SPI1
//	tempreg|=0<<10;			//全双工模式	
//	tempreg|=1<<9;			//软件nss管理
//	tempreg|=1<<8;			 
//	tempreg|=1<<2;			//SPI主机  

//	tempreg|=0<<1;			// CPOL=0
//	tempreg|=0<<0;			//CPHA=0
// 	//对SPI1属于APB2的外设.时钟频率最大为80Mhz频率.
//	tempreg|=3<<3;			//Fsck=Fpclk1/8 
//	tempreg|=0<<7;			//MSB First
//	tempreg|=1<<6;			//SPI启动
//	
//	SPI1->CR2 = 1<<12;  //FIFO reception threshold 8bit
//	SPI1->CR2 = 7<<8;		//8位数据格式	
//	
//	
//	SPI1->CR1=tempreg; 		//设置CR1
////	SPI1->I2SCFGR&=~(1<<11);//选择SPI模式

//	
//	SPI1->CR1&=~(1<<6); 	//SPI设备失能
//	SPI1->CR1&=~(1<<1); 	//空闲模式下SCK为0 CPOL=0
//	SPI1->CR1&=~(1<<0); 	//数据采样从第1个时间边沿开始,CPHA=0  
//	SPI1->CR1|=1<<6; 		//SPI设备使能 
//		 
//}   
////SPI1速度设置函数
////SpeedSet:0~7
////SPI速度=fAPB2/2^(SpeedSet+1)
////fAPB2时钟一般为84Mhz
//void SPI1_SetSpeed(uint8_t SpeedSet)
//{
//	SpeedSet&=0X07;			//限制范围
//	SPI1->CR1&=0XFFC7; 
//	SPI1->CR1|=SpeedSet<<3;	//设置SPI1速度  
//	SPI1->CR1|=1<<6; 		//SPI设备使能	  
//} 


//////SPI1 读一个字节
//////TxData:要写入的字节
//////返回值:读取到的字节
////uint8_t SPI1_ReadByte(uint8_t addr)
////{		 			 
////	uint16_t reg;
////	GPIOB->BRR = GPIO_PIN_6;
////	while((SPI1->SR&1<<1)==0);		//等待发送区空 
////	SPI1->DR=(uint16_t )addr;	 	  		//发送一个byte  
////	while((SPI1->SR&1<<0)==0);		//等待接收完一个byte
////	reg = SPI1->DR;
////	while((SPI1->SR&1<<0)==0);
////	reg = SPI1->DR;
////	GPIOB->BSRR = GPIO_PIN_6;
//// 	return reg;        		//返回收到的数据		
////}

//uint8_t SPI1_ReadByte(uint8_t TxData)
//{		 			 
//	while((SPI1->SR&1<<1)==0);		//等待发送区空 	
//	*(__IO uint8_t *)&SPI1->DR = TxData;	//发送一个byte 
//	while((SPI1->SR&1<<1)==1);		//等待接收完一个byte  
// 	return SPI1->DR;          		//返回收到的数据				    
//}

uint8_t SPI1_WriteByte_u8(uint8_t TxData)
{		 			 
	while((SPI1->SR&1<<1)==0);		//等待发送区空 	
	*(__IO uint8_t *)&SPI1->DR = TxData;	//发送一个byte 
	while((SPI1->SR&1<<0)==0);		//等待接收完一个byte  
 	return SPI1->DR;          		//返回收到的数据				    
}

uint16_t SPI1_WriteByte_u16(uint16_t TxData)
{		 		

	while((SPI1->SR&1<<1)==0);		
	SPI1->DR = TxData;	
	while((SPI1->SR&1<<0)==0);
	return SPI1->DR;
 	           			    
}



//uint8_t SX1276_Burst_Read(uint8_t reg,uint8_t *pBuf,uint8_t len)
//{
//	uint8_t status,uint8_t_ctr;	       
//  	GPIOB->BRR = GPIO_PIN_6;           //使能SPI传输
//  	status=SPI1_ReadByte(reg | 0x80);//发送寄存器值(位置),并读取状态值   	   
// 	for(uint8_t_ctr=0;uint8_t_ctr<len;uint8_t_ctr++)pBuf[uint8_t_ctr]=SPI1_ReadByte(0XFF);//读出数据
//  	GPIOB->BSRR = GPIO_PIN_6;       //关闭SPI传输
//  	return status;        //返回读到的状态值
//}

uint8_t CMT2300A_Burst_Write(uint8_t addr, uint8_t *pBuf, uint8_t len)
{
	uint8_t ctr;	    
 	GPIOB->BRR = GPIO_PIN_6;          //使能SPI传输

	for(ctr=0;ctr<len;ctr++)
	{
		SPI1_WriteByte_u16(( (addr + ctr) << 8) |  pBuf[ctr] );
	}

	GPIOB->BSRR = GPIO_PIN_6;       //关闭SPI传输
	return 1;          //返回读到的状态值
}

SPI_HandleTypeDef SPI1_Handler;  //SPI1句柄

//以下是SPI模块的初始化代码，配置成主机模式 						  
//SPI口初始化
//这里针是对SPI1的初始化
void SPI1_Init(void)
{
    SPI1_Handler.Instance=SPI1;                         //SPI1
    SPI1_Handler.Init.Mode=SPI_MODE_MASTER;             //设置SPI工作模式，设置为主模式
    SPI1_Handler.Init.Direction=SPI_DIRECTION_2LINES;   //设置SPI单向或者双向的数据模式:SPI设置为双线模式
    SPI1_Handler.Init.DataSize=SPI_DATASIZE_16BIT;       //设置SPI的数据大小:SPI发送接收8位帧结构
    SPI1_Handler.Init.CLKPolarity=SPI_POLARITY_LOW;    //串行同步时钟的空闲状态为高电平
    SPI1_Handler.Init.CLKPhase=SPI_PHASE_1EDGE;         //串行同步时钟的第二个跳变沿（上升或下降）数据被采样
    SPI1_Handler.Init.NSS=SPI_NSS_SOFT;                 //NSS信号由硬件（NSS管脚）还是软件（使用SSI位）管理:内部NSS信号有SSI位控制
    SPI1_Handler.Init.BaudRatePrescaler=SPI_BAUDRATEPRESCALER_2;//定义波特率预分频的值:波特率预分频值为256
    SPI1_Handler.Init.FirstBit=SPI_FIRSTBIT_MSB;        //指定数据传输从MSB位还是LSB位开始:数据传输从MSB位开始
    SPI1_Handler.Init.TIMode=SPI_TIMODE_DISABLE;        //关闭TI模式
    SPI1_Handler.Init.CRCCalculation=SPI_CRCCALCULATION_DISABLE;//关闭硬件CRC校验
    SPI1_Handler.Init.CRCPolynomial=7;                  //CRC值计算的多项式
    HAL_SPI_Init(&SPI1_Handler);//初始化
    
    __HAL_SPI_ENABLE(&SPI1_Handler);                    //使能SPI1
		SET_BIT(SPI1->CR2, SPI_RXFIFO_THRESHOLD);
//    SPI1_ReadWriteByte(0Xff);                           //启动传输
}

//SPI5底层驱动，时钟使能，引脚配置
//此函数会被HAL_SPI_Init()调用
//hspi:SPI句柄
void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi)
{
    GPIO_InitTypeDef GPIO_Initure;
    
    __HAL_RCC_GPIOB_CLK_ENABLE();       //使能GPIOB时钟
		__HAL_RCC_GPIOA_CLK_ENABLE();       //使能GPIOB时钟
    __HAL_RCC_SPI1_CLK_ENABLE();        //使能SPI1时钟
    
    //PA5.6.7
    GPIO_Initure.Pin=GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;
    GPIO_Initure.Mode=GPIO_MODE_AF_PP;              //复用推挽输出
    GPIO_Initure.Pull=GPIO_NOPULL;                  //上拉
    GPIO_Initure.Speed=GPIO_SPEED_FREQ_VERY_HIGH;             //快速            
    GPIO_Initure.Alternate=GPIO_AF5_SPI1;           //复用为SPI1
    HAL_GPIO_Init(GPIOA,&GPIO_Initure);
	
		GPIO_Initure.Pin=GPIO_PIN_6;
		GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;
		HAL_GPIO_Init(GPIOB,&GPIO_Initure);
	
		HAL_GPIO_WritePin(GPIOA,GPIO_PIN_6,GPIO_PIN_SET);
}


//SPI速度设置函数
//SPI速度=fAPB1/分频系数
//@ref SPI_BaudRate_Prescaler:SPI_BAUDRATEPRESCALER_2~SPI_BAUDRATEPRESCALER_2 256
//fAPB1时钟一般为42Mhz：
void SPI1_SetSpeed(u8 SPI_BaudRatePrescaler)
{
    assert_param(IS_SPI_BAUDRATE_PRESCALER(SPI_BaudRatePrescaler));//判断有效性
    __HAL_SPI_DISABLE(&SPI1_Handler);            //关闭SPI
    SPI1_Handler.Instance->CR1&=0XFFC7;          //位3-5清零，用来设置波特率
    SPI1_Handler.Instance->CR1|=SPI_BaudRatePrescaler;//设置SPI速度
    __HAL_SPI_ENABLE(&SPI1_Handler);             //使能SPI
    
}

//SPI1 读写一个字节
//TxData:要写入的字节
//返回值:读取到的字节
u8 SPI1_ReadWriteByte(u8 TxData)
{
    u8 Rxdata;
    HAL_SPI_TransmitReceive(&SPI1_Handler,&TxData,&Rxdata,1, 1000);       
 	return Rxdata;          		    //返回收到的数据		
}




