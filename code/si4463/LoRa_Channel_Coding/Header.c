#include "Header.h"
#include "string.h"
#include <stdlib.h>

#define DEBUG

//impl_head : True or Flase
//has_crc : True or Flase
//cr : compress rate 
//input_str: payload string
//in: Whitened data
//outputput: Header + whitened data
uint8_t* Add_Header(bool impl_head, bool has_crc, uint8_t cr, uint16_t buffer_size, uint8_t *in, uint16_t ninput_items, uint16_t *noutput_items)
{
	bool c0,c1,c2,c3,c4;
	
	uint8_t *output= malloc((ninput_items+5)*sizeof(uint8_t));
	
	if(impl_head)
	{//no header to add
		memcpy(output,in,ninput_items*sizeof(uint8_t));
		
		*noutput_items = ninput_items;
	}
	else
	{//add header
		//payload length
		output[0]=(buffer_size>>4);
		output[1]=(buffer_size&0x0F);

		//coding rate and has_crc
		output[2]=((cr<<1)|has_crc);

		//header checksum
		c4=(output[0] & 0x8)>>3 ^ (output[0] & 0x4)>>2 ^ (output[0] & 0x2)>>1 ^ (output[0] & 0x1);
		c3=(output[0] & 0x8)>>3 ^ (output[1] & 0x8)>>3 ^ (output[1] & 0x4)>>2 ^ (output[1] & 0x2)>>1 ^ (output[2] & 0x1);
		c2=(output[0] & 0x4)>>2 ^ (output[1] & 0x8)>>3 ^ (output[1] & 0x1)    ^ (output[2] & 0x8)>>3 ^ (output[2] & 0x2)>>1;
		c1=(output[0] & 0x2)>>1 ^ (output[1] & 0x4)>>2 ^ (output[1] & 0x1)    ^ (output[2] & 0x4)>>2 ^ (output[2] & 0x2)>>1 ^ (output[2] & 0x1);
		c0=(output[0] & 0x1)    ^ (output[1] & 0x2)>>1 ^ (output[2] & 0x8)>>3 ^ (output[2] & 0x4)>>2 ^ (output[2] & 0x2)>>1 ^ (output[2] & 0x1);

		output[3]=c4;
		output[4]=c3<<3|c2<<2|c1<<1|c0;
		
		memcpy(&output[5],in,ninput_items*sizeof(uint8_t));
		
		*noutput_items = ninput_items+5;
	}
	
	return output;
}

