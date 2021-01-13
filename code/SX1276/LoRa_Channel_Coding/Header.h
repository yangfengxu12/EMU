#ifndef __HEADER__H__
#define	__HEADER__H__

#include "stm32l4xx.h"
#include <stdbool.h>

uint8_t* Add_Header(bool impl_head, bool has_crc, uint8_t cr, char *input_str, uint8_t *in);

#endif
