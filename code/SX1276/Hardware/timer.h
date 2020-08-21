#ifndef _TIMER_H
#define _TIMER_H
#include "stm32l4xx.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F429������
//��ʱ����������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2016/1/6
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	
typedef uint32_t  u32;
typedef uint16_t u16;
typedef uint8_t  u8;

/* TIM handle declaration */
extern TIM_HandleTypeDef TIM1_Handler;

extern uint32_t time_count;
/* Prescaler declaration */
//uint32_t uwPrescalerValue = 0;

void TIM1_Init(u32 arr,u16 psc);
#endif

