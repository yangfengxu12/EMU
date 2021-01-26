#include "Modulation.h"
#include "string.h"
#include <stdlib.h>
//#include "malloc.h" 

#define DEBUG

#ifdef DEBUG
#include "usart.h"
#endif



int *Modulation(uint8_t cr, uint8_t sf, uint32_t bw, uint8_t *input, uint8_t ninput_items, uint8_t *noutput_items)
{
	
	int id1 = 8;
	int id2 = 16;
	int *output;
	
//	uint8_t *in = malloc(ninput_items*sizeof(uint8_t));
	
//	mymemcpy(in,input,ninput_items);
	
	output = malloc((ninput_items+2)*sizeof(int));
	
	if(output == NULL)
	{
		printf("\n memery is not align!!\n");
	}
	
	output[0] = (int) ((double)bw/((double)(1<<sf))*(double)id1 - 62500);
	output[1] = (int) ((double)bw/((double)(1<<sf))*(double)id2 - 62500);
	
	for(int i=2;i<ninput_items+2;i++)
	{
		output[i] = (int)(((double)bw)/((double)(1<<sf))*((double)(input[i-2])) - 62500);
	}
	
	*noutput_items = ninput_items + 2;
	
//	free(output);
	
	return output;
}


