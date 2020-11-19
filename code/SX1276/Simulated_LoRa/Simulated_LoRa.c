#include "hw.h"
#include "radio.h"
#include "sx1276.h"
#include "sx1276mb1mas.h"
#include "delay.h"
#include "Timer_Calibration.h"
#include "control_GPIO.h"

#include "Simulated_LoRa.h"


#define Comped_Time ( TIM2->CNT + Timer_Compensation_Count ) 

uint8_t test_symbol_point=2;
// sf =11 payload =1 
//int LoRa_ID_Start_Freq[LORA_ID_LENGTH] = {-62011,-61523};
//int LoRa_Payload_Start_Freq[] = {
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

//sf = 7 payload =1 
//int LoRa_ID_Start_Freq[LORA_ID_LENGTH] = {-54695,-46890};
//int LoRa_Payload_Start_Freq[] = {
//-46890,
//-62500,
//-62500,
//46768,
//-31280,
//31158,
//-7865,
//31158,
//53597,
//54573,
//30182,
//-31280,
//-31280,
//-4939,
//-53719,
//-54695,
//};

//sf = 8 payload =1
int LoRa_ID_Start_Freq[LORA_ID_LENGTH] = {-58597,-54695};
int LoRa_Payload_Start_Freq[] = {
-54695,
-2012,
-33231,
52621,
-31280,
33109,
-7865,
-35183,
-32743,
-61,
-23963,
-46890,
-46890,
-58109,
57987,
-60548
};


uint8_t Channel_Freq_MSB_temp = 0;
uint8_t Channel_Freq_MID_temp = 0;
uint8_t Channel_Freq_LSB_temp = 0;

uint32_t Input_Freq;
uint32_t Channel;

uint32_t Time_temp[ 1<<LORA_SF ]={0};
uint32_t n_temp[ 1<<LORA_SF ]={0};

