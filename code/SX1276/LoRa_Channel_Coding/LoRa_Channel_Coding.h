#ifndef __LORA_CHANNEL_CODING__H__
#define	__LORA_CHANNEL_CODING__H__

#include "Whitening.h"
#include "Header.h"
#include "Add_CRC.h"
#include "Hanmming_Enc.h"
#include "Interleaver.h"
#include "Gray_decode.h"
#include "Modulation.h"


int *LoRa_Channel_Coding(uint8_t *tx_buffer, uint16_t buffer_size, uint32_t bw, uint8_t sf, uint8_t cr, bool has_crc, bool impl_head, int *symbol_len, bool low_data_rate_optimize);

#endif
