#ifndef __HEADER__H__
#define	__HEADER__H__

#include "stm32l4xx.h"
#include <stdbool.h>

uint8_t* Add_Header(bool impl_head, bool has_crc, uint8_t cr, uint16_t buffer_size, uint8_t *in, uint16_t ninput_items, uint16_t *noutput_items);

#endif
