#include "hw.h"
#include "radio.h"
#include "sx1276.h"
#include "sx1276mb1mas.h"
#include "delay.h"
#include "control_GPIO.h"

#include "LoRa_Channel_Coding.h"
#include "Simulated_LoRa.h"


#include "Timer_Calibration_From_SX1276.h"
#define COMPENSATION_TIME (( (uint16_t)TIM4->CNT << 16 ) | (uint16_t)TIM3->CNT ) 

enum EMU_MODE_SELECT
{
	normal_lora = 0,
	snipped_lora = 1,
	multiplexed_lora = 2,
}EMU_MODE_SELECT;	


int Lora_Packet_Symbol_Length_Ch1	=	0; 	// global temp
int Lora_Packet_Symbol_Length_Ch2	=	0;	// global temp

enum Chirp_Status Chirp_Status_Ch1 = Preamble;
enum Chirp_Status Chirp_Status_Ch2 = Preamble;

int *LoRa_Start_Freq_Ch1;
int *LoRa_Start_Freq_Ch2;

uint32_t *Symbol_Start_Time_Ch1;
uint32_t *Symbol_End_Time_Ch1;
uint32_t *Symbol_Start_Time_Ch2;
uint32_t *Symbol_End_Time_Ch2;



float Input_Freq;
uint32_t Channel;

uint8_t Channel_Freq[3] = {0};  //MSB,MID,LSB
uint8_t Changed_Register_Count = 1;  // the number of changed registers.


int Simulated_LoRa_Tx(
											uint8_t mode_select, uint8_t snipped_ratio,
											//channel No.1 parameters
											uint32_t central_freq_ch1, uint8_t *tx_buffer_ch1,uint8_t tx_buffer_length_ch1, uint32_t bw_ch1, uint8_t sf_ch1, uint8_t cr_ch1, bool has_crc_ch1, bool implict_header_ch1, bool ldro_ch1,
											//channel No.2 parameters if necessary
											uint32_t central_freq_ch2, uint8_t *tx_buffer_ch2,uint8_t tx_buffer_length_ch2, uint32_t bw_ch2, uint8_t sf_ch2, uint8_t cr_ch2, bool has_crc_ch2, bool implict_header_ch2, bool ldro_ch2
)
{
	int *packet_freq_points_ch1 = NULL;
	int *packet_freq_points_ch2 = NULL;
	
	int symbol_len_ch1 = 0;
	int symbol_len_ch2 = 0;
	/****** parameters check *******/
	if(mode_select == multiplexed_lora)
	{
		ldro_ch1 = true;
		ldro_ch2 = true;
	}
	if(sf_ch1 < 7)
		sf_ch1 = 7;
	else if (sf_ch1 > 12)
		sf_ch1 = 12;
	
	if(sf_ch2 < 7)
		sf_ch2 = 7;
	else if (sf_ch2 > 12)
		sf_ch2 = 12;
	
	if(sf_ch1 >= 11)
		ldro_ch1 = true;
	if(sf_ch2 >= 11)
		ldro_ch2 = true;
	
	/******LoRa channel coding preparation******/
	packet_freq_points_ch1 = LoRa_Channel_Coding(tx_buffer_ch1, tx_buffer_length_ch1, bw_ch1, sf_ch1, cr_ch1, has_crc_ch1, implict_header_ch1, &symbol_len_ch1, ldro_ch1);

	if(mode_select == multiplexed_lora)
		packet_freq_points_ch2 = LoRa_Channel_Coding(tx_buffer_ch2, tx_buffer_length_ch2, bw_ch2, sf_ch2, cr_ch2, has_crc_ch2, implict_header_ch2, &symbol_len_ch2, ldro_ch2);
	
	/******end of LoRa channel coding preparation******/
	/******transmit CSS waveform******/
	if(mode_select == normal_lora)
		LoRa_Generate_Signal(packet_freq_points_ch1,symbol_len_ch1,central_freq_ch1, bw_ch1, sf_ch1);
	else if(mode_select == snipped_lora)
		LoRa_Generate_Signal_Sinpping(packet_freq_points_ch1, snipped_ratio, symbol_len_ch1, central_freq_ch1, bw_ch1, sf_ch1);
	else if(mode_select == multiplexed_lora)
		LoRa_Generate_Double_Packet(packet_freq_points_ch1,symbol_len_ch1,packet_freq_points_ch2,symbol_len_ch2,central_freq_ch1,central_freq_ch2, bw_ch1, bw_ch2, sf_ch1, sf_ch2);
	else
		return 0;
	/******end of transmit CSS waveform******/
	/****** relase the temps ********/
	free(packet_freq_points_ch1);
	if(mode_select == multiplexed_lora)
		free(packet_freq_points_ch2);
	/******end of relase the temps ********/
}

