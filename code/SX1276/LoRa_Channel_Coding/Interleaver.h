#ifndef __INTERLEAVER_H__
#define __INTERLEAVER_H__

#include "stm32l4xx.h"
#include <stdbool.h>

long mod(long a, long b);
uint16_t *Interleaver(uint8_t cr, uint8_t sf, char *input_str, uint8_t *input, uint8_t ninput_items, uint8_t *noutput_items);

#endif
