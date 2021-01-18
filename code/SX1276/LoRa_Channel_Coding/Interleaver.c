#include "Interleaver.h"
#include "string.h"
#include <stdlib.h>



#define DEBUG

#ifdef DEBUG
#include "usart.h"
#endif

inline long mod(long a, long b)
{ 
	return (a%b+b)%b; 
}


uint8_t *Interleaver(uint8_t cr, uint8_t sf, char *input_str, uint8_t *input, uint8_t ninput_items, uint8_t *noutput_items)
{
	uint16_t cw_cnt = 0;
	uint8_t ppm = 0;
	uint8_t sf_app = 0;
	uint16_t remaind_cnt = ninput_items - sf_app;
	uint16_t i = 0, j=0,k=0,sum=0;
	
	uint8_t *input_1st;
	uint8_t *output_1st;
	
	while(remaind_cnt!=0)
	{
		ppm = 4+((cw_cnt<sf-2)?4:cr);
		sf_app = (cw_cnt<sf-2)?sf-2:sf;
		
		input_1st = malloc(sf_app*sizeof(uint8_t));
		output_1st = malloc(sf_app*sizeof(uint8_t));
		
		memcpy(input_1st,input,sf_app*sizeof(uint8_t));
		
		for (int32_t i = 0; i < ppm ; i++) 
    {
			for (int32_t j = 0; j < sf_app; j++) 
			{
				output_1st[i] |= (input_1st[mod((i-j-1),sf_app)]&(1<<i));
			}
			if(cw_cnt == sf-2)
			{
				
			}

		remaind_cnt = ninput_items - sf_app;
	}
	
	
	
	return 0;
}


