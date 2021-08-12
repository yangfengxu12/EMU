#include "hw.h"
#include "radio.h"
#include "sx1276.h"
#include "sx1276mb1mas.h"
#include "delay.h"
#include "control_GPIO.h"

#include "Simulated_LoRa.h"

#include "Timer_Calibration_From_SX1276.h"
#define Comped_Time (( (uint16_t)TIM4->CNT << 16 ) | (uint16_t)TIM3->CNT ) 



float 	 LORA_FREQ_STEP_NO1		=											(float)LORA_BW / (float)((1 << LORA_SF_NO1));
int 		 LORA_SYMBOL_TIME_NO1	=											(( 1 << LORA_SF_NO1 ) << 3);
float 	 LORA_FREQ_STEP_NO2		=											(float)LORA_BW / (float)((1 << LORA_SF_NO2));
int 		 LORA_SYMBOL_TIME_NO2	=											(( 1 << LORA_SF_NO2 ) << 3);

int      LORA_PAYLOAD_LENGTH_NO1 = 									0;
int  		 LORA_TOTAL_LENGTH_NO1	 =									0;
int      LORA_PAYLOAD_LENGTH_NO2 = 									0;
int  		 LORA_TOTAL_LENGTH_NO2	 =									0;

enum Chirp_Status Chirp_Status_No1 = Preamble;
enum Chirp_Status Chirp_Status_No2 = Preamble;

int *LoRa_Start_Freq_No1;
int *LoRa_Start_Freq_No2;

uint32_t *Symbol_Start_Time_No1;
uint32_t *Symbol_End_Time_No1;
uint32_t *Symbol_Start_Time_No2;
uint32_t *Symbol_End_Time_No2;

uint8_t Channel_Freq_MSB_temp = 0;
uint8_t Channel_Freq_MID_temp = 0;
uint8_t Channel_Freq_LSB_temp = 0;

float Input_Freq;
uint32_t Channel;

uint8_t Channel_Freq[3] = {0};  //MSB,MID,LSB
uint8_t Changed_Register_Count = 1;  // the number of changed registers.

void Fast_SetChannel( uint8_t *freq, uint8_t Changed_Register_Count )
{
	uint8_t Reg_Address;
	switch(Changed_Register_Count)
	{
		case 1:Reg_Address = REG_FRFLSB;break;
		case 2:Reg_Address = REG_FRFMID;break;
		case 3:Reg_Address = REG_FRFMSB;break;
		default:Reg_Address = REG_FRFMSB;break;
	}
	SX1276_Burst_Write( Reg_Address, freq + 3 - Changed_Register_Count, Changed_Register_Count);
}

void symbol_start_end_time_cal()
{
	Symbol_Start_Time_No1 = malloc(LORA_TOTAL_LENGTH_NO1 * sizeof(uint32_t));
	Symbol_End_Time_No1 = malloc(LORA_TOTAL_LENGTH_NO1 * sizeof(uint32_t));
	
	for(int i=0;i < LORA_TOTAL_LENGTH_NO1;i++)
	{
		if(i< LORA_PREAMBLE_LENGTH_NO1 + LORA_ID_LENGTH_NO1 + LORA_SFD_LENGTH_NO1 + LORA_QUARTER_SFD_LENGTH_NO1)
			Symbol_Start_Time_No1[i] = (i*(1<<LORA_SF_NO1))<<3;
		else
			Symbol_Start_Time_No1[i] = ((i-1)*(1<<LORA_SF_NO1) + (1<<LORA_SF_NO1)/4) << 3;
	}
	Symbol_End_Time_No1 = Symbol_Start_Time_No1 + 1;
}

