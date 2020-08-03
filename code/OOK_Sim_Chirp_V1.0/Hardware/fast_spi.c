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
	RCC->APB2RSTR|=1<<12;	//��λSPI1
	RCC->APB2RSTR&=~(1<<12);//ֹͣ��λSPI1
	tempreg|=0<<10;			//ȫ˫��ģʽ	
	tempreg|=1<<9;			//���nss����
	tempreg|=1<<8;			 
	tempreg|=1<<2;			//SPI����  
	tempreg|=0<<11;			//8λ���ݸ�ʽ	
	tempreg|=1<<1;			//����ģʽ��SCKΪ1 CPOL=1 
	tempreg|=1<<0;			//���ݲ����ӵ�2��ʱ����ؿ�ʼ,CPHA=1  
 	//��SPI1����APB2������.ʱ��Ƶ�����Ϊ80MhzƵ��.
	tempreg|=3<<3;			//Fsck=Fpclk1/16
	tempreg|=0<<7;			//MSB First  
	tempreg|=1<<6;			//SPI���� 
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


//SPI1 ��һ���ֽ�
//TxData:Ҫд����ֽ�
//����ֵ:��ȡ�����ֽ�
uint8_t SPI1_ReadByte(uint8_t addr)
{		 			 
	uint16_t reg;
	GPIOB->BRR = GPIO_PIN_6;
	while((SPI1->SR&1<<1)==0);		//�ȴ��������� 
	SPI1->DR=(uint16_t )addr;	 	  		//����һ��byte  
	while((SPI1->SR&1<<0)==0);		//�ȴ�������һ��byte
	reg = SPI1->DR;
	while((SPI1->SR&1<<0)==0);
	reg = SPI1->DR;
	GPIOB->BSRR = GPIO_PIN_6;
 	return reg;        		//�����յ�������		
}

void SPI1_WriteByte(uint16_t addr, uint16_t data)
{		 			 
	uint16_t reg;
	GPIOB->BRR = GPIO_PIN_6;
	while((SPI1->SR&1<<1)==0);		//�ȴ��������� 
	SPI1->DR=(uint16_t )( data<< 8 | (addr|0x80));	//����һ��byte  
	while((SPI1->SR&1<<0)==0);		//�ȴ�������һ��byte
	reg = SPI1->DR;
	while((SPI1->SR&1<<0)==0);
	reg = SPI1->DR;
	GPIOB->BSRR = GPIO_PIN_6;		
}