int Simulated_LoRa_Init_SX1276(uint32_t central_freq, uint8_t tx_power)
{
	int device_datarate = 250000; // SX1276 OOK/FSK datarate
	uint16_t datarate = 0;
	
	SX1276SetChannel( central_freq );
	SX1276SetTxContinuousWave( central_freq, tx_power, 3 );
	
	SX1276Write( REG_OSC, RF_OSC_CLKOUT_1_MHZ );
	
	SX1276Write( REG_PLLHOP, ( SX1276Read( REG_PLLHOP ) & RF_PLLHOP_FASTHOP_MASK ) | RF_PLLHOP_FASTHOP_ON );
	SX1276Write( REG_PLL, ( SX1276Read( REG_PLL ) & RF_PLL_BANDWIDTH_MASK ) | RF_PLL_BANDWIDTH_150 );
	SX1276Write( REG_PARAMP, ( SX1276Read( REG_PARAMP ) & RF_PARAMP_MASK ) | RF_PARAMP_0010_US );
	SX1276Write( REG_PARAMP, ( SX1276Read( REG_PARAMP ) & RF_PARAMP_MODULATIONSHAPING_MASK ) | RF_PARAMP_MODULATIONSHAPING_00 );
	SX1276Write( REG_OCP, ( SX1276Read( REG_OCP ) & RF_OCP_MASK ) | RF_OCP_OFF );
	
	datarate = ( uint16_t )( ( double )XTAL_FREQ / ( double )device_datarate );
	SX1276Write( REG_BITRATEMSB, ( uint8_t )( datarate >> 8 ) );
	SX1276Write( REG_BITRATELSB, ( uint8_t )( datarate & 0xFF ) );
	
	return 1;
}

void Fast_SetChannel( uint32_t Input_Freq )
{
	uint8_t Reg_Address;
	uint8_t Channel_Freq_MSB_temp = 0;
	uint8_t Channel_Freq_MID_temp = 0;
	uint8_t Channel_Freq_LSB_temp = 0;
	
	SX_FREQ_TO_CHANNEL( Channel, (uint32_t)Input_Freq );
	Channel_Freq[0] = ( uint8_t )(( Channel >> 16 ) & 0xFF );
	Channel_Freq[1] = ( uint8_t )(( Channel >> 8 ) & 0xFF );
	Channel_Freq[2] = ( uint8_t )(( Channel ) & 0xFF );
	
	if( Channel_Freq_MSB_temp != Channel_Freq[0] )
	{
		Changed_Register_Count = 3;
		Channel_Freq_MSB_temp = Channel_Freq[0];
		Channel_Freq_MID_temp = Channel_Freq[1];
		Channel_Freq_LSB_temp = Channel_Freq[2];
	}
	else if( Channel_Freq_MID_temp != Channel_Freq[1] )
	{
		Changed_Register_Count = 2;
		Channel_Freq_MID_temp = Channel_Freq[1];
		Channel_Freq_LSB_temp = Channel_Freq[2];
	}
	else if( Channel_Freq_LSB_temp != Channel_Freq[2] )
	{
		Changed_Register_Count = 1;
		Channel_Freq_LSB_temp = Channel_Freq[2];
	}
	
	switch(Changed_Register_Count)
	{
		case 1:Reg_Address = REG_FRFLSB;break;
		case 2:Reg_Address = REG_FRFMID;break;
		case 3:Reg_Address = REG_FRFMSB;break;
		default:Reg_Address = REG_FRFMSB;break;
	}
	SX1276_Burst_Write( Reg_Address, Channel_Freq + 3 - Changed_Register_Count, Changed_Register_Count);
}

void Symbol_Start_End_Time_Cal(uint8_t sf)
{
	Symbol_Start_Time_Ch1 = malloc(Lora_Packet_Symbol_Length_Ch1 * sizeof(uint32_t));
	Symbol_End_Time_Ch1 = malloc(Lora_Packet_Symbol_Length_Ch1 * sizeof(uint32_t));
	
	for(int i=0;i < Lora_Packet_Symbol_Length_Ch1;i++)
	{
		if(i< LORA_PREAMBLE_LENGTH_CH1 + LORA_ID_LENGTH_CH1 + LORA_SFD_LENGTH_CH1 + LORA_QUARTER_SFD_LENGTH_CH1)
			Symbol_Start_Time_Ch1[i] = (i*(1<<sf))<<3;
		else
			Symbol_Start_Time_Ch1[i] = ((i-1)*(1<<sf) + (1<<sf)/4) << 3;
	}
	Symbol_End_Time_Ch1 = Symbol_Start_Time_Ch1 + 1;
}

void Channel_Coding_Convert(int *freq_points,int id_and_payload_symbol_len,int central_freq, uint8_t sf)
{
  Lora_Packet_Symbol_Length_Ch1			=	LORA_PREAMBLE_LENGTH_CH1 + LORA_ID_LENGTH_CH1 + \
															LORA_SFD_LENGTH_CH1 + LORA_QUARTER_SFD_LENGTH_CH1 + \
															id_and_payload_symbol_len;
	
	LoRa_Start_Freq_Ch1 = malloc(Lora_Packet_Symbol_Length_Ch1 * sizeof(int));
	
	int freq_offset = 0;
	if(sf == 7)
		freq_offset = 500;
	else if(sf == 8)
		freq_offset = 700;
	else if(sf == 9)
		freq_offset = 600;
	else if(sf == 10)
		freq_offset = 300;
	else if(sf == 11)
		freq_offset = 600;
	else if(sf == 12)
		freq_offset = 600;
	else
		while(1);
	
	for(int i=0; i< Lora_Packet_Symbol_Length_Ch1; i++)
	{
		if(i < LORA_PREAMBLE_LENGTH_CH1)
		{
			LoRa_Start_Freq_Ch1[i] = central_freq - 62500;
		}
		else if(i < LORA_PREAMBLE_LENGTH_CH1 + LORA_ID_LENGTH_CH1)
		{
			LoRa_Start_Freq_Ch1[i] = central_freq + freq_points[i - LORA_PREAMBLE_LENGTH_CH1];
		}
		else if(i < LORA_PREAMBLE_LENGTH_CH1 + LORA_ID_LENGTH_CH1 + LORA_SFD_LENGTH_CH1 + LORA_QUARTER_SFD_LENGTH_CH1)
		{
			LoRa_Start_Freq_Ch1[i] = central_freq + 62500 + freq_offset;
		}
		else if(i < Lora_Packet_Symbol_Length_Ch1)
		{
			LoRa_Start_Freq_Ch1[i] = central_freq + freq_points[i - LORA_PREAMBLE_LENGTH_CH1 - LORA_SFD_LENGTH_CH1 -  LORA_QUARTER_SFD_LENGTH_CH1];
		}
		else
		{
			while(1);
		}
	}
}