uint8_t Channel_Freq[3] = {0};  //MSB,MID,LSB
uint8_t Changed_Register_Count = 1;  // the number of changed registers.
uint32_t Input_Freq_temp[ (1<<LORA_SF) ]={0};
uint32_t Input_Freq_temp1[ (1<<LORA_SF) ]={0};

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
	uint32_t Chip_Count[LORA_TOTAL_LENGTH] = {0};
	uint32_t Chip_Position = 0;
	uint32_t Chirp_Count = 0;
	uint32_t Chirp_Time_Record[LORA_TOTAL_LENGTH] = {0};
	
	uint32_t Init_Frequency_Begin_Point = LORA_BASE_FREQ;
	uint32_t Init_Frequency_End_Point = LORA_MAX_FREQ;
	
	enum Chirp_Status Chirp_Status = Preamble;
	
	
	// +: RTC > TIM ---> TIM +
	// -: RTC < TIM ---> TIM - 
	
	Timer_Compensation_Index = RTC_Timer_Calibration();
	
	TIM3_Init( Timer_Compensation_Index - 1 );
	
 	SX1276SetOpMode( RF_OPMODE_TRANSMITTER );
	delay_ms(100);

	TIM2->CNT = 0;
	TIM3->CNT = 0;
	HAL_TIM_Base_Start_IT(&TIM2_Handler);
	HAL_TIM_Base_Start_IT(&TIM3_Handler);
	/*******************/
	LL_GPIO_TogglePin(GPIOB,GPIO_PIN_2);
	while( Chirp_Count < LORA_TOTAL_LENGTH )
	{
		/******  Packet states machine ********/
		Symbol_Start:
		//symbol @ preamble
		if( Chirp_Count < LORA_PREAMBLE_LENGTH )
		{
			Chirp_Status = Preamble;
			Init_Frequency_Begin_Point = LORA_BASE_FREQ;
//			Init_Frequency_End_Point = LORA_MAX_FREQ;
		}
		//symbol @ LoRa ID 0x12 0x34
		else if( Chirp_Count >= LORA_PREAMBLE_LENGTH && \
						 Chirp_Count < LORA_PREAMBLE_LENGTH + LORA_ID_LENGTH )
		{
			Chirp_Status = ID;
			Init_Frequency_Begin_Point = RF_FREQUENCY + LoRa_ID_Start_Freq[ Chirp_Count - LORA_PREAMBLE_LENGTH ];
//			Init_Frequency_End_Point = LORA_MAX_FREQ;
		}
		//symbol @ LoRa SFD 
		else if( Chirp_Count >= LORA_PREAMBLE_LENGTH + LORA_ID_LENGTH && \
						 Chirp_Count < LORA_PREAMBLE_LENGTH + LORA_ID_LENGTH + LORA_SFD_LENGTH )
		{
			Chirp_Status = SFD;
			Init_Frequency_Begin_Point = LORA_MAX_FREQ;
//			Init_Frequency_End_Point = LORA_BASE_FREQ;
		}
		//symbol @ LoRa 0.25 SFD
		else if( Chirp_Count >= LORA_PREAMBLE_LENGTH + LORA_ID_LENGTH + LORA_SFD_LENGTH && \
						 Chirp_Count < LORA_PREAMBLE_LENGTH + LORA_ID_LENGTH + LORA_SFD_LENGTH + LORA_QUARTER_SFD_LENGTH )
		{
			Chirp_Status = Quarter_SFD;
			Init_Frequency_Begin_Point = LORA_MAX_FREQ;
//			Init_Frequency_End_Point = LORA_BASE_FREQ;
		}
		//symbol @ LoRa payload
		else if( Chirp_Count >= LORA_PREAMBLE_LENGTH + LORA_ID_LENGTH + LORA_SFD_LENGTH + LORA_QUARTER_SFD_LENGTH && \
						 Chirp_Count < LORA_PREAMBLE_LENGTH + LORA_ID_LENGTH + LORA_SFD_LENGTH + LORA_QUARTER_SFD_LENGTH + LORA_PAYLOAD_LENGTH )
		{
			Chirp_Status = Payload;
			Init_Frequency_Begin_Point = RF_FREQUENCY + LoRa_Payload_Start_Freq[ Chirp_Count - LORA_PREAMBLE_LENGTH - LORA_ID_LENGTH - LORA_SFD_LENGTH - 1 ];
//			Init_Frequency_End_Point = LORA_MAX_FREQ;
		}
		
		/******  end of Packet states machine ********/
		while(1)  // generate symbol
		{
			switch (Chirp_Status)
			{
				case Preamble:
				{
					Chip_Position = (int)((Comped_Time +  - Chirp_Count * LORA_SYMBOL_TIME ) >> 3);
					Input_Freq = Init_Frequency_Begin_Point + Chip_Position * LORA_FREQ_STEP;
					break;
				}
				case ID:
				{
					Chip_Position = (int)((Comped_Time +  - Chirp_Count * LORA_SYMBOL_TIME ) >> 3);
					Input_Freq = Init_Frequency_Begin_Point + Chip_Position * LORA_FREQ_STEP;
					if( Input_Freq > LORA_MAX_FREQ )
						Input_Freq = Input_Freq - LORA_BW;
					break;
				}
				case SFD:
				{
					Chip_Position = (int)((Comped_Time +  - Chirp_Count * LORA_SYMBOL_TIME ) >> 3);
					Input_Freq = (int)(Init_Frequency_Begin_Point - Chip_Position * LORA_FREQ_STEP);
					break;
				}
				case Quarter_SFD:
				{
					Chip_Position = (int)((Comped_Time +  - Chirp_Count * LORA_SYMBOL_TIME ) >> 3);
					Input_Freq = (int)(Init_Frequency_Begin_Point - Chip_Position * LORA_FREQ_STEP);
					break;
				}
				case Payload:
				{
					Chip_Position = (int)(( Comped_Time - ( LORA_SYMBOL_TIME >> 2 ) - (Chirp_Count - 1) * LORA_SYMBOL_TIME ) >> 3 );
					Input_Freq = Init_Frequency_Begin_Point + Chip_Position * LORA_FREQ_STEP;
					if( Input_Freq > LORA_MAX_FREQ )
						Input_Freq = Input_Freq - LORA_BW;
					break;
				}
				default:break;
			}
//			//symbol @ upchirp (Payload)
//			if( Chirp_Status == Payload )
//			{
//				Chip_Position = (int)(( Comped_Time - ( LORA_SYMBOL_TIME >> 2 ) - (Chirp_Count - 1) * LORA_SYMBOL_TIME ) >> 3 );
//				Input_Freq = Init_Frequency_Begin_Point + Chip_Position * LORA_FREQ_STEP;
//				if( Input_Freq > LORA_MAX_FREQ )
//					Input_Freq = Input_Freq - LORA_BW;
//			}
//			//symbol @ upchirp (ID,)
//			else if( Chirp_Status == ID )
//			{
//				Chip_Position = (int)((Comped_Time +  - Chirp_Count * LORA_SYMBOL_TIME ) >> 3) ;
//				Input_Freq = Init_Frequency_Begin_Point + Chip_Position * LORA_FREQ_STEP;
//				if( Input_Freq > LORA_MAX_FREQ )
//					Input_Freq = Input_Freq - LORA_BW;
//			}
//			//symbol @ downchirp (SFD, Quarter_SFD)
//			else if( Chirp_Status == SFD || Chirp_Status == Quarter_SFD )
//			{
//				Chip_Position = (int)((Comped_Time +  - Chirp_Count * LORA_SYMBOL_TIME ) >> 3) ;
//				Input_Freq = (int)(Init_Frequency_Begin_Point - Chip_Position * LORA_FREQ_STEP);
//			}
//			//symbol @ upchirp (preamble)
//			else if( Chirp_Status == Preamble )
//			{
//				Chip_Position = (int)((Comped_Time +  - Chirp_Count * LORA_SYMBOL_TIME ) >> 3) ;
//				Input_Freq = Init_Frequency_Begin_Point + Chip_Position * LORA_FREQ_STEP;
//				if( Input_Freq > LORA_MAX_FREQ )
//					Input_Freq = Input_Freq - LORA_BW;
//			}
			
			SX_FREQ_TO_CHANNEL( Channel, Input_Freq );
			
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
			
			while( Comped_Time & ( 8 - 1 ));// chip time = 8us
			
			Fast_SetChannel( Channel_Freq, Changed_Register_Count );
			Time_temp[ Chip_Count[ Chirp_Count ] ] = Comped_Time;
			
			
			if( Chirp_Count == test_symbol_point - 1)
			{
//			Input_Freq_temp[ Chip_Count[ Chirp_Count ] ] = Input_Freq;
				Input_Freq_temp[ Chip_Position ] = Input_Freq;
				Input_Freq_temp1[ Chip_Position ] = Chip_Position;
			}
//			n_temp[ Chip_Count[ Chirp_Count ] ] = Changed_Register_Count;
			Changed_Register_Count = 1;
			Chip_Count[ Chirp_Count ]++;
			
			LL_GPIO_TogglePin(GPIOB,GPIO_PIN_12);
			
			switch (Chirp_Status)
			{
				case Preamble:	if( Comped_Time - LORA_SYMBOL_TIME * Chirp_Count >= LORA_SYMBOL_TIME )
												{
													Chirp_Time_Record[ Chirp_Count ] = Comped_Time;
													goto Symbol_End;
												}
												break;
				case ID:				if( Comped_Time - LORA_SYMBOL_TIME * Chirp_Count >= LORA_SYMBOL_TIME )
												{
													Chirp_Time_Record[ Chirp_Count ] = Comped_Time;
													goto Symbol_End;
												}
												break;
				case SFD:				if( Comped_Time - LORA_SYMBOL_TIME * Chirp_Count >= LORA_SYMBOL_TIME )
												{
													Chirp_Time_Record[ Chirp_Count ] = Comped_Time;
													goto Symbol_End;
												}
												break;
				case Quarter_SFD:if( Comped_Time - LORA_SYMBOL_TIME * Chirp_Count >= ( LORA_SYMBOL_TIME >> 2 ) )
												{
													Chirp_Time_Record[ Chirp_Count ] = Comped_Time;
													goto Symbol_End;
												}
												break;
				case Payload:		if( Comped_Time - ( LORA_SYMBOL_TIME * ( Chirp_Count - 1 ) + ( LORA_SYMBOL_TIME >> 2 )) >= ( LORA_SYMBOL_TIME ) )
												{
													Chirp_Time_Record[ Chirp_Count ] = Comped_Time;
													goto Symbol_End;
												}
												break;
				default:break;
			}

				
		}	// end loop of symbol
		Symbol_End:
		LL_GPIO_TogglePin(GPIOB,GPIO_PIN_2);
		if( Chirp_Count == test_symbol_point - 1 )
		{
			Chirp_Count = Chirp_Count;
		}
		Chirp_Count++;
		
//		memset(Input_Freq_temp,0, (1<<LORA_SF) * sizeof(uint32_t) );
	}
	
	
	/*******************/
	
	SX1276SetOpMode( RF_OPMODE_STANDBY );
	HAL_TIM_Base_Stop_IT(&TIM2_Handler);
	Chirp_Count = 0;
	
	delay_ms(1000);

}

