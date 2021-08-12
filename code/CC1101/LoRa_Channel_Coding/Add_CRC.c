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


uint8_t *Add_CRC(bool has_crc, uint8_t *tx_buffer, uint16_t buffer_size, uint8_t *input, uint16_t ninput_items, uint16_t *noutput_items)
{
	uint8_t *output;
	if(has_crc)
	{//append the CRC to the payload
		output = malloc((ninput_items+4)*sizeof(uint8_t));
		uint16_t crc=0x0000;
		
		memcpy(output,input,ninput_items*sizeof(uint8_t));
		
		uint8_t payload_len = buffer_size;
		//calculate CRC on the N-2 firsts data bytes using Poly=1021 Init=0000
		for(int i =0;i<(int)payload_len-2;i++)
			crc=crc16(crc,tx_buffer[i]);

		//XOR the obtained CRC with the last 2 data bytes
		if(payload_len<2)
			crc=crc ^ tx_buffer[payload_len-1] ^ 0x00;
		else
			crc=crc ^ tx_buffer[payload_len-1] ^ (tx_buffer[payload_len-2]<<8);

		output[ninput_items]  = ((crc & 0x000F));
		output[ninput_items+1]= ((crc & 0x00F0)>>4);
		output[ninput_items+2]= ((crc & 0x0F00)>>8);
		output[ninput_items+3]= ((crc & 0xF000)>>12);

		*noutput_items = ninput_items+4;
	}
	else
	{
		output= malloc(ninput_items*sizeof(uint8_t));
		memcpy(output,input,ninput_items*sizeof(uint8_t));
		*noutput_items = ninput_items;
	}
	
	return output;
}