void LoRa_Generate_Signal(int *freq_points, int id_and_payload_symbol_len, uint32_t central_freq, uint32_t bw, uint8_t sf)
{
	/******** debug temps define ***********/
	
	/******** end of debug temps def ***********/
	float 	 lora_chip_step_freq	=	(float)bw / (float)((1 << sf));
	int 		 lora_symbol_duartion	=	(( 1 << sf ) << 3);
	
	Channel_Coding_Convert(freq_points, id_and_payload_symbol_len, central_freq, sf);
	Symbol_Start_End_Time_Cal(sf);
	
	int lora_bw_min_freq  = (central_freq - (bw >> 1)); // Hz
	int lora_bw_max_freq	=	(central_freq + (bw >> 1)); // Hz

	int chip_position = 0;

	uint32_t chirp_count = 0;
	
	uint32_t total_chip_count = 0;
	uint32_t symbol_chip_count = 0;
	
	Init_Timer_Calibration_From_SX1276();
	
	Fast_SetChannel( central_freq );
	
	Send_packets:
	LL_GPIO_SetOutputPin(GPIOB,GPIO_PIN_5);
 	SX1276SetOpMode( RF_OPMODE_TRANSMITTER );
	
	delay_ms(1);
	
	LL_TIM_EnableCounter(TIM3);
	LL_TIM_EnableCounter(TIM4);
	TIM3->CNT = 0;
	TIM4->CNT = 0;
	/*******************/
	while( chirp_count < Lora_Packet_Symbol_Length_Ch1)
	{
		while(COMPENSATION_TIME <= Symbol_End_Time_Ch1[chirp_count])  // generate symbol
		{
			if(chirp_count < LORA_PREAMBLE_LENGTH_CH1)
			{
				Chirp_Status_Ch1 = Preamble;
				chip_position = (COMPENSATION_TIME - Symbol_Start_Time_Ch1[chirp_count] ) >> 3;
				Input_Freq = LoRa_Start_Freq_Ch1[chirp_count] + (chip_position * lora_chip_step_freq);
			}
			else if(chirp_count < LORA_PREAMBLE_LENGTH_CH1 + LORA_ID_LENGTH_CH1)
			{
				Chirp_Status_Ch1 = ID;
				chip_position = (COMPENSATION_TIME - Symbol_Start_Time_Ch1[chirp_count] ) >> 3;
				Input_Freq = LoRa_Start_Freq_Ch1[chirp_count] + (chip_position * (lora_chip_step_freq));
				if( Input_Freq > lora_bw_max_freq )
					Input_Freq = Input_Freq - bw;
			}
			else if(chirp_count < LORA_PREAMBLE_LENGTH_CH1 + LORA_ID_LENGTH_CH1 + LORA_SFD_LENGTH_CH1)
			{
				Chirp_Status_Ch1 = SFD;
				chip_position = (COMPENSATION_TIME - Symbol_Start_Time_Ch1[chirp_count] ) >> 3;
				Input_Freq = LoRa_Start_Freq_Ch1[chirp_count] - (float)chip_position * (lora_chip_step_freq);
			}
			else if(chirp_count < LORA_PREAMBLE_LENGTH_CH1 + LORA_ID_LENGTH_CH1 + LORA_SFD_LENGTH_CH1 + LORA_QUARTER_SFD_LENGTH_CH1)
			{
				Chirp_Status_Ch1 = Quarter_SFD;
				chip_position = (COMPENSATION_TIME - Symbol_Start_Time_Ch1[chirp_count] ) >> 3;
				Input_Freq = LoRa_Start_Freq_Ch1[chirp_count] - (float)chip_position * (lora_chip_step_freq);
			}
			else if(chirp_count < Lora_Packet_Symbol_Length_Ch1)
			{
				Chirp_Status_Ch1 = Payload;
				chip_position = ( COMPENSATION_TIME - Symbol_Start_Time_Ch1[chirp_count] ) >> 3;
				Input_Freq = LoRa_Start_Freq_Ch1[chirp_count] + (float)chip_position * lora_chip_step_freq;
				if( Input_Freq > lora_bw_max_freq )
					Input_Freq = Input_Freq - bw;
			}
			else 
				break;
			
			Fast_SetChannel( Input_Freq );
			
			while( COMPENSATION_TIME & ( 8 - 1 ));// chip time = 8us
			
			LL_GPIO_SetOutputPin(GPIOB,GPIO_PIN_5);
			
			total_chip_count++;
			symbol_chip_count++;
			Changed_Register_Count = 1;

		}	// end loop of symbol
		Symbol_End:
		symbol_chip_count = 0;
		chirp_count++;
		chip_position = 0;
	}

	SX1276SetOpMode( RF_OPMODE_SLEEP );
	LL_GPIO_ResetOutputPin(GPIOB,GPIO_PIN_5);
	
	free(LoRa_Start_Freq_Ch1);
	free(Symbol_Start_Time_Ch1);
	free(Symbol_End_Time_Ch1);
	
	total_chip_count = 0;
	chirp_count = 0;
	Chirp_Status_Ch1 = Preamble;

	TIM3->CNT = 0;
	TIM4->CNT = 0;
	LL_TIM_DisableCounter(TIM3);
	LL_TIM_DisableCounter(TIM4);
}
//*************************************************************/
//
//
//
//  Chirp sinpping
//
//
//
//*************************************************************/
void Channel_Coding_Convert_Snipping(int * freq_points,int id_and_payload_symbol_len, uint32_t central_freq, uint8_t sf)
{
  Lora_Packet_Symbol_Length_Ch1	=	LORA_PREAMBLE_LENGTH_CH1 + LORA_ID_LENGTH_CH1 + \
																	LORA_SFD_LENGTH_CH1 + LORA_QUARTER_SFD_LENGTH_CH1 + \
																	id_and_payload_symbol_len;
	
	LoRa_Start_Freq_Ch1 = malloc(Lora_Packet_Symbol_Length_Ch1 * sizeof(int));
	
	int freq_offset = 0;
	if(sf == 7)
		freq_offset = 660;
	else if(sf == 8)
		freq_offset = 250;
	else if(sf == 9)
		freq_offset = 150;
	else if(sf == 10)
		freq_offset = 90;
	else if(sf == 11)
		freq_offset = 500;
	else if(sf == 12)
		freq_offset = 500;
	else
		while(1);
	
	for(int i=0; i< Lora_Packet_Symbol_Length_Ch1; i++)
	{
		if(i < LORA_PREAMBLE_LENGTH_CH1)
		{
			LoRa_Start_Freq_Ch1[i] = central_freq - 62500;
		}
		else if(i < LORA_PREAMBLE_LENGTH_CH1 + LORA_ID_LENGTH_CH1)
		{
			LoRa_Start_Freq_Ch1[i] = central_freq + freq_points[i - LORA_PREAMBLE_LENGTH_CH1];
		}
		else if(i < LORA_PREAMBLE_LENGTH_CH1 + LORA_ID_LENGTH_CH1 + LORA_SFD_LENGTH_CH1 + LORA_QUARTER_SFD_LENGTH_CH1)
		{
			LoRa_Start_Freq_Ch1[i] = central_freq + 62500 + freq_offset;
		}
		else if(i < Lora_Packet_Symbol_Length_Ch1)
		{
			LoRa_Start_Freq_Ch1[i] = central_freq + freq_points[i - LORA_PREAMBLE_LENGTH_CH1 - LORA_SFD_LENGTH_CH1 -  LORA_QUARTER_SFD_LENGTH_CH1];
		}
		else
		{
			while(1);
		}
	}
}


