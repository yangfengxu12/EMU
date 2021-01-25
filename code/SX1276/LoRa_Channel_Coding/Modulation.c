#include "Modulation.h"
#include "string.h"
#include <stdlib.h>

#define DEBUG

#ifdef DEBUG
#include "usart.h"
#endif

int *Modulation(uint8_t cr, uint8_t sf, uint32_t bw, uint8_t *input, uint8_t ninput_items, uint8_t *noutput_items)
{
	int *output;
	int id1 = 8;
	int id2 = 16;
	
	output = calloc(ninput_items+2,sizeof(int));
	
	output[0] = (int) ((float)bw/((float)(1<<sf))*(float)id1 - 62500.0);
	output[1] = (int) ((float)bw/((float)(1<<sf))*(float)id2 - 62500.0);
	
	for(int i=0;i<ninput_items+2;i++)
	{
		output[i+2] = (int)((float)bw/((float)(1<<sf))*(float)input[i] - 62500.0);
	}
	
	*noutput_items = ninput_items + 2;
	
	return output;
}


