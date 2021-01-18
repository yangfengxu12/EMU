#ifndef __HANMMING_ENC_H__
#define __HANMMING_ENC_H__

#include "stm32l4xx.h"
#include <stdbool.h>

uint8_t *Hanmming_Enc(uint8_t cr, uint8_t sf, char *input_str, uint8_t *input, uint8_t ninput_items, uint8_t *noutput_items);

#endif