void LoRa_Generate_Signal_Sinpping(int * freq_points, int id_and_payload_symbol_len, float gap_width, uint32_t central_freq, uint32_t bw, uint8_t sf)
{
	/******** debug temps   ***********/
	
	/******** end of debug temps   ***********/
	
	float 	 lora_chip_step_freq	=	(float)bw / (float)((1 << sf));
	int 		 lora_symbol_duartion	=	(( 1 << sf ) << 3);
	
	uint32_t gap_position = lora_symbol_duartion*(1-gap_width);
	
	Channel_Coding_Convert_Snipping(freq_points, id_and_payload_symbol_len, central_freq, sf);
	Symbol_Start_End_Time_Cal(sf);
	
	int lora_bw_min_freq  = (central_freq - (bw >> 1)); // Hz
	int lora_bw_max_freq	=	(central_freq + (bw >> 1)); // Hz
	
	int chip_position = 0;

	uint32_t chirp_count_ch1 = 0;
	
	uint32_t total_chip_count = 0;
	uint32_t symbol_chip_count = 0;
	
	Init_Timer_Calibration_From_SX1276();
	
	Fast_SetChannel(central_freq);
	
	Send_packets:
	LL_GPIO_SetOutputPin(GPIOB,GPIO_PIN_5);
	SX1276SetOpMode( RF_OPMODE_TRANSMITTER );
	delay_ms(5);
	
	LL_TIM_EnableCounter(TIM3);
	LL_TIM_EnableCounter(TIM4);
	TIM3->CNT = 0;
	TIM4->CNT = 0;
	/*******************/
	while( chirp_count_ch1 < Lora_Packet_Symbol_Length_Ch1)
	{
		while(COMPENSATION_TIME <= Symbol_End_Time_Ch1[chirp_count_ch1])  // generate symbol
		{
			if(chirp_count_ch1 < LORA_PREAMBLE_LENGTH_CH1)
			{
				Chirp_Status_Ch1 = Preamble;
				chip_position = (COMPENSATION_TIME - Symbol_Start_Time_Ch1[chirp_count_ch1] ) >> 3;
				Input_Freq = LoRa_Start_Freq_Ch1[chirp_count_ch1] + (chip_position * lora_chip_step_freq);
			}
			else if(chirp_count_ch1 < LORA_PREAMBLE_LENGTH_CH1 + LORA_ID_LENGTH_CH1)
			{
				Chirp_Status_Ch1 = ID;
				chip_position = (COMPENSATION_TIME - Symbol_Start_Time_Ch1[chirp_count_ch1] ) >> 3;
				Input_Freq = LoRa_Start_Freq_Ch1[chirp_count_ch1] + (chip_position * (lora_chip_step_freq));
				if( Input_Freq > lora_bw_max_freq )
					Input_Freq = Input_Freq - bw;
			}
			else if(chirp_count_ch1 < LORA_PREAMBLE_LENGTH_CH1 + LORA_ID_LENGTH_CH1 + LORA_SFD_LENGTH_CH1)
			{
				Chirp_Status_Ch1 = SFD;
				chip_position = (COMPENSATION_TIME - Symbol_Start_Time_Ch1[chirp_count_ch1] ) >> 3;
				Input_Freq = LoRa_Start_Freq_Ch1[chirp_count_ch1] - (float)chip_position * (lora_chip_step_freq);
			}
			else if(chirp_count_ch1 < LORA_PREAMBLE_LENGTH_CH1 + LORA_ID_LENGTH_CH1 + LORA_SFD_LENGTH_CH1 + LORA_QUARTER_SFD_LENGTH_CH1)
			{
				Chirp_Status_Ch1 = Quarter_SFD;
				chip_position = (COMPENSATION_TIME - Symbol_Start_Time_Ch1[chirp_count_ch1] ) >> 3;
				Input_Freq = LoRa_Start_Freq_Ch1[chirp_count_ch1] - (float)chip_position * (lora_chip_step_freq);
			}
			else if(chirp_count_ch1 < Lora_Packet_Symbol_Length_Ch1)
			{
				Chirp_Status_Ch1 = Payload;
				chip_position = ( COMPENSATION_TIME - Symbol_Start_Time_Ch1[chirp_count_ch1] ) >> 3;
				Input_Freq = LoRa_Start_Freq_Ch1[chirp_count_ch1] + (float)(chip_position + 1) * lora_chip_step_freq;
				if( Input_Freq > lora_bw_max_freq )
					Input_Freq = Input_Freq - bw;
			}
			else 
				break;
			
			Fast_SetChannel( Input_Freq );

			while( COMPENSATION_TIME & ( 8 - 1 ));// chip time = 8us
			
			if((chirp_count_ch1>LORA_PREAMBLE_LENGTH_CH1+LORA_ID_LENGTH_CH1+LORA_SFD_LENGTH_CH1+LORA_QUARTER_SFD_LENGTH_CH1-1))
			{
				//end
				if(COMPENSATION_TIME > (Symbol_Start_Time_Ch1[chirp_count_ch1] +  gap_position))
				{
					LL_GPIO_ResetOutputPin(GPIOB,GPIO_PIN_5);
					if(sf != 7)
					{
						SX1276SetOpMode( RF_OPMODE_STANDBY );
						while(COMPENSATION_TIME < Symbol_End_Time_Ch1[chirp_count_ch1]-120);
						SX1276SetOpMode(RF_OPMODE_TRANSMITTER);
					}
					while(COMPENSATION_TIME < Symbol_End_Time_Ch1[chirp_count_ch1]-10);
					goto Symbol_End;
				}
			}
		
			Changed_Register_Count = 1;
		}	// end loop of symbol
		Symbol_End:
		LL_GPIO_SetOutputPin(GPIOB,GPIO_PIN_5);
		symbol_chip_count = 0;
		chirp_count_ch1++;
		chip_position = 0;
	}

	SX1276SetOpMode( RF_OPMODE_STANDBY );
	LL_GPIO_ResetOutputPin(GPIOB,GPIO_PIN_5);
	
	free(LoRa_Start_Freq_Ch1);
	free(Symbol_Start_Time_Ch1);
	free(Symbol_End_Time_Ch1);
	
	total_chip_count = 0;
	chirp_count_ch1 = 0;
	Chirp_Status_Ch1 = Preamble;

	TIM3->CNT = 0;
	TIM4->CNT = 0;
	LL_TIM_DisableCounter(TIM3);
	LL_TIM_DisableCounter(TIM4);
}

