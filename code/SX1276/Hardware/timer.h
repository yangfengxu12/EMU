#ifndef _TIMER_H
#define _TIMER_H
#include "stm32l4xx.h"

typedef uint32_t  u32;
typedef uint16_t u16;
typedef uint8_t  u8;


extern TIM_HandleTypeDef TIM2_Handler;
extern uint32_t time_count;
extern uint32_t Time_temp;

void TIM2_Init(u32 arr,u16 psc);
#endif

