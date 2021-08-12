#ifndef __CMT_SPI3_H
#define __CMT_SPI3_H

//#include "typedefs.h"
#include <stdint.h>

__inline void cmt_spi3_delay(void);
__inline void cmt_spi3_delay_us(void);

void cmt_spi3_init(void);

void cmt_spi3_send(uint8_t data8);
uint8_t cmt_spi3_recv(void);

void cmt_spi3_write(uint8_t addr, uint8_t dat);
void cmt_spi3_read(uint8_t addr, uint8_t* p_dat);

void cmt_spi3_write_fifo(const uint8_t* p_buf, uint16_t len);
void cmt_spi3_read_fifo(uint8_t* p_buf, uint16_t len);

#endif