//*************************************************************/
//
//
//
//  Double throughtputs
//
//
//
//*************************************************************/



void Channel_Coding_Convert_Multiplexing(int *freq_points_ch1,int id_and_payload_symbol_len_ch1,int *freq_points_ch2,int id_and_payload_symbol_len_ch2,
																				int central_freq_ch1,int central_freq_ch2)
{
	Lora_Packet_Symbol_Length_Ch1	=	LORA_PREAMBLE_LENGTH_CH1 + LORA_ID_LENGTH_CH1 + \
																	LORA_SFD_LENGTH_CH1 + LORA_QUARTER_SFD_LENGTH_CH1 + \
																	id_and_payload_symbol_len_ch1;
	
	Lora_Packet_Symbol_Length_Ch2	=	LORA_PREAMBLE_LENGTH_CH2 + LORA_ID_LENGTH_CH2 + \
																	LORA_SFD_LENGTH_CH2 + LORA_QUARTER_SFD_LENGTH_CH2 + \
																	id_and_payload_symbol_len_ch2;
	
	LoRa_Start_Freq_Ch1 = malloc(Lora_Packet_Symbol_Length_Ch1 * sizeof(int));
	LoRa_Start_Freq_Ch2 = malloc(Lora_Packet_Symbol_Length_Ch2 * sizeof(int));
	
	for(int i=0; i< Lora_Packet_Symbol_Length_Ch1; i++)
	{
		if(i < LORA_PREAMBLE_LENGTH_CH1)
		{
			LoRa_Start_Freq_Ch1[i] = central_freq_ch1 - 62500;
		}
		else if(i < LORA_PREAMBLE_LENGTH_CH1 + LORA_ID_LENGTH_CH1)
		{
			LoRa_Start_Freq_Ch1[i] = central_freq_ch1 + freq_points_ch1[i - LORA_PREAMBLE_LENGTH_CH1];
		}
		else if(i < LORA_PREAMBLE_LENGTH_CH1 + LORA_ID_LENGTH_CH1 + LORA_SFD_LENGTH_CH1 + LORA_QUARTER_SFD_LENGTH_CH1)
		{
			LoRa_Start_Freq_Ch1[i] = central_freq_ch1 + 62500 - 500;
		}
		else if(i < Lora_Packet_Symbol_Length_Ch1)
		{
			LoRa_Start_Freq_Ch1[i] = central_freq_ch1 + freq_points_ch1[i - LORA_PREAMBLE_LENGTH_CH1 - LORA_SFD_LENGTH_CH1 -  LORA_QUARTER_SFD_LENGTH_CH1];
		}
		else
		{
			while(1);
		}
	}
	
	for(int i=0; i< Lora_Packet_Symbol_Length_Ch2; i++)
	{
		if(i < LORA_PREAMBLE_LENGTH_CH2)
		{
			LoRa_Start_Freq_Ch2[i] = central_freq_ch2 - 62500;
		}
		else if(i < LORA_PREAMBLE_LENGTH_CH2 + LORA_ID_LENGTH_CH2)
		{
			LoRa_Start_Freq_Ch2[i] = central_freq_ch2 + freq_points_ch2[i - LORA_PREAMBLE_LENGTH_CH2];
		}
		else if(i < LORA_PREAMBLE_LENGTH_CH2 + LORA_ID_LENGTH_CH2 + LORA_SFD_LENGTH_CH2 + LORA_QUARTER_SFD_LENGTH_CH2)
		{
			LoRa_Start_Freq_Ch2[i] = central_freq_ch2 + 62500 - 500;
		}
		else if(i < Lora_Packet_Symbol_Length_Ch2)
		{
			LoRa_Start_Freq_Ch2[i] = central_freq_ch2 + freq_points_ch2[i - LORA_PREAMBLE_LENGTH_CH2 - LORA_SFD_LENGTH_CH2 -  LORA_QUARTER_SFD_LENGTH_CH2];
		}
		else
		{
			while(1);
		}
	}
	
}

