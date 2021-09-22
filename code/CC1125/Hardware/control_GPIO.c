#include "control_GPIO.h"

void Control_GPIO_Init( void )
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();

	GPIO_InitStruct.Pin   = GPIO_PIN_5 | GPIO_PIN_2 | GPIO_PIN_1;					//PB5
	GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull  = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

//	GPIO_InitStruct.Pin   = GPIO_PIN_1;
//	GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
//	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pin   = GPIO_PIN_0 | GPIO_PIN_10;				//PA0 RESET_N  PA10 GDIO0(Tx Data)
	GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_0,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_10,GPIO_PIN_SET);
}


void LL_GPIO_TogglePin(GPIO_TypeDef *GPIOx, uint32_t PinMask)
{
  WRITE_REG(GPIOx->ODR, READ_REG(GPIOx->ODR) ^ PinMask);
}

void LL_GPIO_ResetOutputPin(GPIO_TypeDef *GPIOx, uint32_t PinMask)
{
  WRITE_REG(GPIOx->BRR, PinMask);
}

void LL_GPIO_SetOutputPin(GPIO_TypeDef *GPIOx, uint32_t PinMask)
{
  WRITE_REG(GPIOx->BSRR, PinMask);
}