void channel_coding_convert(int * freq_points,int id_and_payload_symbol_len)
{
  LORA_TOTAL_LENGTH_NO1			=	LORA_PREAMBLE_LENGTH_NO1 + LORA_ID_LENGTH_NO1 + \
															LORA_SFD_LENGTH_NO1 + LORA_QUARTER_SFD_LENGTH_NO1 + \
															id_and_payload_symbol_len;
	
	LoRa_Start_Freq_No1 = malloc(LORA_TOTAL_LENGTH_NO1 * sizeof(int));
	
	for(int i=0; i< LORA_TOTAL_LENGTH_NO1; i++)
	{
		if(i < LORA_PREAMBLE_LENGTH_NO1)
		{
			LoRa_Start_Freq_No1[i] = RF_FREQUENCY - 62500;
		}
		else if(i < LORA_PREAMBLE_LENGTH_NO1 + LORA_ID_LENGTH_NO1)
		{
			LoRa_Start_Freq_No1[i] = RF_FREQUENCY + freq_points[i - LORA_PREAMBLE_LENGTH_NO1];
		}
		else if(i < LORA_PREAMBLE_LENGTH_NO1 + LORA_ID_LENGTH_NO1 + LORA_SFD_LENGTH_NO1 + LORA_QUARTER_SFD_LENGTH_NO1)
		{
			LoRa_Start_Freq_No1[i] = RF_FREQUENCY + 62500 - 500;
		}
		else if(i < LORA_TOTAL_LENGTH_NO1)
		{
			LoRa_Start_Freq_No1[i] = RF_FREQUENCY + freq_points[i - LORA_PREAMBLE_LENGTH_NO1 - LORA_SFD_LENGTH_NO1 -  LORA_QUARTER_SFD_LENGTH_NO1];
		}
		else
		{
			while(1);
		}
	}
}

void blank_position_cal(uint8_t sf, int freq, int bw, uint16_t *start_p1, uint16_t *end_p1, uint16_t *start_p2, uint16_t *end_p2)
{
//	int16_t total_chip = 1<<sf;
//	int start_reserver_distance = (int)(0.50 * total_chip);
//	int end_reserver_distance = (int)(0.00 * total_chip);
//	int turn_reserver_distance = (int)(0.00 * total_chip);
//	int blank_width = (int)(0.5 * total_chip);
//	
//	float distance_to_max_freq = ((bw>>1)-freq)/(bw/(1<<sf));
//	float distance_to_symbol_end = (1<<sf)-distance_to_max_freq;
	
	int start_blank_position_1 = 0;
	int end_blank_position_1 = 0;
	
	int start_blank_position_2 = 0;
	int end_blank_position_2 = 0;
	
//	if(distance_to_max_freq > start_reserver_distance + turn_reserver_distance)
//	{
//		start_blank_position_1 = start_reserver_distance;
//		if(distance_to_max_freq - start_reserver_distance - turn_reserver_distance > blank_width)
//		{
//			end_blank_position_1 = blank_width + start_reserver_distance;
//			
//		}
//		else
//		{
//			end_blank_position_1 = distance_to_max_freq - turn_reserver_distance;
//			start_blank_position_2 = distance_to_max_freq + turn_reserver_distance;
//			end_blank_position_2 = start_blank_position_2 + blank_width - (end_blank_position_1 - start_blank_position_1);
//		}
//	}
//	else
//	{
//		start_blank_position_1 = distance_to_max_freq + turn_reserver_distance;
//		end_blank_position_1 = start_blank_position_1 + blank_width;
//	}
	start_blank_position_1 = (1<<sf)/2;
	end_blank_position_1 = (1<<sf);
	
	start_blank_position_1 = 0;
	end_blank_position_1 = 0;
	
	start_blank_position_2 = 0;
	end_blank_position_2 = 0;
	
	*start_p1 = (uint16_t) start_blank_position_1;
	*end_p1   = (uint16_t) end_blank_position_1;
	*start_p2 = (uint16_t) start_blank_position_2;
	*end_p2   = (uint16_t) end_blank_position_2;
}


