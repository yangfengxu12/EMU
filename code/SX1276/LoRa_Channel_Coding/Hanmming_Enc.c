#include "Hanmming_Enc.h"
#include "string.h"
#include <stdlib.h>

uint8_t *Hanmming_Enc(uint8_t cr, uint8_t sf, char *input_str, uint8_t *input, uint8_t ninput_items, uint8_t *noutput_items)
{
	bool p0,p1,p2,p3,p4;
	int i;
	uint8_t cr_app;
	uint8_t data_bin;
	uint8_t *output= malloc(ninput_items);
	*noutput_items = ninput_items;
	for(i = 0; i < ninput_items; i++)
	{
			cr_app=(i<sf-2)?4:cr;
			data_bin = input[i];

			//the data_bin is msb first
			if(cr_app != 1){ //need hamming parity bits
					p0=(data_bin&0x01)      ^ ((data_bin&0x02)>>1) ^ ((data_bin&0x04)>>2);
					p1=((data_bin&0x02)>>1) ^ ((data_bin&0x04)>>2) ^ ((data_bin&0x08)>>3);
					p2=(data_bin&0x01) 			^ ((data_bin&0x02)>>1) ^ ((data_bin&0x08)>>3);
					p3=(data_bin&0x01) 			^ ((data_bin&0x04)>>2) ^ ((data_bin&0x08)>>3);
					//we put the data LSB first and append the parity bits
					output[i]=((data_bin&0x01)<<7|((data_bin&0x02)>>1)<<6|((data_bin&0x04)>>2)<<5|((data_bin&0x08)>>3)<<4|p0<<3|p1<<2|p2<<1|p3)>>(4-cr_app);
			}
			else{// coding rate = 4/5 we add a parity bit
					p4=(data_bin&0x08) ^ ((data_bin&0x04)>>1) ^ ((data_bin&0x02)>>2) ^ ((data_bin&0x01)>>3);
					output[i]=((data_bin&0x08)|(data_bin&0x04)|(data_bin&0x02)|(data_bin&0x01)|p4);
			}
	}
	return output;
}

