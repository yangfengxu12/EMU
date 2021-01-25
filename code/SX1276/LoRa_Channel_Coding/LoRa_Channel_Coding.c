#include "LoRa_Channel_Coding.h"
#include <string.h>
#include <stdlib.h>

#define DEBUG

#ifdef DEBUG
#include "usart.h"
#endif

int *LoRa_Channel_Coding(char *str_tx, uint32_t bw, uint8_t sf, uint8_t cr, bool has_crc, bool impl_head, int *symbol_len)
{
	int i;
	uint8_t *whitened_data;
	uint8_t *add_header_data;
	uint8_t *add_CRC;
	uint8_t *hanmingcode_data;
	uint8_t *interleaver_data;
	uint8_t *gray_data;
	int 	*modulation_data;
	
	uint8_t noutput_add_CRC=0;
	uint8_t noutput_hanmming_coding = 0;
	uint8_t noutput_interleaver = 0;
	uint8_t noutput_gray = 0;
	uint8_t noutput_modulation = 0;
	
	
	#ifdef DEBUG
	printf("\n------------------Whitening-----------------------\n");
	#endif
	
	whitened_data = Whitening(str_tx);
	
	#ifdef DEBUG
	printf("Len of Output:%d\n",2*strlen(str_tx));
	for(i=0;i<2*strlen(str_tx);i++)
	{
		printf("Out[%d]:%x (hex)\n",i,whitened_data[i]);
	}
	printf("\n------------------Add header-----------------------\n");
	#endif
	
	add_header_data = Add_Header(impl_head, has_crc, cr, str_tx, whitened_data);
	
	#ifdef DEBUG
	printf("Len of Output:%d\n",2*strlen(str_tx)+5);
	for(i=0;i<2*strlen(str_tx)+5;i++)
	{
		printf("Out[%d]:%x (hex)\n",i,add_header_data[i]);
	}
	
	printf("\n------------------Add CRC-----------------------\n");
	#endif
	
	add_CRC = Add_CRC(has_crc, str_tx, add_header_data, 2*strlen(str_tx)+5, &noutput_add_CRC);
	
	#ifdef DEBUG
	printf("Len of Output:%d\n",noutput_add_CRC);
	for(i=0;i<noutput_add_CRC;i++)
	{
		printf("Out[%d]:%x (hex)\n",i,add_CRC[i]);
	}
	
	printf("\n------------------Hanmming coding-----------------------\n");
	#endif
	
	hanmingcode_data = Hanmming_Enc(cr, sf, str_tx, add_CRC, noutput_add_CRC, &noutput_hanmming_coding);
	
	#ifdef DEBUG
	printf("Len of Output:%d\n",noutput_hanmming_coding);
	for(i=0;i<noutput_hanmming_coding;i++)
	{
		printf("Out[%d]:%x (hex)\n",i,hanmingcode_data[i]);
	}
	
	printf("\n------------------Interleaver-----------------------\n");
	#endif
	
	interleaver_data = Interleaver(cr, sf, str_tx, hanmingcode_data, noutput_hanmming_coding, &noutput_interleaver);
	
	#ifdef DEBUG
	printf("Len of Output:%d\n",noutput_interleaver);
	for(i=0;i<noutput_interleaver;i++)
	{
		printf("Out[%d]:%x (hex)\n",i,interleaver_data[i]);
	}
	
	printf("\n------------------Gray coder-----------------------\n");
	#endif
	
	gray_data = Gray_Decoder(cr, sf, str_tx, interleaver_data, noutput_interleaver, &noutput_gray);
	
	#ifdef DEBUG
	printf("Len of Output:%d\n",noutput_gray);
	for(i=0;i<noutput_interleaver;i++)
	{
		printf("Out[%d]:%x (hex)\n",i,gray_data[i]);
	}
	
	printf("\n------------------Modualtion-----------------------\n");
	#endif
	
	modulation_data = Modulation(cr, sf, bw, gray_data, noutput_gray, &noutput_modulation);
	
	#ifdef DEBUG
	printf("Len of Output:%d\n",noutput_modulation);
	for(i=0;i<noutput_modulation;i++)
	{
		printf("Out[%d]:%d (float)\n",i,modulation_data[i]);
	}
	
	#endif
	
	*symbol_len = noutput_modulation;
	
	
	return modulation_data;
	
	
	
}
