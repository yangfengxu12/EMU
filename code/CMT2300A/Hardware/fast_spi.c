#include "fast_spi.h"
//������SPIģ��ĳ�ʼ�����룬���ó�����ģʽ 						  
//SPI�ڳ�ʼ��
//�������Ƕ�SPI1�ĳ�ʼ��
//void SPI1_Init(void)
//{	 
//	uint16_t tempreg=0;
//	RCC->AHB1ENR|=1<<0;    	//ʹ��PORTAʱ��	   
//	RCC->APB2ENR|=1<<12;   	//SPI1ʱ��ʹ�� 
//	GPIO_Set(GPIOA,7<<5,GPIO_MODE_AF,GPIO_OTYPE_PP,GPIO_SPEED_100M,GPIO_PUPD_PU);	//PA5~7���ù������	
////  	GPIO_AF_Set(GPIOB,3,5);	//PB3,AF5
//// 	GPIO_AF_Set(GPIOB,4,5);	//PB4,AF5
//// 	GPIO_AF_Set(GPIOB,5,5);	//PB5,AF5 

//	//����ֻ���SPI�ڳ�ʼ��
////	RCC->APB2RSTR|=1<<12;	//��λSPI1
////	RCC->APB2RSTR&=~(1<<12);//ֹͣ��λSPI1
//	tempreg|=0<<10;			//ȫ˫��ģʽ	
//	tempreg|=1<<9;			//���nss����
//	tempreg|=1<<8;			 
//	tempreg|=1<<2;			//SPI����  

//	tempreg|=0<<1;			// CPOL=0
//	tempreg|=0<<0;			//CPHA=0
// 	//��SPI1����APB2������.ʱ��Ƶ�����Ϊ80MhzƵ��.
//	tempreg|=3<<3;			//Fsck=Fpclk1/8 
//	tempreg|=0<<7;			//MSB First
//	tempreg|=1<<6;			//SPI����
//	
//	SPI1->CR2 = 1<<12;  //FIFO reception threshold 8bit
//	SPI1->CR2 = 7<<8;		//8λ���ݸ�ʽ	
//	
//	
//	SPI1->CR1=tempreg; 		//����CR1
////	SPI1->I2SCFGR&=~(1<<11);//ѡ��SPIģʽ

//	
//	SPI1->CR1&=~(1<<6); 	//SPI�豸ʧ��
//	SPI1->CR1&=~(1<<1); 	//����ģʽ��SCKΪ0 CPOL=0
//	SPI1->CR1&=~(1<<0); 	//���ݲ����ӵ�1��ʱ����ؿ�ʼ,CPHA=0  
//	SPI1->CR1|=1<<6; 		//SPI�豸ʹ�� 
//		 
//}   
////SPI1�ٶ����ú���
////SpeedSet:0~7
////SPI�ٶ�=fAPB2/2^(SpeedSet+1)
////fAPB2ʱ��һ��Ϊ84Mhz
//void SPI1_SetSpeed(uint8_t SpeedSet)
//{
//	SpeedSet&=0X07;			//���Ʒ�Χ
//	SPI1->CR1&=0XFFC7; 
//	SPI1->CR1|=SpeedSet<<3;	//����SPI1�ٶ�  
//	SPI1->CR1|=1<<6; 		//SPI�豸ʹ��	  
//} 


//////SPI1 ��һ���ֽ�
//////TxData:Ҫд����ֽ�
//////����ֵ:��ȡ�����ֽ�
////uint8_t SPI1_ReadByte(uint8_t addr)
////{		 			 
////	uint16_t reg;
////	GPIOB->BRR = GPIO_PIN_6;
////	while((SPI1->SR&1<<1)==0);		//�ȴ��������� 
////	SPI1->DR=(uint16_t )addr;	 	  		//����һ��byte  
////	while((SPI1->SR&1<<0)==0);		//�ȴ�������һ��byte
////	reg = SPI1->DR;
////	while((SPI1->SR&1<<0)==0);
////	reg = SPI1->DR;
////	GPIOB->BSRR = GPIO_PIN_6;
//// 	return reg;        		//�����յ�������		
////}

//uint8_t SPI1_ReadByte(uint8_t TxData)
//{		 			 
//	while((SPI1->SR&1<<1)==0);		//�ȴ��������� 	
//	*(__IO uint8_t *)&SPI1->DR = TxData;	//����һ��byte 
//	while((SPI1->SR&1<<1)==1);		//�ȴ�������һ��byte  
// 	return SPI1->DR;          		//�����յ�������				    
//}

