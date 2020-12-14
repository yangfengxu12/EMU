#include "hw.h"
#include "radio.h"
#include "sx1276.h"
#include "sx1276mb1mas.h"
#include "delay.h"
#include "Timer_Calibration.h"
#include "control_GPIO.h"

#include "Simulated_LoRa.h"


#define Comped_Time ( TIM2->CNT + Timer_Compensation_Count ) 

uint8_t test_symbol_point=1;

uint32_t LORA_FREQ_STEP_NO1		=											( LORA_BW / ( 1 << LORA_SF_NO1 ));
uint32_t LORA_SYMBOL_TIME_NO1	=											(( 1 << LORA_SF_NO1 ) << 3);

uint32_t LORA_FREQ_STEP_NO2		=											( LORA_BW / ( 1 << LORA_SF_NO2 ));
uint32_t LORA_SYMBOL_TIME_NO2	=											(( 1 << LORA_SF_NO2 ) << 3);


//sf = 7 payload =1 
int LoRa_ID_Start_Freq_No1[LORA_ID_LENGTH_NO1] = {-54695,-46890};
int LoRa_Payload_Start_Freq_No1[] = {
-46890,
-62500,
-62500,
46768,
-31280,
31158,
-7865,
31158,
53597,
54573,
30182,
-31280,
-31280,
-4939,
-53719,
-54695,
};

//sf = 8 payload = 1 
//int LoRa_ID_Start_Freq_No1[LORA_ID_LENGTH_NO1] = {-58597,-54695};
//int LoRa_Payload_Start_Freq_No1[] = {
//-54695,
//-2012,
//-33231,
//52621,
//-31280,
//33109,
//-7865,
//-35183,
//-32743,
//-61,
//-23963,
//-46890,
//-46890,
//-58109,
//57987,
//-60548
//};

// sf =11 payload =1 
//int LoRa_ID_Start_Freq_No2[LORA_ID_LENGTH_NO2] = {-62011,-61523};
//int LoRa_Payload_Start_Freq_No2[] = {
//-7083,
//-27345,
//-58594,
//60295,
//-37355,
//33441,
//-53223,
//-35646,
//3597,
//46746,
//8662,
//61088,
//-60364,
//-61706,
//-16543,
//-52307};

//sf = 8 payload = lorawan
int LoRa_ID_Start_Freq_No2[LORA_ID_LENGTH_NO2] = {-50787,-46882};
int LoRa_Payload_Start_Freq_No2[] = {
13634,
17539,
31204,
-15647,
-56643,
42917,
60486,
-25408,
-24432,
56582,
-51275,
27787,
44869,
37548,
59998,
-4910,
-41026.,
-35169,
27299,
-41026,
-39562,
58534,
-3446,
57070,
-7351,
-34193,
34132,
5826
};

// sf =9 payload =lorawan
//int LoRa_ID_Start_Freq_No2[LORA_ID_LENGTH_NO2] = {-59570,-58594,};
//int LoRa_Payload_Start_Freq_No2[] = {
//19033,
//-22465,
//54673,
//-4401,
//-61035,
//43444,
//61508,
//-24418,
//44664,
//37951,
//27821,
//29285,
//2067,
//-32840,
//-11847,
//21474,
//49546,
//-11236,
//44664,
//-37600,
//25624,
//41857,
//-21611,
//50157,
//-45900,
//12319,
//-15752,
//-53467
//};

uint8_t Channel_Freq_MSB_temp = 0;
uint8_t Channel_Freq_MID_temp = 0;
uint8_t Channel_Freq_LSB_temp = 0;

uint32_t Input_Freq;
uint32_t Channel;


//uint32_t Time_temp[ LORA_TOTAL_LENGTH_NO2 * ( 1 << LORA_SF_NO2 ) ]={0};
//uint32_t Time_temp1[ LORA_TOTAL_LENGTH_NO2 * ( 1 << LORA_SF_NO2 ) ]={0};
//uint32_t Time_temp2[ LORA_TOTAL_LENGTH_NO2 * ( 1 << LORA_SF_NO2 ) ]={0};

//uint32_t n_temp[ 1<<LORA_SF_NO2 ]={0};

uint8_t Channel_Freq[3] = {0};  //MSB,MID,LSB
uint8_t Changed_Register_Count = 1;  // the number of changed registers.

