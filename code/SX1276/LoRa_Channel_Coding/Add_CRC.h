#ifndef __ADD_CRC_H__
#define __ADD_CRC_H__

#include "stm32l4xx.h"
#include <stdbool.h>

unsigned int crc16(unsigned int crcValue, unsigned char newByte);
uint8_t *Add_CRC(bool has_crc, char *input_str, uint8_t *input, uint8_t ninput_items, uint8_t *noutput_items);

#endif
