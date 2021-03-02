#ifndef __HANMMING_ENC_H__
#define __HANMMING_ENC_H__

#include "stm32l4xx.h"
#include <stdbool.h>

uint8_t *Hanmming_Enc(uint8_t cr, uint8_t sf, uint8_t *input, uint16_t ninput_items, uint16_t *noutput_items);

#endif
