#ifndef __GRAY_DECODE_H__
#define __GRAY_DECODE_H__

#include "stm32l4xx.h"
#include <stdbool.h>

long mod(long a, long b);
uint16_t *Gray_Decoder(uint8_t cr, uint8_t sf, bool low_data_rate_optimize, uint16_t *input, uint16_t ninput_items, uint16_t *noutput_items);

#endif
