#ifndef _TIMER_H
#define _TIMER_H
#include "stm32l4xx.h"
#include "control_GPIO.h"
typedef uint32_t  u32;
typedef uint16_t u16;
typedef uint8_t  u8;


extern TIM_HandleTypeDef TIM2_Handler;
extern uint32_t time_count;
extern uint32_t Time;
extern _Bool Chip_flag;

void TIM2_Init(u32 arr,u16 psc);

uint32_t LL_TIM_IsActiveFlag_UPDATE(TIM_TypeDef *TIMx);
void LL_TIM_ClearFlag_UPDATE(TIM_TypeDef *TIMx);

void TIM15_Init(void);
#endif

