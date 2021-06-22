#ifndef _CONTROL_H__
#define _CONTROL_H__

#include "stm32l4xx.h"


void Control_GPIO_Init( void );

void LL_GPIO_TogglePin(GPIO_TypeDef *GPIOx, uint32_t PinMask);
void LL_GPIO_ResetOutputPin(GPIO_TypeDef *GPIOx, uint32_t PinMask);
void LL_GPIO_SetOutputPin(GPIO_TypeDef *GPIOx, uint32_t PinMask);
#endif
