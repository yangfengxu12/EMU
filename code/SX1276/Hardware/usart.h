#ifndef _USART_H
#define _USART_H
#include "sys.h"
#include "stdio.h"	
	
#define USART_REC_LEN  			300  	//�����������ֽ��� 256
#define EN_USART2_RX 			1		//ʹ�ܣ�1��/��ֹ��0������1����
	  	
extern uint8_t  USART_RX_BUF[USART_REC_LEN]; //���ջ���,���USART_REC_LEN���ֽ�.ĩ�ֽ�Ϊ���з� 
extern uint16_t USART_RX_STA;         		//����״̬���	
extern UART_HandleTypeDef UART2_Handler; //UART���

#define RXBUFFERSIZE   1 //�����С
extern uint8_t aRxBuffer[RXBUFFERSIZE];//HAL��USART����Buffer

//����봮���жϽ��գ��벻Ҫע�����º궨��
void uart_init(uint32_t bound);


#endif
