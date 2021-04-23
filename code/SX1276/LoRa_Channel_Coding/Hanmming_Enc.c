#include "Hanmming_Enc.h"
#include "string.h"
#include <stdlib.h>

uint8_t *Hanmming_Enc(uint8_t cr, uint8_t sf, uint8_t *input, uint16_t ninput_items, uint16_t *noutput_items)
{
	bool p0 = false,p1 = false,p2 = false,p3 = false,p4 = false;
	int i;
	uint8_t cr_app=0;
	uint8_t data_bin=0;
	uint8_t *output= malloc(ninput_items*sizeof(uint8_t));
	*noutput_items = ninput_items;
	for(i = 0; i < ninput_items; i++)
	{
			cr_app=(i<sf-2)?4:cr; // header part
			data_bin = input[i];

			
			if(cr_app != 1){ //need hamming parity bits
					p0=(data_bin&0x01)      ^ ((data_bin&0x02)>>1) ^ ((data_bin&0x04)>>2);
					p1=((data_bin&0x02)>>1) ^ ((data_bin&0x04)>>2) ^ ((data_bin&0x08)>>3);
					p2=(data_bin&0x01) 			^ ((data_bin&0x02)>>1) ^ ((data_bin&0x08)>>3);
					p3=(data_bin&0x01) 			^ ((data_bin&0x04)>>2) ^ ((data_bin&0x08)>>3);
					//we put the data LSB first and append the parity bits
					output[i]=((data_bin&0x01)<<7|((data_bin&0x02)>>1)<<6|((data_bin&0x04)>>2)<<5|((data_bin&0x08)>>3)<<4|p0<<3|p1<<2|p2<<1|p3)>>(4-cr_app);
			}
			else{// coding rate = 4/5 we add a parity bit
					p4=((data_bin&0x08)>>3) ^ ((data_bin&0x04)>>2) ^ ((data_bin&0x02)>>1) ^ ((data_bin&0x01));
					output[i]=( ((data_bin&0x01)<<4) | (((data_bin&0x02)>>1)<<3) | (((data_bin&0x04)>>2)<<2) | (((data_bin&0x08)>>3)<<1) | p4 );
			}
	}
	
	return output;
}