//uint32_t Input_Freq_temp[ LORA_TOTAL_LENGTH_NO2 * ( 1 << LORA_SF_NO2 )]={0};
//uint32_t Input_Freq_temp1[ LORA_TOTAL_LENGTH_NO2 * ( 1 << LORA_SF_NO2 )]={0};
//uint32_t Input_Freq_temp_No2[ LORA_TOTAL_LENGTH_NO2 ][( 1 << LORA_SF_NO2 )]={0};

int Timer_Compensation_Index = 0;
int Timer_Compensation_Flag = 0;
int Timer_Compensation_Flag_Last = 0;

uint8_t temp_u8 = 0;
uint16_t temp_u16 = 0;
uint32_t temp_u32 = 0;

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

void LoRa_Generate_Signal()
{
	/******** debug temps   ***********/
	uint32_t Chip_Count_No1[LORA_TOTAL_LENGTH_NO1] = {0};
	uint32_t Chip_Count_No2[LORA_TOTAL_LENGTH_NO2] = {0};
	
	uint32_t Chip_Position_No1 = 0;
	uint32_t Chip_Position_No2 = 0;
	
	uint32_t Chirp_Count_No1 = 0;
	uint32_t Chirp_Count_No2 = 0;
	
	
	
//	uint32_t Chirp_Time_Record_No1[LORA_TOTAL_LENGTH_NO1] = {0};
//	uint32_t Chirp_Time_Record_No2[LORA_TOTAL_LENGTH_NO2] = {0};
	
	/*********************************/
	
	bool Mix_Packets_flag = 1;
	bool Packet_No1_or_No2 = 0; // 0:No1,1:No2
	bool Last_Packet_No1_or_No2 = Packet_No1_or_No2;
	
	
	uint32_t Init_Frequency_Begin_Point_No1 = LORA_BASE_FREQ;
	uint32_t Init_Frequency_End_Point_No1 = LORA_MAX_FREQ;
	
	uint32_t Init_Frequency_Begin_Point_No2 = LORA_BASE_FREQ;
	uint32_t Init_Frequency_End_Point_No2 = LORA_MAX_FREQ;
	
	enum Chirp_Status Chirp_Status_No1 = Preamble;
	enum Chirp_Status Chirp_Status_No2 = Preamble;
	
	uint32_t Total_Chip_Count = 0;
	
	// +: RTC > TIM ---> TIM +
	// -: RTC < TIM ---> TIM - 
	
	Timer_Compensation_Index = RTC_Timer_Calibration();
	
	

	TIM3_Init( Timer_Compensation_Index - 1 );
	
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
	Send_packets:
 	SX1276SetOpMode( RF_OPMODE_TRANSMITTER );
	delay_ms(500);

	TIM2->CNT = 0;
	TIM3->CNT = 0;
	HAL_TIM_Base_Start_IT(&TIM2_Handler);
	HAL_TIM_Base_Start_IT(&TIM3_Handler);
	/*******************/
	LL_GPIO_TogglePin(GPIOB,GPIO_PIN_2);
	LL_GPIO_TogglePin(GPIOB,GPIO_PIN_11);
	LL_GPIO_TogglePin(GPIOB,GPIO_PIN_12);
	while( Chirp_Count_No2 < LORA_TOTAL_LENGTH_NO2 )
	{
//		Total_Chip_Count = 0;
		if( Chirp_Count_No1 < LORA_TOTAL_LENGTH_NO1 )
			Mix_Packets_flag = 1;
		else
			Mix_Packets_flag = 0;
			
		/***********  NO1 Packet states machine  ***************/
		//symbol @ preamble
		if( Chirp_Count_No1 < LORA_PREAMBLE_LENGTH_NO1 )
		{
			Chirp_Status_No1 = Preamble;
			Init_Frequency_Begin_Point_No1 = LORA_BASE_FREQ;
		}
		//symbol @ LoRa ID 0x12 0x34
		else if( Chirp_Count_No1 >= LORA_PREAMBLE_LENGTH_NO1 && \
						 Chirp_Count_No1 < LORA_PREAMBLE_LENGTH_NO1 + LORA_ID_LENGTH_NO1 )
		{
			Chirp_Status_No1 = ID;
			Init_Frequency_Begin_Point_No1 = RF_FREQUENCY + LoRa_ID_Start_Freq_No1[ Chirp_Count_No1 - LORA_PREAMBLE_LENGTH_NO1 ];
		}
		//symbol @ LoRa SFD 
		else if( Chirp_Count_No1 >= LORA_PREAMBLE_LENGTH_NO1 + LORA_ID_LENGTH_NO1 && \
						 Chirp_Count_No1 < LORA_PREAMBLE_LENGTH_NO1 + LORA_ID_LENGTH_NO1 + LORA_SFD_LENGTH_NO1 )
		{
			Chirp_Status_No1 = SFD;
			Init_Frequency_Begin_Point_No1 = LORA_MAX_FREQ;
		}
		//symbol @ LoRa 0.25 SFD
		else if( Chirp_Count_No1 >= LORA_PREAMBLE_LENGTH_NO1 + LORA_ID_LENGTH_NO1 + LORA_SFD_LENGTH_NO1 && \
						 Chirp_Count_No1 < LORA_PREAMBLE_LENGTH_NO1 + LORA_ID_LENGTH_NO1 + LORA_SFD_LENGTH_NO1 + LORA_QUARTER_SFD_LENGTH_NO1 )
		{
			Chirp_Status_No1 = Quarter_SFD;
			Init_Frequency_Begin_Point_No1 = LORA_MAX_FREQ;
		}
		//symbol @ LoRa payload
		else if(( Chirp_Count_No1 >= LORA_PREAMBLE_LENGTH_NO1 + LORA_ID_LENGTH_NO1 + LORA_SFD_LENGTH_NO1 + LORA_QUARTER_SFD_LENGTH_NO1) && \
						( Chirp_Count_No1 < LORA_PREAMBLE_LENGTH_NO1 + LORA_ID_LENGTH_NO1 + LORA_SFD_LENGTH_NO1 + LORA_QUARTER_SFD_LENGTH_NO1 + \
															LORA_PAYLOAD_LENGTH_NO1 ))
		{
			Chirp_Status_No1 = Payload;
			Init_Frequency_Begin_Point_No1 = RF_FREQUENCY + LoRa_Payload_Start_Freq_No1[ Chirp_Count_No1 - LORA_PREAMBLE_LENGTH_NO1 - \
																																								LORA_ID_LENGTH_NO1 - LORA_SFD_LENGTH_NO1 - 1 ];
		}
		
		/***********  NO2 Packet states machine ***************/
		//symbol @ preamble
		if( Chirp_Count_No2 < LORA_PREAMBLE_LENGTH_NO2 )
		{
			Chirp_Status_No2 = Preamble;
			Init_Frequency_Begin_Point_No2 = LORA_BASE_FREQ;
		}
		//symbol @ LoRa ID 0x12 0x34
		else if( Chirp_Count_No2 >= LORA_PREAMBLE_LENGTH_NO2 && \
						 Chirp_Count_No2 < LORA_PREAMBLE_LENGTH_NO2 + LORA_ID_LENGTH_NO2 )
		{
			Chirp_Status_No2 = ID;
			Init_Frequency_Begin_Point_No2 = RF_FREQUENCY + LoRa_ID_Start_Freq_No2[ Chirp_Count_No2 - LORA_PREAMBLE_LENGTH_NO2 ];
		}
		//symbol @ LoRa SFD 
		else if( Chirp_Count_No2 >= LORA_PREAMBLE_LENGTH_NO2 + LORA_ID_LENGTH_NO2 && \
						 Chirp_Count_No2 < LORA_PREAMBLE_LENGTH_NO2 + LORA_ID_LENGTH_NO2 + LORA_SFD_LENGTH_NO2 )
		{
			Chirp_Status_No2 = SFD;
			Init_Frequency_Begin_Point_No2 = LORA_MAX_FREQ;
		}
		//symbol @ LoRa 0.25 SFD
		else if( Chirp_Count_No2 >= LORA_PREAMBLE_LENGTH_NO2 + LORA_ID_LENGTH_NO2 + LORA_SFD_LENGTH_NO2 && \
						 Chirp_Count_No2 < LORA_PREAMBLE_LENGTH_NO2 + LORA_ID_LENGTH_NO2 + LORA_SFD_LENGTH_NO2 + LORA_QUARTER_SFD_LENGTH_NO2 )
		{
			Chirp_Status_No2 = Quarter_SFD;
			Init_Frequency_Begin_Point_No2 = LORA_MAX_FREQ;
		}
		//symbol @ LoRa payload
		else if( Chirp_Count_No2 >= LORA_PREAMBLE_LENGTH_NO2 + LORA_ID_LENGTH_NO2 + LORA_SFD_LENGTH_NO2 + LORA_QUARTER_SFD_LENGTH_NO2 && \
						 Chirp_Count_No2 < LORA_PREAMBLE_LENGTH_NO2 + LORA_ID_LENGTH_NO2 + LORA_SFD_LENGTH_NO2 + \
																LORA_QUARTER_SFD_LENGTH_NO2 + LORA_PAYLOAD_LENGTH_NO2 )
		{
			Chirp_Status_No2 = Payload;
			Init_Frequency_Begin_Point_No2 = RF_FREQUENCY + LoRa_Payload_Start_Freq_No2[ Chirp_Count_No2 - LORA_PREAMBLE_LENGTH_NO2 \
																																							- LORA_ID_LENGTH_NO2 - LORA_SFD_LENGTH_NO2 - 1 ];
		}
		
		/******  end of Packet states machine ********/
		while(1)  // generate symbol
		{
//			Time_temp[ Total_Chip_Count ] = Comped_Time;
			if( (!Packet_No1_or_No2) && Mix_Packets_flag )
			{
				switch (Chirp_Status_No1)
				{
					case Preamble:
					{
						Chip_Position_No1 = (Comped_Time - Chirp_Count_No1 * LORA_SYMBOL_TIME_NO1 ) >> 3;
						Input_Freq = Init_Frequency_Begin_Point_No1 + Chip_Position_No1 * LORA_FREQ_STEP_NO1;
						break;
					}
					case ID:
					{
						Chip_Position_No1 = (Comped_Time - Chirp_Count_No1 * LORA_SYMBOL_TIME_NO1 ) >> 3;
						Input_Freq = Init_Frequency_Begin_Point_No1 + Chip_Position_No1 * LORA_FREQ_STEP_NO1;
						if( Input_Freq > LORA_MAX_FREQ )
							Input_Freq = Input_Freq - LORA_BW;
						break;
					}
					case SFD:
					{
						Chip_Position_No1 = (Comped_Time - Chirp_Count_No1 * LORA_SYMBOL_TIME_NO1 ) >> 3;
						Input_Freq = Init_Frequency_Begin_Point_No1 - Chip_Position_No1 * LORA_FREQ_STEP_NO1;
						break;
					}
					case Quarter_SFD:
					{
						Chip_Position_No1 = (Comped_Time - Chirp_Count_No1 * LORA_SYMBOL_TIME_NO1 ) >> 3;
						Input_Freq = Init_Frequency_Begin_Point_No1 - Chip_Position_No1 * LORA_FREQ_STEP_NO1;
						break;
					}
					case Payload:
					{
						Chip_Position_No1 = ( Comped_Time - ( LORA_SYMBOL_TIME_NO1 >> 2 ) - (Chirp_Count_No1 - 1) * LORA_SYMBOL_TIME_NO1 ) >> 3;
						Input_Freq = Init_Frequency_Begin_Point_No1 + Chip_Position_No1 * LORA_FREQ_STEP_NO1;
						if( Input_Freq > LORA_MAX_FREQ )
							Input_Freq = Input_Freq - LORA_BW;
						break;
					}
					default:break;
				}
				
				Chip_Count_No1[Chirp_Count_No1]++;
	//			Input_Freq_temp_No1[ Chirp_Count_No1 ] [ Chip_Count_No1[Chirp_Count_No1]] = Input_Freq;
			}
			else
			{

				switch (Chirp_Status_No2)
				{
					case Preamble:
					{
						Chip_Position_No2 = (Comped_Time - Chirp_Count_No2 * LORA_SYMBOL_TIME_NO2 ) >> 3;
						Input_Freq = Init_Frequency_Begin_Point_No2 + Chip_Position_No2 * LORA_FREQ_STEP_NO2;
						break;
					}
					case ID:
					{
						Chip_Position_No2 = (Comped_Time - Chirp_Count_No2 * LORA_SYMBOL_TIME_NO2 ) >> 3;
						Input_Freq = Init_Frequency_Begin_Point_No2 + Chip_Position_No2 * LORA_FREQ_STEP_NO2;
						if( Input_Freq > LORA_MAX_FREQ )
							Input_Freq = Input_Freq - LORA_BW;
						break;
					}
					case SFD:
					{
						Chip_Position_No2 = (Comped_Time - Chirp_Count_No2 * LORA_SYMBOL_TIME_NO2 ) >> 3;
						Input_Freq = Init_Frequency_Begin_Point_No2 - Chip_Position_No2 * LORA_FREQ_STEP_NO2;
						break;
					}
					case Quarter_SFD:
					{
						Chip_Position_No2 = (Comped_Time - Chirp_Count_No2 * LORA_SYMBOL_TIME_NO2 ) >> 3;
						Input_Freq = Init_Frequency_Begin_Point_No2 - Chip_Position_No2 * LORA_FREQ_STEP_NO2;
						break;
					}
					case Payload:
					{
						Chip_Position_No2 = ( Comped_Time - ( LORA_SYMBOL_TIME_NO2 >> 2 ) - (Chirp_Count_No2 - 1) * LORA_SYMBOL_TIME_NO2 ) >> 3;
						Input_Freq = Init_Frequency_Begin_Point_No2 + Chip_Position_No2 * LORA_FREQ_STEP_NO2;
						if( Input_Freq > LORA_MAX_FREQ )
							Input_Freq = Input_Freq - LORA_BW;
						break;
					}
					default:break;
				}
				Input_Freq += FREQ_OFFSET_1_2;
				Chip_Count_No2[Chirp_Count_No2]++;
	//			Input_Freq_temp_No2[ Chirp_Count_No2 ] [ Chip_Count_No2[Chirp_Count_No2]] = Input_Freq;
			}
			
			
//			if( !Packet_No1_or_No2 )
//			{
//				LL_GPIO_SetOutputPin(GPIOB,GPIO_PIN_5);
				SX_FREQ_TO_CHANNEL( Channel, Input_Freq );
//			}
//			else
//			{
//				LL_GPIO_ResetOutputPin(GPIOB,GPIO_PIN_5);
//			}
				
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
//			Time_temp1[ Total_Chip_Count ] = Comped_Time;
			while( Comped_Time & ( 8 - 1 ));// chip time = 8us
//			Time_temp1[ Total_Chip_Count ] = Comped_Time;
			Fast_SetChannel( Channel_Freq, Changed_Register_Count );
			
//			Time_temp2[ Total_Chip_Count ] = Comped_Time;

			Total_Chip_Count++; 
//			Input_Freq_temp[ Total_Chip_Count ] = Input_Freq;
			
			Changed_Register_Count = 1;
			if(Mix_Packets_flag)
			{
				if( (Total_Chip_Count % 64) <  32)
				{
					Last_Packet_No1_or_No2 = Packet_No1_or_No2;
					Packet_No1_or_No2 = 0;
				}
				else
				{
					Last_Packet_No1_or_No2 = Packet_No1_or_No2;
					Packet_No1_or_No2 = 1;
				}
				
//				if( Packet_No1_or_No2 != Last_Packet_No1_or_No2)
//				{
//					LL_GPIO_ResetOutputPin(GPIOB,GPIO_PIN_5);
//				}
			}
			
			LL_GPIO_TogglePin(GPIOB,GPIO_PIN_12); 
			
			if( Mix_Packets_flag )
			{
				switch (Chirp_Status_No1)
				{
					case Preamble:	if( Comped_Time - LORA_SYMBOL_TIME_NO1 * Chirp_Count_No1 >= LORA_SYMBOL_TIME_NO1 )
													{
//														Chirp_Time_Record_No1[ Chirp_Count_No1 ] = Comped_Time;
														Chirp_Count_No1++;
														LL_GPIO_TogglePin(GPIOB,GPIO_PIN_2);
														goto Symbol_End;		
													}
													break;
					case ID:				if( Comped_Time - LORA_SYMBOL_TIME_NO1 * Chirp_Count_No1 >= LORA_SYMBOL_TIME_NO1 )
													{
//														Chirp_Time_Record_No1[ Chirp_Count_No1 ] = Comped_Time;
														Chirp_Count_No1++;
														LL_GPIO_TogglePin(GPIOB,GPIO_PIN_2);
														goto Symbol_End;
													}
													break;
					case SFD:				if( Comped_Time - LORA_SYMBOL_TIME_NO1 * Chirp_Count_No1 >= LORA_SYMBOL_TIME_NO1 )
													{
//														Chirp_Time_Record_No1[ Chirp_Count_No1 ] = Comped_Time;
														Chirp_Count_No1++;
														LL_GPIO_TogglePin(GPIOB,GPIO_PIN_2);
														goto Symbol_End;
													}
													break; 
					case Quarter_SFD:if( Comped_Time - LORA_SYMBOL_TIME_NO1 * Chirp_Count_No1 >= ( LORA_SYMBOL_TIME_NO1 >> 2 ) )
													{
//														Chirp_Time_Record_No1[ Chirp_Count_No1 ] = Comped_Time;
														Chirp_Count_No1++;
														LL_GPIO_TogglePin(GPIOB,GPIO_PIN_2);
														goto Symbol_End;													
													}
													break;
					case Payload:		if( Comped_Time - ( LORA_SYMBOL_TIME_NO1 * ( Chirp_Count_No1 - 1 ) + ( LORA_SYMBOL_TIME_NO1 >> 2 )) >= ( LORA_SYMBOL_TIME_NO1 ) )
													{
//														Chirp_Time_Record_No1[ Chirp_Count_No1 ] = Comped_Time;
														Chirp_Count_No1++;
														LL_GPIO_TogglePin(GPIOB,GPIO_PIN_2);		
														goto Symbol_End;													
													}
													break;
					default:break;
				}
				
				Check_Symbol_End_No2:
				switch (Chirp_Status_No2)
				{
					case Preamble:	if( Comped_Time - LORA_SYMBOL_TIME_NO2 * Chirp_Count_No2 >= LORA_SYMBOL_TIME_NO2 )
													{
//														Chirp_Time_Record_No2[ Chirp_Count_No2 ] = Comped_Time;
														Chirp_Count_No2++;
														LL_GPIO_TogglePin(GPIOB,GPIO_PIN_11);
														goto Symbol_End;
													}
													break;
					case ID:				if( Comped_Time - LORA_SYMBOL_TIME_NO2 * Chirp_Count_No2 >= LORA_SYMBOL_TIME_NO2 )
													{
//														Chirp_Time_Record_No2[ Chirp_Count_No2 ] = Comped_Time;
														Chirp_Count_No2++;
														LL_GPIO_TogglePin(GPIOB,GPIO_PIN_11);
														goto Symbol_End;
													}
													break;
					case SFD:				if( Comped_Time - LORA_SYMBOL_TIME_NO2 * Chirp_Count_No2 >= LORA_SYMBOL_TIME_NO2 )
													{
//														Chirp_Time_Record_No2[ Chirp_Count_No2 ] = Comped_Time;
														Chirp_Count_No2++;
														LL_GPIO_TogglePin(GPIOB,GPIO_PIN_11);
														goto Symbol_End;
													}
													break; 
					case Quarter_SFD:if( Comped_Time - LORA_SYMBOL_TIME_NO2 * Chirp_Count_No2 >= ( LORA_SYMBOL_TIME_NO2 >> 2 ) )
													{
//														Chirp_Time_Record_No2[ Chirp_Count_No2 ] = Comped_Time;
														Chirp_Count_No2++;
														LL_GPIO_TogglePin(GPIOB,GPIO_PIN_11);
														goto Symbol_End;
													}
													break;
					case Payload:		if( Comped_Time - ( LORA_SYMBOL_TIME_NO2 * ( Chirp_Count_No2 - 1 ) + ( LORA_SYMBOL_TIME_NO2 >> 2 )) >= ( LORA_SYMBOL_TIME_NO2 ) )
													{
//														Chirp_Time_Record_No2[ Chirp_Count_No2 ] = Comped_Time;
														Chirp_Count_No2++;
														LL_GPIO_TogglePin(GPIOB,GPIO_PIN_11);
														goto Symbol_End;
													}
													break;
					default:break;
				}
			}
			else
				goto Check_Symbol_End_No2;
		}	// end loop of symbol
		Symbol_End:
	}
	/*******************/
	
	SX1276SetOpMode( RF_OPMODE_STANDBY );
	HAL_TIM_Base_Stop_IT(&TIM2_Handler);
	Total_Chip_Count = 0;
	Chirp_Count_No1 = 0;
	Chirp_Count_No2 = 0;
	Chirp_Count_No1 = 0;
	Chirp_Count_No2 = 0;
//	for(temp_u32 = 0 ; temp_u32 < sizeof(Input_Freq_temp)/sizeof(Input_Freq_temp[0]); temp_u32++)
//	{
//		if( Input_Freq_temp[temp_u32] != 0)
//			printf("%d\n",Input_Freq_temp[temp_u32]);
//	}
	printf("done\n");
	
	delay_ms(1000);
	
//	 goto Send_packets;
}

