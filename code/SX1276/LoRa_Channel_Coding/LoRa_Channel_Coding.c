#include "LoRa_Channel_Coding.h"
#include <string.h>
#include <stdlib.h>
//#include "malloc.h" 

//#define DEBUG
#ifdef DEBUG
#include "usart.h"
#endif
                          



int *LoRa_Channel_Coding(uint8_t *tx_buffer, uint16_t buffer_size, uint32_t bw, uint8_t sf, uint8_t cr, bool has_crc, bool impl_head, int *symbol_len, bool low_data_rate_optimize)
{
	int i;
	uint8_t *whitened_data = NULL;
	uint8_t *add_header_data = NULL;
	uint8_t *add_CRC_data = NULL;
	uint8_t *hanmingcode_data = NULL;
	uint16_t *interleaver_data = NULL;
	uint16_t *gray_data = NULL;
	int 	*modulation_data = NULL;
	
	uint16_t noutput_whitening = 0;
	uint16_t noutput_add_header = 0;
	uint16_t noutput_add_CRC = 0;
	uint16_t noutput_hanmming_coding = 0;
	uint16_t noutput_interleaver = 0;
	uint16_t noutput_gray = 0;
	uint16_t noutput_modulation = 0;
	
	
	
	#ifdef DEBUG
	for(i=0;i<buffer_size;i++)
	{
		printf("Out[%d]:%x (hex)\n",i,tx_buffer[i]);
	}
	
	printf("\n------------------Whitening-----------------------\n");
	#endif
	
	whitened_data = Whitening(tx_buffer, buffer_size, &noutput_whitening);
	
	#ifdef DEBUG
	printf("Len of Output:%d\n",noutput_whitening);
	for(i=0;i<noutput_whitening;i++)
	{
		printf("Out[%d]:%x (hex)\n",i,whitened_data[i]);
	}
	printf("\n------------------Add header-----------------------\n");
	#endif
	
	add_header_data = Add_Header(impl_head, has_crc, cr, buffer_size, whitened_data, noutput_whitening, &noutput_add_header);
	
	free(whitened_data);
	
	#ifdef DEBUG
	printf("Len of Output:%d\n",noutput_add_header);
	for(i=0;i<noutput_add_header;i++)
	{
		printf("Out[%d]:%x (hex)\n",i,add_header_data[i]);
	}
	
	printf("\n------------------Add CRC-----------------------\n");
	#endif
	
	add_CRC_data = Add_CRC(has_crc, tx_buffer, buffer_size, add_header_data, noutput_add_header, &noutput_add_CRC);
	
	free(add_header_data);
	
	#ifdef DEBUG
	printf("Len of Output:%d\n",noutput_add_CRC);
	for(i=0;i<noutput_add_CRC;i++)
	{
		printf("Out[%d]:%x (hex)\n",i,add_CRC_data[i]);
	}
	
	printf("\n------------------Hanmming coding-----------------------\n");
	#endif
	
	hanmingcode_data = Hanmming_Enc(cr, sf, add_CRC_data, noutput_add_CRC, &noutput_hanmming_coding);
	
	free(add_CRC_data);
	
	#ifdef DEBUG
	printf("Len of Output:%d\n",noutput_hanmming_coding);
	for(i=0;i<noutput_hanmming_coding;i++)
	{
		printf("Out[%d]:%x (hex)\n",i,hanmingcode_data[i]);
	}
	
	printf("\n------------------Interleaver-----------------------\n");
	#endif
	
	interleaver_data = Interleaver(cr, sf, low_data_rate_optimize, hanmingcode_data, noutput_hanmming_coding, &noutput_interleaver);
	
	free(hanmingcode_data);
	
	#ifdef DEBUG
	printf("Len of Output:%d\n",noutput_interleaver);
	for(i=0;i<noutput_interleaver;i++)
	{
		printf("Out[%d]:%x (hex)\n",i,interleaver_data[i]);
	}
	
	printf("\n------------------Gray coder-----------------------\n");
	#endif
	
	gray_data = Gray_Decoder(cr, sf, low_data_rate_optimize, interleaver_data, noutput_interleaver, &noutput_gray);
	
	free(interleaver_data);

	
	#ifdef DEBUG
	printf("Len of Output:%d\n",noutput_gray);
	for(i=0;i<noutput_gray;i++)
	{
		printf("Out[%d]:0x%x\n",i,gray_data[i]);
	}
	
	printf("\n------------------Modualtion-----------------------\n");
	#endif
	
	modulation_data = Modulation(cr, sf, bw, gray_data, noutput_gray, &noutput_modulation);
	
	free(gray_data);
	
	#ifdef DEBUG
	printf("Len of Output:%d\n",noutput_modulation);
	for(i=0;i<noutput_modulation;i++)
	{
		printf("Out[%d]:\t%d,\t (int)\n",i,modulation_data[i]);
	}
	
	#endif
	
	*symbol_len = noutput_modulation;
	
	
	return modulation_data;
	
	
	
}