void Symbol_Start_End_Time_Cal_Multiplexing(uint8_t sf_ch1, uint8_t sf_ch2)
{
	int symbol_offset=0;
	/***********  packet no 1       *****************/
	Symbol_Start_Time_Ch1 = malloc(Lora_Packet_Symbol_Length_Ch1 * sizeof(uint32_t));
	Symbol_End_Time_Ch1 = malloc(Lora_Packet_Symbol_Length_Ch2 * sizeof(uint32_t));
	
	for(int i=0;i < Lora_Packet_Symbol_Length_Ch1;i++)
	{
		if(i< LORA_PREAMBLE_LENGTH_CH1 + LORA_ID_LENGTH_CH1 + LORA_SFD_LENGTH_CH1 + LORA_QUARTER_SFD_LENGTH_CH1)
			Symbol_Start_Time_Ch1[i] = ((i+symbol_offset)*(1<<sf_ch1))<<3;
		else
			Symbol_Start_Time_Ch1[i] = ((i+symbol_offset-1)*(1<<sf_ch1) + (1<<sf_ch1)/4) << 3;
	}
	Symbol_End_Time_Ch1 = Symbol_Start_Time_Ch1 + 1;
	
	/***********  packet no 2       *****************/
	Symbol_Start_Time_Ch2 = malloc(Lora_Packet_Symbol_Length_Ch2 * sizeof(uint32_t));
	Symbol_End_Time_Ch2 = malloc(Lora_Packet_Symbol_Length_Ch2 * sizeof(uint32_t));
	
	for(int i=0;i < Lora_Packet_Symbol_Length_Ch2;i++)
	{
		if(i< LORA_PREAMBLE_LENGTH_CH2 + LORA_ID_LENGTH_CH2 + LORA_SFD_LENGTH_CH2 + LORA_QUARTER_SFD_LENGTH_CH2)
			Symbol_Start_Time_Ch2[i] = (i*(1<<sf_ch2))<<3;
		else
			Symbol_Start_Time_Ch2[i] = ((i-1)*(1<<sf_ch2) + (1<<sf_ch2)/4) << 3;
	}
	Symbol_End_Time_Ch2 = Symbol_Start_Time_Ch2 + 1;
	
}



