#ifndef __GPIO_DEFS_H
#define __GPIO_DEFS_H

#include "stm32l4xx.h"

#define CMT_CSB_GPIO                GPIOB
#define CMT_CSB_GPIO_PIN            GPIO_PIN_6

#define CMT_FCSB_GPIO               GPIOA
#define CMT_FCSB_GPIO_PIN           GPIO_PIN_10

#define CMT_SCLK_GPIO               GPIOA
#define CMT_SCLK_GPIO_PIN           GPIO_PIN_5

#define CMT_SDIO_GPIO               GPIOA
#define CMT_SDIO_GPIO_PIN           GPIO_PIN_7

#define CMT_GPIO1_GPIO              GPIOB
#define CMT_GPIO1_GPIO_PIN          GPIO_PIN_3

#define CMT_GPIO2_GPIO              GPIOB
#define CMT_GPIO2_GPIO_PIN          GPIO_PIN_5

#define CMT_GPIO3_GPIO              GPIOB
#define CMT_GPIO3_GPIO_PIN          GPIO_PIN_4

#define CMT_GPIO4_GPIO              GPIOA
#define CMT_GPIO4_GPIO_PIN          GPIO_PIN_9


#define SET_GPIO_OUT(x)             GPIO_Pin_Setting(x, x##_PIN, GPIO_SPEED_FREQ_VERY_HIGH, GPIO_MODE_OUTPUT_PP)
#define SET_GPIO_IN(x)              GPIO_Pin_Setting(x, x##_PIN, GPIO_SPEED_FREQ_VERY_HIGH, GPIO_MODE_INPUT)
#define SET_GPIO_OD(x)              GPIO_Pin_Setting(x, x##_PIN, GPIO_SPEED_FREQ_VERY_HIGH, GPIO_MODE_OUTPUT_OD)
#define SET_GPIO_AIN(x)             GPIO_Pin_Setting(x, x##_PIN, GPIO_SPEED_FREQ_VERY_HIGH, GPIO_MODE_ANALOG)
#define SET_GPIO_AFOUT(x)           GPIO_Pin_Setting(x, x##_PIN, GPIO_SPEED_FREQ_VERY_HIGH, GPIO_MODE_AF_PP)
#define SET_GPIO_AFOD(x)            GPIO_Pin_Setting(x, x##_PIN, GPIO_SPEED_FREQ_VERY_HIGH, GPIO_MODE_AF_OD)
#define SET_GPIO_H(x)               (x->BSRR = x##_PIN) //GPIO_SetBits(x, x##_PIN)
#define SET_GPIO_L(x)               (x->BRR  = x##_PIN) //GPIO_ResetBits(x, x##_PIN)
#define READ_GPIO_PIN(x)            (((x->IDR & x##_PIN)!= GPIO_PIN_RESET) ?1 :0) //GPIO_ReadInputDataBit(x, x##_PIN) 

#endif
