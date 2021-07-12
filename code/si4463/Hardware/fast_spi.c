#include "fast_spi.h"
//������SPIģ��ĳ�ʼ�����룬���ó�����ģʽ 						  
//SPI�ڳ�ʼ��
//�������Ƕ�SPI1�ĳ�ʼ��
void SPI1_Init(void)
{	 
	uint16_t tempreg=0;
	RCC->AHB1ENR|=1<<0;    	//ʹ��PORTAʱ��	   
	RCC->APB2ENR|=1<<12;   	//SPI1ʱ��ʹ�� 
	GPIO_Set(GPIOA,7<<5,GPIO_MODE_AF,GPIO_OTYPE_PP,GPIO_SPEED_100M,GPIO_PUPD_PU);	//PA5~7���ù������	
//  	GPIO_AF_Set(GPIOB,3,5);	//PB3,AF5
// 	GPIO_AF_Set(GPIOB,4,5);	//PB4,AF5
// 	GPIO_AF_Set(GPIOB,5,5);	//PB5,AF5 

	//����ֻ���SPI�ڳ�ʼ��
//	RCC->APB2RSTR|=1<<12;	//��λSPI1
//	RCC->APB2RSTR&=~(1<<12);//ֹͣ��λSPI1
	tempreg|=0<<10;			//ȫ˫��ģʽ	
	tempreg|=1<<9;			//���nss����
	tempreg|=1<<8;			 
	tempreg|=1<<2;			//SPI����  

	tempreg|=0<<1;			// CPOL=0
	tempreg|=0<<0;			//CPHA=0
 	//��SPI1����APB2������.ʱ��Ƶ�����Ϊ80MhzƵ��.
	tempreg|=5<<3;			//Fsck=Fpclk1/8 
	tempreg|=0<<7;			//MSB First
	tempreg|=1<<6;			//SPI����
	
	SPI1->CR2 = 1<<12;  //FIFO reception threshold 8bit
	SPI1->CR2 = 7<<8;		//8λ���ݸ�ʽ	
	
	
	SPI1->CR1=tempreg; 		//����CR1
//	SPI1->I2SCFGR&=~(1<<11);//ѡ��SPIģʽ

	
	SPI1->CR1&=~(1<<6); 	//SPI�豸ʧ��
	SPI1->CR1&=~(1<<1); 	//����ģʽ��SCKΪ0 CPOL=0
	SPI1->CR1&=~(1<<0); 	//���ݲ����ӵ�1��ʱ����ؿ�ʼ,CPHA=0  
	SPI1->CR1|=1<<6; 		//SPI�豸ʹ�� 
		 
}   
//SPI1�ٶ����ú���
//SpeedSet:0~7
//SPI�ٶ�=fAPB2/2^(SpeedSet+1)
//fAPB2ʱ��һ��Ϊ84Mhz
void SPI1_SetSpeed(uint8_t SpeedSet)
{
	SpeedSet&=0X07;			//���Ʒ�Χ
	SPI1->CR1&=0XFFC7; 
	SPI1->CR1|=SpeedSet<<3;	//����SPI1�ٶ�  
	SPI1->CR1|=1<<6; 		//SPI�豸ʹ��	  
} 


////SPI1 ��һ���ֽ�
////TxData:Ҫд����ֽ�
////����ֵ:��ȡ�����ֽ�
//uint8_t SPI1_ReadByte(uint8_t addr)
//{		 			 
//	uint16_t reg;
//	GPIOB->BRR = GPIO_PIN_6;
//	while((SPI1->SR&1<<1)==0);		//�ȴ��������� 
//	SPI1->DR=(uint16_t )addr;	 	  		//����һ��byte  
//	while((SPI1->SR&1<<0)==0);		//�ȴ�������һ��byte
//	reg = SPI1->DR;
//	while((SPI1->SR&1<<0)==0);
//	reg = SPI1->DR;
//	GPIOB->BSRR = GPIO_PIN_6;
// 	return reg;        		//�����յ�������		
//}

uint8_t SPI1_ReadByte(uint8_t TxData)
{		 			 
	while((SPI1->SR&1<<1)==0);		//�ȴ��������� 	
	*(__IO uint8_t *)&SPI1->DR = TxData;	//����һ��byte 
	while((SPI1->SR&1<<0)==0);		//�ȴ�������һ��byte  
 	return SPI1->DR;          		//�����յ�������				    
}

uint8_t SPI1_WriteByte_u8(uint8_t TxData)
{		 			 
	while((SPI1->SR&1<<1)==0);		//�ȴ��������� 	
	*(__IO uint8_t *)&SPI1->DR = TxData;	//����һ��byte 
	while((SPI1->SR&1<<0)==0);		//�ȴ�������һ��byte  
 	return SPI1->DR;          		//�����յ�������				    
}

uint8_t SPI1_WriteByte_u16(uint16_t TxData)
{		 			 
	uint8_t value;
//	while(!(SPI1->SR&1<<1));		//�ȴ��������� 	
	SPI1->DR = TxData;	//����һ��byte 
	while(!(SPI1->SR&1<<0));		//�ȴ�������һ��byte
  value = SPI1->DR;
	while(!(SPI1->SR&1<<0));
 	return SPI1->DR;          		//�����յ�������				    
}



uint8_t SX1276_Burst_Read(uint8_t reg,uint8_t *pBuf,uint8_t len)
{
	uint8_t status,uint8_t_ctr;	       
  GPIOB->BRR = GPIO_PIN_6;           //ʹ��SPI����
  status=SPI1_ReadByte(reg);//���ͼĴ���ֵ(λ��),����ȡ״ֵ̬   	   
 	for(uint8_t_ctr=0;uint8_t_ctr<len;uint8_t_ctr++)
		pBuf[uint8_t_ctr]=SPI1_ReadByte(0XFF);//��������
  GPIOB->BSRR = GPIO_PIN_6;       //�ر�SPI����
  return status;        //���ض�����״ֵ̬
}

uint8_t SX1276_Burst_Write(uint8_t reg, uint8_t *pBuf, uint8_t len)
{
	uint8_t uint8_t_ctr;	    
 	GPIOB->BRR = GPIO_PIN_6;          //ʹ��SPI����

	if( len & 1 ) // len = odd  
	{
		SPI1_WriteByte_u16(( pBuf[0] << 8 ) | ( reg | 0x80 ));
		if( len > 1 ) // len = 3,5,7.....
		{
			for(uint8_t_ctr = 0; uint8_t_ctr < (( len - 1 ) >> 1 ); uint8_t_ctr++ )
				SPI1_WriteByte_u16(( pBuf[ uint8_t_ctr + 2 ] << 8 ) | pBuf[ uint8_t_ctr + 1 ]); //д������	 
		}
	}
	else	// len = even
	{
		SPI1_WriteByte_u8( reg );
		if( len >= 1 )	// len = 2,4,6...
		{
			for(uint8_t_ctr = 0; uint8_t_ctr < ( len >> 1 ); uint8_t_ctr++ )
				SPI1_WriteByte_u16(( pBuf[ uint8_t_ctr + 1 ] << 8 ) | pBuf[ uint8_t_ctr ]); //д������	 
		}
	}
	GPIOB->BSRR = GPIO_PIN_6;       //�ر�SPI����
	return 1;          //���ض�����״ֵ̬
}







