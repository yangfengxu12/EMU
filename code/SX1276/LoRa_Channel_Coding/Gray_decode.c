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

uint16_t *Gray_Decoder(uint8_t cr, uint8_t sf, bool low_data_rate_optimize, uint16_t *input, uint16_t ninput_items, uint16_t *noutput_items)
{
	uint16_t *output;
	output = malloc(ninput_items*sizeof(uint16_t));
	
	#ifdef DEBUG
	for(int i=0;i<ninput_items;i++)
	{
		printf("Input[%d]:%x (hex)\n",i,input[i]);
	}
	#endif
	

	
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
	
	if(low_data_rate_optimize)
	{
		for(int i=8;i<ninput_items;i++)
		{
			if(output[i] % 2 == 0)
			{
				output[i] = mod(output[i]-3,(1<<sf));
			}
		}
	}
	
	
	
	*noutput_items = ninput_items;
	
	return output;
}