void LoRa_Generate_Signal(int * freq_points, int id_and_payload_symbol_len)
{
	uint16_t start_p1,end_p1,start_p2,end_p2;
	channel_coding_convert(freq_points,id_and_payload_symbol_len);
	symbol_start_end_time_cal();
	
	/******** debug temps   ***********/
//	uint32_t Chip_Count_No1[LORA_TOTAL_LENGTH_NO1] = {0};
//	uint32_t *Chirp_Time_Record_No1 = malloc(LORA_TOTAL_LENGTH_NO1 * sizeof(uint32_t));
	uint32_t *Chip_Count_Record_No1 = malloc(LORA_TOTAL_LENGTH_NO1 * sizeof(uint32_t));
	
//	float *Freq_Record_NO1 = malloc((1<<LORA_SF_NO1) * sizeof(float));
//	uint32_t *Chip_Time_Record_NO1 = malloc( (1<< LORA_SF_NO1) * sizeof(uint32_t));
	
//	float *Max_Freq_In_Symbol = malloc(LORA_TOTAL_LENGTH_NO1 * sizeof(float));
//	float *Min_Freq_In_Symbol = malloc(LORA_TOTAL_LENGTH_NO1 * sizeof(float));
//	memset(Max_Freq_In_Symbol, 0, LORA_TOTAL_LENGTH_NO1 * sizeof(float));
//	memset(Min_Freq_In_Symbol, 1000000000, LORA_TOTAL_LENGTH_NO1 * sizeof(float));
	
	
	
	int Chip_Position_No1 = 0;
	int Chip_Mid_KeyPoint_No1 = 0;

	uint32_t Chirp_Count_No1 = 0;

//	int Init_Frequency_Begin_Point_No1 = LORA_BASE_FREQ;
//	int Init_Frequency_End_Point_No1 = LORA_MAX_FREQ;

//	
//	uint32_t Next_Init_Frequency_Begin_Point_No1 = LORA_BASE_FREQ;
//	uint32_t Next_Init_Frequency_End_Point_No1 = LORA_MAX_FREQ;

	uint32_t Total_Chip_Count = 0;
	uint32_t Symbol_Chip_Count = 0;
	
	Init_Timer_Calibration_From_SX1276();
	
	SX_FREQ_TO_CHANNEL( Channel, RF_FREQUENCY );
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
	else if( Channel_Freq_MID_temp != Channel_Freq[2] )
	{
		Changed_Register_Count = 1;
		Channel_Freq_LSB_temp = Channel_Freq[2];
	}
	Fast_SetChannel( Channel_Freq, Changed_Register_Count );
	
	
	Send_packets:
	LL_GPIO_SetOutputPin(GPIOB,GPIO_PIN_5);
 	SX1276SetOpMode( RF_OPMODE_TRANSMITTER );
	delay_ms(5);
	
	LL_TIM_EnableCounter(TIM3);
	LL_TIM_EnableCounter(TIM4);
	TIM3->CNT = 0;
	TIM4->CNT = 0;

	
	/*******************/
// 	LL_GPIO_TogglePin(GPIOB,GPIO_PIN_2);
//	LL_GPIO_TogglePin(GPIOB,GPIO_PIN_11);
//	LL_GPIO_TogglePin(GPIOB,GPIO_PIN_12);
	
	while( Chirp_Count_No1 < LORA_TOTAL_LENGTH_NO1 )
	{
		while(Comped_Time <= Symbol_End_Time_No1[Chirp_Count_No1])  // generate symbol
		{
			if(Chirp_Count_No1 < LORA_PREAMBLE_LENGTH_NO1)
			{
				Chirp_Status_No1 = Preamble;
				Chip_Position_No1 = (Comped_Time - Symbol_Start_Time_No1[Chirp_Count_No1] ) >> 3;
				Input_Freq = LoRa_Start_Freq_No1[Chirp_Count_No1] + (Chip_Position_No1 * LORA_FREQ_STEP_NO1);
			}
			else if(Chirp_Count_No1 < LORA_PREAMBLE_LENGTH_NO1 + LORA_ID_LENGTH_NO1)
			{
				Chirp_Status_No1 = ID;
				Chip_Position_No1 = (Comped_Time - Symbol_Start_Time_No1[Chirp_Count_No1] ) >> 3;
				Input_Freq = LoRa_Start_Freq_No1[Chirp_Count_No1] + (Chip_Position_No1 * (LORA_FREQ_STEP_NO1));
				if( Input_Freq > LORA_MAX_FREQ_NO1 )
				{
					Input_Freq = Input_Freq - LORA_BW;
				}
			}
			else if(Chirp_Count_No1 < LORA_PREAMBLE_LENGTH_NO1 + LORA_ID_LENGTH_NO1 + LORA_SFD_LENGTH_NO1)
			{
				Chirp_Status_No1 = SFD;
				Chip_Position_No1 = (Comped_Time - Symbol_Start_Time_No1[Chirp_Count_No1] ) >> 3;
				Input_Freq = LoRa_Start_Freq_No1[Chirp_Count_No1] - (float)Chip_Position_No1 * (LORA_FREQ_STEP_NO1);
			}
			else if(Chirp_Count_No1 < LORA_PREAMBLE_LENGTH_NO1 + LORA_ID_LENGTH_NO1 + LORA_SFD_LENGTH_NO1 + LORA_QUARTER_SFD_LENGTH_NO1)
			{
				Chirp_Status_No1 = Quarter_SFD;
				Chip_Position_No1 = (Comped_Time - Symbol_Start_Time_No1[Chirp_Count_No1] ) >> 3;
				Input_Freq = LoRa_Start_Freq_No1[Chirp_Count_No1] - (float)Chip_Position_No1 * (LORA_FREQ_STEP_NO1);
			}
			else if(Chirp_Count_No1 < LORA_TOTAL_LENGTH_NO1)
			{
				Chirp_Status_No1 = Payload;
				Chip_Position_No1 = ( Comped_Time - Symbol_Start_Time_No1[Chirp_Count_No1] ) >> 3;
				Input_Freq = LoRa_Start_Freq_No1[Chirp_Count_No1] + (float)Chip_Position_No1 * LORA_FREQ_STEP_NO1;
				if( Input_Freq > LORA_MAX_FREQ_NO1 )
				{
					Input_Freq = Input_Freq - LORA_BW;
				}
			}
			else
			{
				while(1);
			}	
			
			if(Chirp_Status_No1 == Payload && Chip_Position_No1 > (1<<LORA_SF_NO1)*10/10)
			{
				SX_FREQ_TO_CHANNEL( Channel, (uint32_t)RF_FREQUENCY);
			}
			else			
			{
				SX_FREQ_TO_CHANNEL( Channel, (uint32_t)Input_Freq );
			}
			
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
			else if( Channel_Freq_MID_temp != Channel_Freq[2] )
			{
				Changed_Register_Count = 1;
				Channel_Freq_LSB_temp = Channel_Freq[2];
			}
			
			Fast_SetChannel( Channel_Freq, Changed_Register_Count );

			while( Comped_Time & ( 8 - 1 ));// chip time = 8us
			
			Total_Chip_Count++;
			Symbol_Chip_Count++;
			Changed_Register_Count = 1;

		}	// end loop of symbol
		Symbol_End:
		Chip_Count_Record_No1[Chirp_Count_No1] = Symbol_Chip_Count;
		Symbol_Chip_Count = 0;
		Chirp_Count_No1++;
		Chip_Position_No1 = 0;
//		LL_GPIO_TogglePin(GPIOB,GPIO_PIN_2);	
	}
	/*******************/
//	temp1 = TIM3->CNT;
//	temp2 = TIM4->CNT;

	SX1276SetOpMode( RF_OPMODE_SLEEP );
	LL_GPIO_ResetOutputPin(GPIOB,GPIO_PIN_5);
	
	free(LoRa_Start_Freq_No1);
	free(Chip_Count_Record_No1);
	free(Symbol_Start_Time_No1);
	free(Symbol_End_Time_No1);
	
	Total_Chip_Count = 0;
	Chirp_Count_No1 = 0;
	Chirp_Status_No1 = Preamble;

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
void channel_coding_convert_double_packets(int* freq_points_no1,int id_and_payload_symbol_len_no1,int* freq_points_no2,int id_and_payload_symbol_len_no2)
{
	LORA_TOTAL_LENGTH_NO1			=	LORA_PREAMBLE_LENGTH_NO1 + LORA_ID_LENGTH_NO1 + \
															LORA_SFD_LENGTH_NO1 + LORA_QUARTER_SFD_LENGTH_NO1 + \
															id_and_payload_symbol_len_no1;
	
	LORA_TOTAL_LENGTH_NO2			=	LORA_PREAMBLE_LENGTH_NO2 + LORA_ID_LENGTH_NO2 + \
															LORA_SFD_LENGTH_NO2 + LORA_QUARTER_SFD_LENGTH_NO2 + \
															id_and_payload_symbol_len_no2;
	
	LoRa_Start_Freq_No1 = malloc(LORA_TOTAL_LENGTH_NO1 * sizeof(int));
	LoRa_Start_Freq_No2 = malloc(LORA_TOTAL_LENGTH_NO2 * sizeof(int));
	
	for(int i=0; i< LORA_TOTAL_LENGTH_NO1; i++)
	{
		if(i < LORA_PREAMBLE_LENGTH_NO1)
		{
			LoRa_Start_Freq_No1[i] = RF_FREQUENCY_NO1 - 62500;
		}
		else if(i < LORA_PREAMBLE_LENGTH_NO1 + LORA_ID_LENGTH_NO1)
		{
			LoRa_Start_Freq_No1[i] = RF_FREQUENCY_NO1 + freq_points_no1[i - LORA_PREAMBLE_LENGTH_NO1];
		}
		else if(i < LORA_PREAMBLE_LENGTH_NO1 + LORA_ID_LENGTH_NO1 + LORA_SFD_LENGTH_NO1 + LORA_QUARTER_SFD_LENGTH_NO1)
		{
			LoRa_Start_Freq_No1[i] = RF_FREQUENCY_NO1 + 62500 - 500;
		}
		else if(i < LORA_TOTAL_LENGTH_NO1)
		{
			LoRa_Start_Freq_No1[i] = RF_FREQUENCY_NO1 + freq_points_no1[i - LORA_PREAMBLE_LENGTH_NO1 - LORA_SFD_LENGTH_NO1 -  LORA_QUARTER_SFD_LENGTH_NO1];
		}
		else
		{
			while(1);
		}
	}
	
	for(int i=0; i< LORA_TOTAL_LENGTH_NO2; i++)
	{
		if(i < LORA_PREAMBLE_LENGTH_NO2)
		{
			LoRa_Start_Freq_No2[i] = RF_FREQUENCY_NO2 - 62500;
		}
		else if(i < LORA_PREAMBLE_LENGTH_NO2 + LORA_ID_LENGTH_NO2)
		{
			LoRa_Start_Freq_No2[i] = RF_FREQUENCY_NO2 + freq_points_no2[i - LORA_PREAMBLE_LENGTH_NO2];
		}
		else if(i < LORA_PREAMBLE_LENGTH_NO2 + LORA_ID_LENGTH_NO2 + LORA_SFD_LENGTH_NO2 + LORA_QUARTER_SFD_LENGTH_NO2)
		{
			LoRa_Start_Freq_No2[i] = RF_FREQUENCY_NO2 + 62500 - 500;
		}
		else if(i < LORA_TOTAL_LENGTH_NO2)
		{
			LoRa_Start_Freq_No2[i] = RF_FREQUENCY_NO2 + freq_points_no2[i - LORA_PREAMBLE_LENGTH_NO2 - LORA_SFD_LENGTH_NO2 -  LORA_QUARTER_SFD_LENGTH_NO2];
		}
		else
		{
			while(1);
		}
	}
	
}

void symbol_start_end_time_cal_double_packets()
{
	/***********  packet no 1       *****************/
	Symbol_Start_Time_No1 = malloc(LORA_TOTAL_LENGTH_NO1 * sizeof(uint32_t));
	Symbol_End_Time_No1 = malloc(LORA_TOTAL_LENGTH_NO1 * sizeof(uint32_t));
	
	for(int i=0;i < LORA_TOTAL_LENGTH_NO1;i++)
	{
		if(i< LORA_PREAMBLE_LENGTH_NO1 + LORA_ID_LENGTH_NO1 + LORA_SFD_LENGTH_NO1 + LORA_QUARTER_SFD_LENGTH_NO1)
			Symbol_Start_Time_No1[i] = (i*(1<<LORA_SF_NO1))<<3;
		else
			Symbol_Start_Time_No1[i] = ((i-1)*(1<<LORA_SF_NO1) + (1<<LORA_SF_NO1)/4) << 3;
	}
	Symbol_End_Time_No1 = Symbol_Start_Time_No1 + 1;
	
	/***********  packet no 2       *****************/
	Symbol_Start_Time_No2 = malloc(LORA_TOTAL_LENGTH_NO2 * sizeof(uint32_t));
	Symbol_End_Time_No2 = malloc(LORA_TOTAL_LENGTH_NO2 * sizeof(uint32_t));
	
	for(int i=0;i < LORA_TOTAL_LENGTH_NO2;i++)
	{
//		Symbol_Start_Time_No2[i] = (Symbol_Start_Time_No1[i] + Symbol_End_Time_No1[i])>>1;
		if(i< LORA_PREAMBLE_LENGTH_NO2 + LORA_ID_LENGTH_NO2 + LORA_SFD_LENGTH_NO2 + LORA_QUARTER_SFD_LENGTH_NO2)
			Symbol_Start_Time_No2[i] = (i*(1<<LORA_SF_NO2))<<3;
		else
			Symbol_Start_Time_No2[i] = ((i-1)*(1<<LORA_SF_NO2) + (1<<LORA_SF_NO2)/4) << 3;
	}
	Symbol_End_Time_No2 = Symbol_Start_Time_No2 + 1;
	

//	for(int i=0;i < LORA_TOTAL_LENGTH_NO1;i++)
//	{
//		Symbol_Start_Time_No1[i] += Symbol_End_Time_No2[ LORA_PREAMBLE_LENGTH_NO2 + LORA_ID_LENGTH_NO2 + LORA_SFD_LENGTH_NO2 + LORA_QUARTER_SFD_LENGTH_NO2 ];
//	}
}



void LoRa_Generate_Double_Packet(int * freq_points_No1, int id_and_payload_symbol_len_No1, int * freq_points_No2, int id_and_payload_symbol_len_No2)
{
	bool No1_or_No2=true;
	channel_coding_convert_double_packets(freq_points_No1,id_and_payload_symbol_len_No1,freq_points_No2,id_and_payload_symbol_len_No2);
	symbol_start_end_time_cal_double_packets();
	
	/******** debug temps   ***********/
	
	
	/******** end of debug temps   ***********/
	int Chip_Position_No1 = 0;
	int Chip_Position_No2 = 0;

	uint32_t Chirp_Count_No1 = 0;
	uint32_t Chirp_Count_No2 = 0;
	
	uint32_t Total_Chip_Count = 0;
	uint32_t Symbol_Chip_Count = 0;
	
	Init_Timer_Calibration_From_SX1276();
	
	SX_FREQ_TO_CHANNEL( Channel, RF_FREQUENCY );
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
	else if( Channel_Freq_MID_temp != Channel_Freq[2] )
	{
		Changed_Register_Count = 1;
		Channel_Freq_LSB_temp = Channel_Freq[2];
	}
	Fast_SetChannel( Channel_Freq, Changed_Register_Count );
	
	
	Send_packets:
	LL_GPIO_SetOutputPin(GPIOB,GPIO_PIN_5);
 	SX1276SetOpMode( RF_OPMODE_TRANSMITTER );
	delay_ms(5);
	
	LL_TIM_EnableCounter(TIM3);
	LL_TIM_EnableCounter(TIM4);
	TIM3->CNT = 0;
	TIM4->CNT = 0;
	
	/*******************/
// 	LL_GPIO_TogglePin(GPIOB,GPIO_PIN_2);
//	LL_GPIO_TogglePin(GPIOB,GPIO_PIN_11);
//	LL_GPIO_TogglePin(GPIOB,GPIO_PIN_12);
	
	while( Chirp_Count_No1 < LORA_TOTAL_LENGTH_NO1 || Chirp_Count_No2 < LORA_TOTAL_LENGTH_NO2)
	{

		while(Comped_Time <= Symbol_End_Time_No1[Chirp_Count_No1])  // generate symbol
		{
			
			if(!No1_or_No2) // false:No1, true: No2
			{
				if(Chirp_Count_No1 < LORA_PREAMBLE_LENGTH_NO1)
				{
					Chirp_Status_No1 = Preamble;
					Chip_Position_No1 = (Comped_Time - Symbol_Start_Time_No1[Chirp_Count_No1] ) >> 3;
					Input_Freq = LoRa_Start_Freq_No1[Chirp_Count_No1] + (Chip_Position_No1 * LORA_FREQ_STEP_NO1);
				}
				else if(Chirp_Count_No1 < LORA_PREAMBLE_LENGTH_NO1 + LORA_ID_LENGTH_NO1)
				{
					Chirp_Status_No1 = ID;
					Chip_Position_No1 = (Comped_Time - Symbol_Start_Time_No1[Chirp_Count_No1] ) >> 3;
					Input_Freq = LoRa_Start_Freq_No1[Chirp_Count_No1] + (Chip_Position_No1 * (LORA_FREQ_STEP_NO1));
					if( Input_Freq > LORA_MAX_FREQ_NO1 )
						Input_Freq = Input_Freq - LORA_BW;
				}
				else if(Chirp_Count_No1 < LORA_PREAMBLE_LENGTH_NO1 + LORA_ID_LENGTH_NO1 + LORA_SFD_LENGTH_NO1)
				{
					Chirp_Status_No1 = SFD;
					Chip_Position_No1 = (Comped_Time - Symbol_Start_Time_No1[Chirp_Count_No1] ) >> 3;
					Input_Freq = LoRa_Start_Freq_No1[Chirp_Count_No1] - (float)Chip_Position_No1 * (LORA_FREQ_STEP_NO1);
				}
				else if(Chirp_Count_No1 < LORA_PREAMBLE_LENGTH_NO1 + LORA_ID_LENGTH_NO1 + LORA_SFD_LENGTH_NO1 + LORA_QUARTER_SFD_LENGTH_NO1)
				{
					Chirp_Status_No1 = Quarter_SFD;
					Chip_Position_No1 = (Comped_Time - Symbol_Start_Time_No1[Chirp_Count_No1] ) >> 3;
					Input_Freq = LoRa_Start_Freq_No1[Chirp_Count_No1] - (float)Chip_Position_No1 * (LORA_FREQ_STEP_NO1);
				}
				else if(Chirp_Count_No1 < LORA_TOTAL_LENGTH_NO1)
				{
					Chirp_Status_No1 = Payload;
					Chip_Position_No1 = ( Comped_Time - Symbol_Start_Time_No1[Chirp_Count_No1] ) >> 3;
					Input_Freq = LoRa_Start_Freq_No1[Chirp_Count_No1] + (float)Chip_Position_No1 * LORA_FREQ_STEP_NO1;
					if( Input_Freq > LORA_MAX_FREQ_NO1 )
						Input_Freq = Input_Freq - LORA_BW;
				}
				else 
					break;
			}
			else
			{
				if(Chirp_Count_No2 < LORA_PREAMBLE_LENGTH_NO2)
				{
					Chirp_Status_No2 = Preamble;
					Chip_Position_No2 = (Comped_Time - Symbol_Start_Time_No2[Chirp_Count_No2] ) >> 3;
					Input_Freq = LoRa_Start_Freq_No2[Chirp_Count_No2] + (Chip_Position_No2 * LORA_FREQ_STEP_NO2);
				}
				else if(Chirp_Count_No2 < LORA_PREAMBLE_LENGTH_NO2 + LORA_ID_LENGTH_NO2)
				{
					Chirp_Status_No2 = ID;
					Chip_Position_No2 = (Comped_Time - Symbol_Start_Time_No2[Chirp_Count_No2] ) >> 3;
					Input_Freq = LoRa_Start_Freq_No2[Chirp_Count_No2] + (Chip_Position_No2 * (LORA_FREQ_STEP_NO2));
					if( Input_Freq > LORA_MAX_FREQ_NO2 )
						Input_Freq = Input_Freq - LORA_BW;
				}
				else if(Chirp_Count_No2 < LORA_PREAMBLE_LENGTH_NO2 + LORA_ID_LENGTH_NO2 + LORA_SFD_LENGTH_NO2)
				{
					Chirp_Status_No2 = SFD;
					Chip_Position_No2 = (Comped_Time - Symbol_Start_Time_No2[Chirp_Count_No2] ) >> 3;
					Input_Freq = LoRa_Start_Freq_No2[Chirp_Count_No2] - (float)Chip_Position_No2 * (LORA_FREQ_STEP_NO2);
				}
				else if(Chirp_Count_No2 < LORA_PREAMBLE_LENGTH_NO2 + LORA_ID_LENGTH_NO2 + LORA_SFD_LENGTH_NO2 + LORA_QUARTER_SFD_LENGTH_NO2)
				{
					Chirp_Status_No2 = Quarter_SFD;
					Chip_Position_No2 = (Comped_Time - Symbol_Start_Time_No2[Chirp_Count_No2] ) >> 3;
					Input_Freq = LoRa_Start_Freq_No2[Chirp_Count_No2] - (float)Chip_Position_No2 * (LORA_FREQ_STEP_NO2);
				}
				else if(Chirp_Count_No2 < LORA_TOTAL_LENGTH_NO2)
				{
					Chirp_Status_No2 = Payload;
					Chip_Position_No2 = ( Comped_Time - Symbol_Start_Time_No2[Chirp_Count_No2] ) >> 3;
					Input_Freq = LoRa_Start_Freq_No2[Chirp_Count_No2] + (float)Chip_Position_No2 * LORA_FREQ_STEP_NO2;
					if( Input_Freq > LORA_MAX_FREQ_NO2 )
						Input_Freq = Input_Freq - LORA_BW;
				}
				else 
					break;
			}
			
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
			else if( Channel_Freq_MID_temp != Channel_Freq[2] )
			{
				Changed_Register_Count = 1;
				Channel_Freq_LSB_temp = Channel_Freq[2];
			}
			
			Fast_SetChannel( Channel_Freq, Changed_Register_Count );

			while( Comped_Time & ( 8 - 1 ));// chip time = 8us
			
			if(Comped_Time > (Symbol_Start_Time_No1[Chirp_Count_No1] + Symbol_End_Time_No1[Chirp_Count_No1])/2)
			{
				No1_or_No2 = true;
			}

			if( Comped_Time >= Symbol_End_Time_No2[Chirp_Count_No2])
			{
				Chirp_Count_No2++;
			}
			
			Total_Chip_Count++;
			Symbol_Chip_Count++;
			Changed_Register_Count = 1;

		}	// end loop of symbol
		Symbol_End:
		
		Symbol_Chip_Count = 0;
		if( Comped_Time >= Symbol_End_Time_No1[Chirp_Count_No1])
		{
			Chirp_Count_No1++;
		}
		if( Chirp_Count_No1 < LORA_TOTAL_LENGTH_NO1)
			No1_or_No2 = false;
		else
			No1_or_No2 = true;
		Chip_Position_No1 = 0;
		Chip_Position_No2 = 0;
//		LL_GPIO_TogglePin(GPIOB,GPIO_PIN_2);	
	}

	SX1276SetOpMode( RF_OPMODE_SLEEP );
	LL_GPIO_ResetOutputPin(GPIOB,GPIO_PIN_5);
	
	free(LoRa_Start_Freq_No1);
	free(Symbol_Start_Time_No1);
	free(Symbol_End_Time_No1);
	
	free(LoRa_Start_Freq_No2);
	free(Symbol_Start_Time_No2);
	free(Symbol_End_Time_No2);
	
	Total_Chip_Count = 0;
	Chirp_Count_No1 = 0;
	Chirp_Status_No1 = Preamble;

	TIM3->CNT = 0;
	TIM4->CNT = 0;
	LL_TIM_DisableCounter(TIM3);
	LL_TIM_DisableCounter(TIM4);
}



