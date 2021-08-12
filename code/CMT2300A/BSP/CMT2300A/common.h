#ifndef __COMMON_H
#define __COMMON_H

//#include "typedefs.h"
#include <stdint.h>
#include <stdbool.h>
#include "gpio_defs.h"


void no_optimize(const void* p_param);
void Common_Init(void);

void GPIO_Config(void);
__inline void GPIO_Pin_Setting(GPIO_TypeDef *gpio, uint16_t nPin, uint32_t speed, uint32_t mode);

void set_uint16_t_to_buf(uint8_t buf[], uint16_t dat16);
uint16_t get_uint16_t_from_buf(const uint8_t buf[]);

void set_uint32_t_to_buf(uint8_t buf[], uint32_t dat32);
uint32_t get_uint32_t_from_buf(const uint8_t buf[]);

void views_print_line(uint8_t nLine, const char* str);

#endif
