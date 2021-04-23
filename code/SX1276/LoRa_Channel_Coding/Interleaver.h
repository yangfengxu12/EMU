#ifndef __INTERLEAVER_H__
#define __INTERLEAVER_H__

#include "stm32l4xx.h"
#include <stdbool.h>

long mod(long a, long b);
uint16_t *Interleaver(uint8_t cr, uint8_t sf, bool low_data_rate_optimize, uint8_t *input, uint16_t ninput_items, uint16_t *noutput_items);

#endif
