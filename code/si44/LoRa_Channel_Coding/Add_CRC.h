#ifndef __ADD_CRC_H__
#define __ADD_CRC_H__

#include "stm32l4xx.h"
#include <stdbool.h>

unsigned int crc16(unsigned int crcValue, unsigned char newByte);
uint8_t *Add_CRC(bool has_crc, uint8_t *tx_buffer, uint16_t buffer_size, uint8_t *input, uint16_t ninput_items, uint16_t *noutput_items);

#endif
