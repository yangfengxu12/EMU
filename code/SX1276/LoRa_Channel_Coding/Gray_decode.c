#include "Gray_decode.h"
#include "string.h"
#include <stdlib.h>

//#define DEBUG

#ifdef DEBUG
#include "usart.h"
#endif

inline long mod(long a, long b)
{ 
	long result_mod;
	
	result_mod = (a%b+b)%b;
	
	return result_mod;
}

uint8_t *Gray_Decoder(uint8_t cr, uint8_t sf, char *input_str, uint8_t *input, uint8_t ninput_items, uint8_t *noutput_items)
{
	uint8_t *output;
	
	output = calloc(ninput_items,sizeof(uint8_t));
	
	for(int i=0;i < ninput_items;i++)
	{
		output[i]=input[i];
		for(int j=1;j<sf;j++)
		{
			output[i]=output[i]^(input[i]>>j);
		}
		//do the shift of 1
		output[i]=mod(output[i]+1,(1<<sf));
	}
	
	*noutput_items = ninput_items;
	return output;
}

