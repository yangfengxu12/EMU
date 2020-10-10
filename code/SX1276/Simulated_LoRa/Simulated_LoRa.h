#ifndef _SIMULATED_LORA_H__
#define _SIMULATED_LORA_H__

#include "stm32l4xx.h"
#include "fast_spi.h"

#define RF_FREQUENCY                                433000000 // Hz
#define TX_OUTPUT_POWER                             14        // dBm

#define LORA_BW																			125000		// Hz
#define LORA_SF																			7				// spread factor
#define LORA_BASE_FREQ															(RF_FREQUENCY - (LORA_BW >> 1)) // Hz
#define LORA_MAX_FREQ																(RF_FREQUENCY + (LORA_BW >> 1)) // Hz


#define LORA_PREAMBLE_LENGTH												8
#define LORA_ID_LENGTH															2
#define LORA_SFD_LENGTH															2
#define LORA_QUARTER_SFD_LENGTH											1
#define LORA_PAYLOAD_LENGTH													( sizeof(LoRa_Payload_Start_Freq)/sizeof(LoRa_Payload_Start_Freq[0] ))
#define LORA_TOTAL_LENGTH														( LORA_PREAMBLE_LENGTH + LORA_ID_LENGTH + LORA_SFD_LENGTH + LORA_QUARTER_SFD_LENGTH + LORA_PAYLOAD_LENGTH )

#define LORA_FREQ_STEP															( LORA_BW / ( 1 << LORA_SF ))
#define LORA_SYMBOL_TIME														( 8 * ( 1 << LORA_SF ))

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



void LoRa_Generate_Signal( void );

void Fast_SetChannel( uint8_t *freq, uint8_t Changed_Register_Count );
void LoRa_UpChirp( void );
void LoRa_DownChirp( void );
void Generate_Quarter_DownChirp( void );
void LoRa_Payload( int Start_freq );


#endif
