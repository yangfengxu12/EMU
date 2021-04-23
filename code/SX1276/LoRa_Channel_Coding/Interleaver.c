#include "Interleaver.h"
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


uint16_t *Interleaver(uint8_t cr, uint8_t sf, bool low_data_rate_optimize, uint8_t *input, uint16_t ninput_items, uint16_t *noutput_items)
{
	uint16_t cw_cnt = 0;
	uint8_t ppm = 0;
	uint8_t sf_app = 0;
	uint16_t remaind_cnt = ninput_items;
	static uint16_t i = 0, j=0,k=0;
	uint16_t sum_interleaved;
	
	uint8_t *codewords = NULL;
	uint16_t *interleaved = NULL;
	uint16_t *output = NULL;
	
	uint32_t cnt_interleaved=0;
	
	uint16_t mem_cnt =0;
	
	
	while(remaind_cnt!=0)
	{
		
		ppm = 4+((cw_cnt<sf-2)?4:cr);
		
		if(low_data_rate_optimize)
		{
			sf_app = sf-2;
		}
		else
		{
			sf_app = (cw_cnt<sf-2)?sf-2:sf;
		}
		
//		printf("ppm,CR%d\n",ppm);
//		printf("sf_app%d\n",sf_app);
		
		codewords = realloc(codewords,sf_app*sizeof(uint8_t));
		memset(codewords, 0, sf_app*sizeof(uint8_t)); 	

		if(cw_cnt + sf_app < ninput_items)
		{
			memcpy(codewords,input+cw_cnt,sf_app*sizeof(uint8_t));
		}
		else
		{
			memcpy(codewords,input+cw_cnt,(ninput_items-cw_cnt)*sizeof(uint8_t));
		}
		
		interleaved = realloc(interleaved,ppm*sizeof(uint16_t));
		memset(interleaved, 0, ppm*sizeof(uint16_t));
		
		for(i=0;i<sf_app;i++)
		{
			cw_cnt++;
		}
		
		
		for (i = 0; i < ppm ; i++) 
    {
			for (j = 0; j < sf_app; j++) 
			{
				interleaved[i] |= (((codewords[mod((i-j-1),sf_app)]&(1<<(ppm-i-1))) >> (ppm-i-1)) << (sf-j-1));
//				printf("i=%d,j=%d,codewords[%d]:0x%x,bits:%d,interleaved[%d]:0x%x\n",i,j,(int)mod((i-j-1),sf_app),codewords[mod((i-j-1),sf_app)],ppm-i-1,i,interleaved[i]);
			}
			
			if(cw_cnt == sf-2)
			{
				sum_interleaved = 0;
				for(k=0;k<sf;k++)
				{
					sum_interleaved += (interleaved[i] >> k) & 1;
				}

				interleaved[i] |= (sum_interleaved%2)<<(sf - sf_app - 1);
			}
		}
		
		cnt_interleaved += ppm;
		
		
		
		output = realloc(output,cnt_interleaved*sizeof(uint16_t));
		
//		memset(output, 0, cnt_interleaved*sizeof(uint8_t));
		
		memcpy(output + cnt_interleaved - ppm, interleaved, ppm * sizeof(uint16_t));
		
		remaind_cnt -= remaind_cnt > sf_app ? sf_app : remaind_cnt;
	}
	
	*noutput_items = cnt_interleaved;
	#ifdef DEBUG
	
	printf("------interleaved------\n");
	for(i=0;i<cnt_interleaved;i++)
			printf("%d (dec)----%x (hex)\n",output[i],output[i]);
	
	#endif
	
	free(codewords);
	free(interleaved);
	
	return output;
}


