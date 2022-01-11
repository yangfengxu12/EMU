#ifndef __MODULATION_H__
#define __MODULATION_H__

#include "stm32l4xx.h"
#include <stdbool.h>
int *Modulation(bool pakcet_type, uint8_t cr, uint8_t sf, uint32_t bw, uint16_t *input, uint16_t ninput_items, uint16_t *noutput_items);

#endif