void LoRa_Generate_Double_Packet(int *freq_points_ch1, int id_and_payload_symbol_len_ch1, int *freq_points_ch2, int id_and_payload_symbol_len_ch2, 
																uint32_t central_freq_ch1, uint32_t central_freq_ch2, uint32_t bw_ch1, uint8_t bw_ch2, uint8_t sf_ch1, uint8_t sf_ch2)
{
	/******** debug temps   ***********/

	/******** end of debug temps   ***********/
	bool ch1_or_ch2=true;
	Channel_Coding_Convert_Multiplexing(freq_points_ch1,id_and_payload_symbol_len_ch1,freq_points_ch2,id_and_payload_symbol_len_ch2,central_freq_ch1,central_freq_ch2);
	Symbol_Start_End_Time_Cal_Multiplexing(sf_ch1, sf_ch2);
	
	float 	 lora_chip_step_freq_ch1	=	(float)bw_ch1 / (float)((1 << sf_ch1));
	float 	 lora_chip_step_freq_ch2	=	(float)bw_ch2 / (float)((1 << sf_ch2));
	
	int lora_bw_min_freq_ch1 =	(central_freq_ch1 - (bw_ch1 >> 1)); // Hz
	int lora_bw_max_freq_ch1 = (central_freq_ch1 + (bw_ch1 >> 1)); // Hz
	
	int lora_bw_min_freq_ch2 =	(central_freq_ch2 - (bw_ch2 >> 1)); // Hz
	int lora_bw_max_freq_ch2 =	(central_freq_ch2 + (bw_ch2 >> 1)); // Hz
	
	int chip_position_ch1 = 0;
	int chip_position_ch2 = 0;

	uint32_t chirp_count_ch1 = 0;
	uint32_t chirp_count_ch2 = 0;
	
	uint32_t total_chip_count = 0;
	uint32_t symbol_chip_count = 0;
	
	Init_Timer_Calibration_From_SX1276();
	
	Fast_SetChannel( central_freq_ch1 );
	
	Send_packets:
	LL_GPIO_SetOutputPin(GPIOB,GPIO_PIN_5);
 	SX1276SetOpMode( RF_OPMODE_TRANSMITTER );
	delay_ms(5);
	
	LL_TIM_EnableCounter(TIM3);
	LL_TIM_EnableCounter(TIM4);
	TIM3->CNT = 0;
	TIM4->CNT = 0;

	/*******************/
	while( chirp_count_ch1 < Lora_Packet_Symbol_Length_Ch1 || chirp_count_ch2 < Lora_Packet_Symbol_Length_Ch2)
	{

		while(COMPENSATION_TIME <= Symbol_End_Time_Ch1[chirp_count_ch1])  // generate symbol
		{
			
			if(!ch1_or_ch2) // false: channel 1, true: channel 2
			{
				if(chirp_count_ch1 < LORA_PREAMBLE_LENGTH_CH1)
				{
					Chirp_Status_Ch1 = Preamble;
					chip_position_ch1 = (COMPENSATION_TIME - Symbol_Start_Time_Ch1[chirp_count_ch1] ) >> 3;
					Input_Freq = LoRa_Start_Freq_Ch1[chirp_count_ch1] + (chip_position_ch1 * lora_chip_step_freq_ch1);
				}
				else if(chirp_count_ch1 < LORA_PREAMBLE_LENGTH_CH1 + LORA_ID_LENGTH_CH1)
				{
					Chirp_Status_Ch1 = ID;
					chip_position_ch1 = (COMPENSATION_TIME - Symbol_Start_Time_Ch1[chirp_count_ch1] ) >> 3;
					Input_Freq = LoRa_Start_Freq_Ch1[chirp_count_ch1] + (chip_position_ch1 * (lora_chip_step_freq_ch1));
					if( Input_Freq > lora_bw_max_freq_ch1 )
						Input_Freq = Input_Freq - bw_ch1;
				}
				else if(chirp_count_ch1 < LORA_PREAMBLE_LENGTH_CH1 + LORA_ID_LENGTH_CH1 + LORA_SFD_LENGTH_CH1)
				{
					Chirp_Status_Ch1 = SFD;
					chip_position_ch1 = (COMPENSATION_TIME - Symbol_Start_Time_Ch1[chirp_count_ch1] ) >> 3;
					Input_Freq = LoRa_Start_Freq_Ch1[chirp_count_ch1] - (float)chip_position_ch1 * (lora_chip_step_freq_ch1);
				}
				else if(chirp_count_ch1 < LORA_PREAMBLE_LENGTH_CH1 + LORA_ID_LENGTH_CH1 + LORA_SFD_LENGTH_CH1 + LORA_QUARTER_SFD_LENGTH_CH1)
				{
					Chirp_Status_Ch1 = Quarter_SFD;
					chip_position_ch1 = (COMPENSATION_TIME - Symbol_Start_Time_Ch1[chirp_count_ch1] ) >> 3;
					Input_Freq = LoRa_Start_Freq_Ch1[chirp_count_ch1] - (float)chip_position_ch1 * (lora_chip_step_freq_ch1);
				}
				else if(chirp_count_ch1 < Lora_Packet_Symbol_Length_Ch1)
				{
					Chirp_Status_Ch1 = Payload;
					chip_position_ch1 = ( COMPENSATION_TIME - Symbol_Start_Time_Ch1[chirp_count_ch1] ) >> 3;
					Input_Freq = LoRa_Start_Freq_Ch1[chirp_count_ch1] + (float)chip_position_ch1 * lora_chip_step_freq_ch1;
					if( Input_Freq > lora_bw_max_freq_ch1 )
						Input_Freq = Input_Freq - bw_ch1;
				}
				else 
					break;
			}
			else
			{
				if(chirp_count_ch2 < LORA_PREAMBLE_LENGTH_CH2)
				{
					Chirp_Status_Ch2 = Preamble;
					chip_position_ch2 = (COMPENSATION_TIME - Symbol_Start_Time_Ch2[chirp_count_ch2] ) >> 3;
					Input_Freq = LoRa_Start_Freq_Ch2[chirp_count_ch2] + (chip_position_ch2 * lora_chip_step_freq_ch2);
				}
				else if(chirp_count_ch2 < LORA_PREAMBLE_LENGTH_CH2 + LORA_ID_LENGTH_CH2)
				{
					Chirp_Status_Ch2 = ID;
					chip_position_ch2 = (COMPENSATION_TIME - Symbol_Start_Time_Ch2[chirp_count_ch2] ) >> 3;
					Input_Freq = LoRa_Start_Freq_Ch2[chirp_count_ch2] + (chip_position_ch2 * (lora_chip_step_freq_ch2));
					if( Input_Freq > lora_bw_max_freq_ch2 )
						Input_Freq = Input_Freq - bw_ch2;
				}
				else if(chirp_count_ch2 < LORA_PREAMBLE_LENGTH_CH2 + LORA_ID_LENGTH_CH2 + LORA_SFD_LENGTH_CH2)
				{
					Chirp_Status_Ch2 = SFD;
					chip_position_ch2 = (COMPENSATION_TIME - Symbol_Start_Time_Ch2[chirp_count_ch2] ) >> 3;
					Input_Freq = LoRa_Start_Freq_Ch2[chirp_count_ch2] - (float)chip_position_ch2 * (lora_chip_step_freq_ch2);
				}
				else if(chirp_count_ch2 < LORA_PREAMBLE_LENGTH_CH2 + LORA_ID_LENGTH_CH2 + LORA_SFD_LENGTH_CH2 + LORA_QUARTER_SFD_LENGTH_CH2)
				{
					Chirp_Status_Ch2 = Quarter_SFD;
					chip_position_ch2 = (COMPENSATION_TIME - Symbol_Start_Time_Ch2[chirp_count_ch2] ) >> 3;
					Input_Freq = LoRa_Start_Freq_Ch2[chirp_count_ch2] - (float)chip_position_ch2 * (lora_chip_step_freq_ch2);
				}
				else if(chirp_count_ch2 < Lora_Packet_Symbol_Length_Ch2)
				{
					Chirp_Status_Ch2 = Payload;
					chip_position_ch2 = ( COMPENSATION_TIME - Symbol_Start_Time_Ch2[chirp_count_ch2] ) >> 3;
					Input_Freq = LoRa_Start_Freq_Ch2[chirp_count_ch2] + (float)chip_position_ch2 * lora_chip_step_freq_ch2;
					if( Input_Freq > lora_bw_max_freq_ch2 )
						Input_Freq = Input_Freq - bw_ch2;
				}
				else 
					break;
			}
			
			Fast_SetChannel( Input_Freq );
			
			while( COMPENSATION_TIME & ( 8 - 1 ));// chip time = 8us
			
			if(COMPENSATION_TIME > (Symbol_Start_Time_Ch1[chirp_count_ch1] + Symbol_End_Time_Ch1[chirp_count_ch1])/2)
			{
				ch1_or_ch2 = true;
			}

			if( COMPENSATION_TIME >= Symbol_End_Time_Ch2[chirp_count_ch2])
			{
				chirp_count_ch2++;
			}
			
			total_chip_count++;
			symbol_chip_count++;
			Changed_Register_Count = 1;

		}	// end loop of symbol
		Symbol_End:
		
		symbol_chip_count = 0;
		if( COMPENSATION_TIME >= Symbol_End_Time_Ch1[chirp_count_ch1])
		{
			chirp_count_ch1++;
		}
		if( chirp_count_ch1 < Lora_Packet_Symbol_Length_Ch1)
			ch1_or_ch2 = false;
		else
			ch1_or_ch2 = true;
		chip_position_ch1 = 0;
		chip_position_ch2 = 0;
	
	}
	SX1276SetOpMode( RF_OPMODE_STANDBY );
	LL_GPIO_ResetOutputPin(GPIOB,GPIO_PIN_5);
	
	free(LoRa_Start_Freq_Ch1);
	free(Symbol_Start_Time_Ch1);
	free(Symbol_End_Time_Ch1);
	
	free(LoRa_Start_Freq_Ch2);
	free(Symbol_Start_Time_Ch2);
	free(Symbol_End_Time_Ch2);
	
	total_chip_count = 0;
	chirp_count_ch1 = 0;
	Chirp_Status_Ch1 = Preamble;

	TIM3->CNT = 0;
	TIM4->CNT = 0;
	LL_TIM_DisableCounter(TIM3);
	LL_TIM_DisableCounter(TIM4);
}



