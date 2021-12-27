#ifndef _SIMULATED_LORA_H__
#define _SIMULATED_LORA_H__

#include "stm32l4xx.h"
#include "fast_spi.h"
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>

//#define RF_FREQUENCY                                486300000// Hz
//#define TX_OUTPUT_POWER                             0        // dBm
//#define DATA_RATE																		250000

//#define FREQ_OFFSET_1_2															400000
	
//#define LORA_BW																			125000		// Hz

/**********  Packet 1 parameters    **************************/
//#define RF_FREQUENCY_NO1														RF_FREQUENCY

//#define LORA_BASE_FREQ_NO1													(RF_FREQUENCY_NO1 - (LORA_BW >> 1)) // Hz
//#define LORA_MAX_FREQ_NO1														(RF_FREQUENCY_NO1 + (LORA_BW >> 1)) // Hz


//#define LORA_SF_NO1													 				11				// spread factor
//#define LORA_CR_NO1																	1				// coding rate [1:4/5, 2:4/6, 3:4/7,  4:4/8]
//#define LORA_HAS_CRC_NO1														true 	// true or false
//#define LORA_IMPL_HEAD_NO1													false		// true or false
//#define LORA_LOWDATERATEOPTIMIZE_NO1								false		// true or false

#define LORA_PREAMBLE_LENGTH_CH1										8
#define LORA_ID_LENGTH_CH1													2
#define LORA_SFD_LENGTH_CH1													2
#define LORA_QUARTER_SFD_LENGTH_CH1									1

/**********  Packet 2 parameters    **************************/
//#define RF_FREQUENCY_NO2														(RF_FREQUENCY_NO1 + 400000) 

//#define LORA_BASE_FREQ_NO2													(RF_FREQUENCY_NO2 - (LORA_BW >> 1)) // Hz
//#define LORA_MAX_FREQ_NO2														(RF_FREQUENCY_NO2 + (LORA_BW >> 1)) // Hz

//#define LORA_SF_NO2																	8				// spread factor
//#define LORA_CR_NO2																	1				// coding rate [1,2,3,4] ([4/5,4/6,4/7,4/8])
//#define LORA_HAS_CRC_NO2														true		// true or false
//#define LORA_IMPL_HEAD_NO2													false		// true or false
//#define LORA_LOWDATERATEOPTIMIZE_NO2								true		// true or false																						

#define LORA_PREAMBLE_LENGTH_CH2										8
#define LORA_ID_LENGTH_CH2													2
#define LORA_SFD_LENGTH_CH2 												2
#define LORA_QUARTER_SFD_LENGTH_CH2									1


/**********  common 2 parameters    **************************/
#define FREQ_STEP                                   61.03515625
#define FREQ_STEP_8                                 15625 /* FREQ_STEP<<8 */

/* channel = Freq / FREQ_STEP */
#define SX_FREQ_TO_CHANNEL( channel, freq )                                                                       \
    do                                                                                                            \
    {                                                                                                             \
        uint32_t initialFreqInt, initialFreqFrac;                                                                 \
        initialFreqInt = freq / FREQ_STEP_8;                                                                      \
        initialFreqFrac = freq - ( initialFreqInt * FREQ_STEP_8 );                                                \
        channel = ( initialFreqInt << 8 ) + ( ( ( initialFreqFrac << 8 ) + ( FREQ_STEP_8 / 2 ) ) / FREQ_STEP_8 ); \
    }while( 0 )
	
	
enum Chirp_Status{
	Preamble = 0,
	ID,
	SFD,
	Quarter_SFD,
	Payload
};


		
extern uint32_t Time;

int Simulated_LoRa_Init_SX1276(uint32_t central_freq, uint8_t tx_power, uint16_t time );

int Simulated_LoRa_Tx(
											uint8_t mode_select, uint8_t snipped_ratio,
											//channel No.1 parameters
											uint32_t central_freq_ch1, uint8_t *tx_buffer_ch1,uint8_t tx_buffer_length_ch1, uint32_t bw_ch1, uint8_t sf_ch1, uint8_t cr_ch1, bool has_crc_ch1, bool implict_header_ch1, bool ldro_ch1,
											//channel No.2 parameters if necessary
											uint32_t central_freq_ch2, uint8_t *tx_buffer_ch2,uint8_t tx_buffer_length_ch2, uint32_t bw_ch2, uint8_t sf_ch2, uint8_t cr_ch2, bool has_crc_ch2, bool implict_header_ch2, bool ldro_ch2
);


void Fast_SetChannel( uint32_t Input_Freq );

void Symbol_Start_End_Time_Cal(uint8_t sf);
											
void Channel_Coding_Convert(int *freq_points,int id_and_payload_symbol_len,int central_freq, uint8_t sf);
void LoRa_Generate_Signal(int *freq_points, int id_and_payload_symbol_len, uint32_t central_freq, uint32_t bw, uint8_t sf);

void LoRa_Generate_Signal_Sinpping(int * freq_points, int id_and_payload_symbol_len, float gap_width, uint32_t central_freq, uint32_t bw, uint8_t sf);
void LoRa_Generate_Signal_With_Blank(int * freq_points, int id_and_payload_symbol_len, float blank,int CF);

											
void Channel_Coding_Convert_Multiplexing(int* freq_points_no1,int id_and_payload_symbol_len_no1,int* freq_points_no2,int id_and_payload_symbol_len_no2,
																				int central_freq_no1,int central_freq_no2);

void Symbol_Start_End_Time_Cal_Multiplexing(uint8_t sf_No1, uint8_t sf_No2);
void LoRa_Generate_Double_Packet(int *freq_points_No1, int id_and_payload_symbol_len_No1, int *freq_points_No2, int id_and_payload_symbol_len_No2, 
																uint32_t central_freq_No1, uint32_t central_freq_No2, uint32_t bw_ch1, uint8_t bw_ch2, uint8_t sf_No1, uint8_t sf_No2);
#endif
