#ifndef _TIMER_H
#define _TIMER_H
#include "stm32l4xx.h"
#include "control_GPIO.h"
#include "stm32l4xx_ll_tim.h"
#include "stm32l4xx_ll_gpio.h"
#include "stm32l4xx_ll_bus.h"
typedef uint32_t  u32;
typedef uint16_t u16;
typedef uint8_t  u8;

#define Calibration_Times 3

extern TIM_HandleTypeDef TIM2_Handler;
extern TIM_HandleTypeDef TIM3_Handler;

extern uint32_t Time;
extern u8 Timer_Calibration_Done_Flag;
extern int Input_Captured_Record[Calibration_Times][4];
extern int Timer_Compensation_Count;

void TIM2_Init(u32 arr,u16 psc);
void TIM3_Init(u32 arr);

uint32_t LL_TIM_IsActiveFlag_UPDATE(TIM_TypeDef *TIMx);
void LL_TIM_ClearFlag_UPDATE(TIM_TypeDef *TIMx);

void TIM15_Init(void);

void MX_TIM3_Init(void);
void MX_TIM4_Init(void);
#endif

