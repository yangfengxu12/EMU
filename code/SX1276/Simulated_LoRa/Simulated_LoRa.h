#ifndef _SIMULATED_LORA_H__
#define _SIMULATED_LORA_H__

#include "stm32l4xx.h"
#include "fast_spi.h"
#include <stdlib.h>
#include <math.h>

#define RF_FREQUENCY                                486300000// Hz
#define TX_OUTPUT_POWER                             14        // dBm
#define DATA_RATE																		250000

#define FREQ_OFFSET_1_2															400000
	
#define LORA_BW																			125000		// Hz

/**********  Packet 1 parameters    **************************/
#define RF_FREQUENCY_NO1														RF_FREQUENCY

//#define LORA_BASE_FREQ_NO1													(RF_FREQUENCY_NO1 - (LORA_BW >> 1)) // Hz
//#define LORA_MAX_FREQ_NO1														(RF_FREQUENCY_NO1 + (LORA_BW >> 1)) // Hz


#define LORA_SF_NO1													 				7				// spread factor
#define LORA_CR_NO1																	1				// coding rate [1:4/5, 2:4/6, 3:4/7,  4:4/8]
#define LORA_HAS_CRC_NO1														true 	// true or false
#define LORA_IMPL_HEAD_NO1													false		// true or false
#define LORA_LOWDATERATEOPTIMIZE_NO1								false		// true or false

#define LORA_PREAMBLE_LENGTH_NO1										8
#define LORA_ID_LENGTH_NO1													2
#define LORA_SFD_LENGTH_NO1													2
#define LORA_QUARTER_SFD_LENGTH_NO1									1

/**********  Packet 2 parameters    **************************/
#define RF_FREQUENCY_NO2														(RF_FREQUENCY_NO1 + 400000) 

//#define LORA_BASE_FREQ_NO2													(RF_FREQUENCY_NO2 - (LORA_BW >> 1)) // Hz
//#define LORA_MAX_FREQ_NO2														(RF_FREQUENCY_NO2 + (LORA_BW >> 1)) // Hz

#define LORA_SF_NO2																	12				// spread factor
#define LORA_CR_NO2																	1				// coding rate [1,2,3,4] ([4/5,4/6,4/7,4/8])
#define LORA_HAS_CRC_NO2														true		// true or false
#define LORA_IMPL_HEAD_NO2													false		// true or false
#define LORA_LOWDATERATEOPTIMIZE_NO2								true		// true or false																						

#define LORA_PREAMBLE_LENGTH_NO2										8
#define LORA_ID_LENGTH_NO2													2
#define LORA_SFD_LENGTH_NO2 												2
#define LORA_QUARTER_SFD_LENGTH_NO2									1


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

void Fast_SetChannel( uint32_t Input_Freq );

void channel_coding_convert(int* freq_points,int id_and_payload_symbol_len,int CF);
void symbol_start_end_time_cal(void);
void LoRa_Generate_Signal(int * freq_points, int id_and_payload_symbol_len, int CF);


void channel_coding_convert_with_blank(int * freq_points,int id_and_payload_symbol_len,int CF);
void LoRa_Generate_Signal_With_Blank(int * freq_points, int id_and_payload_symbol_len, float blank,int CF);


void channel_coding_convert_double_packets(int* freq_points_no1,int id_and_payload_symbol_len_no1,int* freq_points_no2,int id_and_payload_symbol_len_no2,int CF1,int CF2);
void symbol_start_end_time_cal_double_packets(void);
void LoRa_Generate_Double_Packet(int * freq_points_No1, int id_and_payload_symbol_len_No1, int * freq_points_No2, int id_and_payload_symbol_len_No2,int CF1,int CF2);

#endif