uint8_t SPI1_WriteByte_u8(uint8_t TxData)
{		 			 
	while((SPI1->SR&1<<1)==0);		//�ȴ��������� 	
	*(__IO uint8_t *)&SPI1->DR = TxData;	//����һ��byte 
	while((SPI1->SR&1<<0)==0);		//�ȴ�������һ��byte  
 	return SPI1->DR;          		//�����յ�������				    
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
//  	GPIOB->BRR = GPIO_PIN_6;           //ʹ��SPI����
//  	status=SPI1_ReadByte(reg | 0x80);//���ͼĴ���ֵ(λ��),����ȡ״ֵ̬   	   
// 	for(uint8_t_ctr=0;uint8_t_ctr<len;uint8_t_ctr++)pBuf[uint8_t_ctr]=SPI1_ReadByte(0XFF);//��������
//  	GPIOB->BSRR = GPIO_PIN_6;       //�ر�SPI����
//  	return status;        //���ض�����״ֵ̬
//}

uint8_t CMT2300A_Burst_Write(uint8_t addr, uint8_t *pBuf, uint8_t len)
{
	uint8_t ctr;	    
 	GPIOB->BRR = GPIO_PIN_6;          //ʹ��SPI����

	for(ctr=0;ctr<len;ctr++)
	{
		SPI1_WriteByte_u16(( (addr + ctr) << 8) |  pBuf[ctr] );
	}

	GPIOB->BSRR = GPIO_PIN_6;       //�ر�SPI����
	return 1;          //���ض�����״ֵ̬
}

SPI_HandleTypeDef SPI1_Handler;  //SPI1���

//������SPIģ��ĳ�ʼ�����룬���ó�����ģʽ 						  
//SPI�ڳ�ʼ��
//�������Ƕ�SPI1�ĳ�ʼ��
void SPI1_Init(void)
{
    SPI1_Handler.Instance=SPI1;                         //SPI1
    SPI1_Handler.Init.Mode=SPI_MODE_MASTER;             //����SPI����ģʽ������Ϊ��ģʽ
    SPI1_Handler.Init.Direction=SPI_DIRECTION_2LINES;   //����SPI�������˫�������ģʽ:SPI����Ϊ˫��ģʽ
    SPI1_Handler.Init.DataSize=SPI_DATASIZE_16BIT;       //����SPI�����ݴ�С:SPI���ͽ���8λ֡�ṹ
    SPI1_Handler.Init.CLKPolarity=SPI_POLARITY_LOW;    //����ͬ��ʱ�ӵĿ���״̬Ϊ�ߵ�ƽ
    SPI1_Handler.Init.CLKPhase=SPI_PHASE_1EDGE;         //����ͬ��ʱ�ӵĵڶ��������أ��������½������ݱ�����
    SPI1_Handler.Init.NSS=SPI_NSS_SOFT;                 //NSS�ź���Ӳ����NSS�ܽţ����������ʹ��SSIλ������:�ڲ�NSS�ź���SSIλ����
    SPI1_Handler.Init.BaudRatePrescaler=SPI_BAUDRATEPRESCALER_2;//���岨����Ԥ��Ƶ��ֵ:������Ԥ��ƵֵΪ256
    SPI1_Handler.Init.FirstBit=SPI_FIRSTBIT_MSB;        //ָ�����ݴ����MSBλ����LSBλ��ʼ:���ݴ����MSBλ��ʼ
    SPI1_Handler.Init.TIMode=SPI_TIMODE_DISABLE;        //�ر�TIģʽ
    SPI1_Handler.Init.CRCCalculation=SPI_CRCCALCULATION_DISABLE;//�ر�Ӳ��CRCУ��
    SPI1_Handler.Init.CRCPolynomial=7;                  //CRCֵ����Ķ���ʽ
    HAL_SPI_Init(&SPI1_Handler);//��ʼ��
    
    __HAL_SPI_ENABLE(&SPI1_Handler);                    //ʹ��SPI1
		SET_BIT(SPI1->CR2, SPI_RXFIFO_THRESHOLD);
//    SPI1_ReadWriteByte(0Xff);                           //��������
}

//SPI5�ײ�������ʱ��ʹ�ܣ���������
//�˺����ᱻHAL_SPI_Init()����
//hspi:SPI���
void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi)
{
    GPIO_InitTypeDef GPIO_Initure;
    
    __HAL_RCC_GPIOB_CLK_ENABLE();       //ʹ��GPIOBʱ��
		__HAL_RCC_GPIOA_CLK_ENABLE();       //ʹ��GPIOBʱ��
    __HAL_RCC_SPI1_CLK_ENABLE();        //ʹ��SPI1ʱ��
    
    //PA5.6.7
    GPIO_Initure.Pin=GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;
    GPIO_Initure.Mode=GPIO_MODE_AF_PP;              //�����������
    GPIO_Initure.Pull=GPIO_NOPULL;                  //����
    GPIO_Initure.Speed=GPIO_SPEED_FREQ_VERY_HIGH;             //����            
    GPIO_Initure.Alternate=GPIO_AF5_SPI1;           //����ΪSPI1
    HAL_GPIO_Init(GPIOA,&GPIO_Initure);
	
		GPIO_Initure.Pin=GPIO_PIN_6;
		GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;
		HAL_GPIO_Init(GPIOB,&GPIO_Initure);
	
		HAL_GPIO_WritePin(GPIOA,GPIO_PIN_6,GPIO_PIN_SET);
}


//SPI�ٶ����ú���
//SPI�ٶ�=fAPB1/��Ƶϵ��
//@ref SPI_BaudRate_Prescaler:SPI_BAUDRATEPRESCALER_2~SPI_BAUDRATEPRESCALER_2 256
//fAPB1ʱ��һ��Ϊ42Mhz��
void SPI1_SetSpeed(u8 SPI_BaudRatePrescaler)
{
    assert_param(IS_SPI_BAUDRATE_PRESCALER(SPI_BaudRatePrescaler));//�ж���Ч��
    __HAL_SPI_DISABLE(&SPI1_Handler);            //�ر�SPI
    SPI1_Handler.Instance->CR1&=0XFFC7;          //λ3-5���㣬�������ò�����
    SPI1_Handler.Instance->CR1|=SPI_BaudRatePrescaler;//����SPI�ٶ�
    __HAL_SPI_ENABLE(&SPI1_Handler);             //ʹ��SPI
    
}

//SPI1 ��дһ���ֽ�
//TxData:Ҫд����ֽ�
//����ֵ:��ȡ�����ֽ�
u8 SPI1_ReadWriteByte(u8 TxData)
{
    u8 Rxdata;
    HAL_SPI_TransmitReceive(&SPI1_Handler,&TxData,&Rxdata,1, 1000);       
 	return Rxdata;          		    //�����յ�������		
}




