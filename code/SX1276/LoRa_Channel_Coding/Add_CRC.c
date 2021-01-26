#include "Add_CRC.h"
#include "string.h"
#include <stdlib.h>


#define DEBUG


unsigned int crc16(unsigned int crcValue, unsigned char newByte) 
{
	unsigned char i;
	for (i = 0; i < 8; i++) 
	{
		if (((crcValue & 0x8000) >> 8) ^ (newByte & 0x80))
		{
				crcValue = (crcValue << 1)  ^ 0x1021;
		}
		else
		{
				crcValue = (crcValue << 1);
		}
		newByte <<= 1;
	}
	return crcValue;
}


uint8_t *Add_CRC(bool has_crc, char *input_str, uint8_t *input, uint8_t ninput_items, uint8_t *noutput_items)
{
	uint8_t *output;
	if(has_crc)
	{//append the CRC to the payload
		output = malloc(ninput_items+4);
		uint16_t crc=0x0000;
		
		memcpy(output,input,ninput_items*sizeof(uint8_t));
		
		uint8_t payload_len = strlen(input_str);
		//calculate CRC on the N-2 firsts data bytes using Poly=1021 Init=0000
		for(int i =0;i<(int)payload_len-2;i++)
		crc=crc16(crc,input_str[i]);

		//XOR the obtained CRC with the last 2 data bytes
		crc=crc ^ input_str[payload_len-1] ^ (input_str[payload_len-2]<<8);

		output[ninput_items]  = ((crc & 0x000F));
		output[ninput_items+1]= ((crc & 0x00F0)>>4);
		output[ninput_items+2]= ((crc & 0x0F00)>>8);
		output[ninput_items+3]= ((crc & 0xF000)>>12);

		*noutput_items = ninput_items+4;
	}
	else
	{
		output= malloc(ninput_items);
		memcpy(output,input,ninput_items*sizeof(uint8_t));
		*noutput_items = ninput_items;
	}
	
	free(output);
	return output;
}
