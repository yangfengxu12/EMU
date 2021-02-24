#ifndef __MODULATION_H__
#define __MODULATION_H__

#include "stm32l4xx.h"

int *Modulation(uint8_t cr, uint8_t sf, uint32_t bw, uint16_t *input, uint8_t ninput_items, uint8_t *noutput_items);

#endif
