#include "common.h"
#include <string.h>

#include "gpio_defs.h"


void no_optimize(const void* p_param)
{
}

void Common_Init(void)
{
}

void GPIO_Config(void)
{ 
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();

	GPIO_InitStruct.Pin   = CMT_CSB_GPIO_PIN;					
	GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull  = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(CMT_CSB_GPIO, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pin   = CMT_FCSB_GPIO_PIN;					
	GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull  = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(CMT_FCSB_GPIO, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pin   = CMT_SCLK_GPIO_PIN;					
	GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull  = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(CMT_SCLK_GPIO, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pin   = CMT_SDIO_GPIO_PIN;					
	GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull  = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(CMT_SDIO_GPIO, &GPIO_InitStruct);
	
	// GPIO 1 2 3
	
	GPIO_InitStruct.Pin   = CMT_GPIO1_GPIO_PIN;					
	GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull  = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(CMT_GPIO1_GPIO, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pin   = CMT_GPIO2_GPIO_PIN;					
	GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull  = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(CMT_GPIO2_GPIO, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pin   = CMT_GPIO3_GPIO_PIN;					
	GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull  = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(CMT_GPIO3_GPIO, &GPIO_InitStruct);
	

//	SET_GPIO_IN(CMT_GPIO1_GPIO);
//	SET_GPIO_IN(CMT_GPIO2_GPIO);
//	SET_GPIO_IN(CMT_GPIO3_GPIO);
//	SET_GPIO_IN(CMT_GPIO4_GPIO);
	
}

void GPIO_Pin_Setting(GPIO_TypeDef *gpio, uint16_t nPin, uint32_t speed, uint32_t mode)
{
//#if 0
//    GPIO_InitTypeDef GPIO_InitStructure;

//    GPIO_InitStructure.GPIO_Pin = nPin;
//    GPIO_InitStructure.GPIO_Speed = speed;
//    GPIO_InitStructure.GPIO_Mode = mode;
//    GPIO_Init(gpio, &GPIO_InitStructure);

	GPIO_InitTypeDef GPIO_InitStruct = {0};

	GPIO_InitStruct.Pin   = nPin;					
	GPIO_InitStruct.Mode  = mode;
	GPIO_InitStruct.Pull  = GPIO_NOPULL;
	GPIO_InitStruct.Speed = speed;
	HAL_GPIO_Init(gpio, &GPIO_InitStruct);

}

void set_uint16_t_to_buf(uint8_t buf[], uint16_t dat16)
{
    buf[0] = (uint8_t)dat16;
    buf[1] = (uint8_t)(dat16 >> 8);
}

uint16_t get_uint16_t_from_buf(const uint8_t buf[])
{
    uint16_t dat16 = 0;
    dat16  = buf[0];
    dat16 |= ((uint16_t)buf[1]) << 8;
    return dat16;
}

void set_uint32_t_to_buf(uint8_t buf[], uint32_t dat32)
{
    buf[0] = (uint8_t)dat32;
    buf[1] = (uint8_t)(dat32 >> 8);
    buf[2] = (uint8_t)(dat32 >> 16);
    buf[3] = (uint8_t)(dat32 >> 24);
}

uint32_t get_uint32_t_from_buf(const uint8_t buf[])
{
    uint32_t dat32 = 0;
    dat32  = buf[0];
    dat32 |= ((uint32_t)buf[1]) << 8;
    dat32 |= ((uint32_t)buf[2]) << 16;
    dat32 |= ((uint32_t)buf[3]) << 24;
    return dat32;
}

