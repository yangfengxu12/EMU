#include "hw.h"
#include "radio.h"
#include "sx1276.h"
#include "sx1276mb1mas.h"
#include "delay.h"
#include "Timer_Calibration.h"
#include "control_GPIO.h"

#include "Simulated_LoRa.h"

extern uint32_t time_count;

int LoRa_ID_Start_Freq[LORA_ID_LENGTH] = {-62011,-61523};
int LoRa_Payload_Start_Freq[] = {-7083,
-27345,
-58594,
60295,
-37355,
33441,
-53223,
-35646,
3597,
46746,
8662,
61088,
-60364,
-61706,
-16543,
-52307};

uint8_t Channel_Freq_MSB_temp = 0;
uint8_t Channel_Freq_MID_temp = 0;
uint8_t Channel_Freq_LSB_temp = 0;

uint32_t Input_Freq;
uint32_t Channel;


uint32_t Input_Freq_temp[ 1<<LORA_SF ]={0};
uint32_t Time_temp[ 1<<LORA_SF ]={0};
uint32_t n_temp[ 1<<LORA_SF ]={0};

uint8_t Channel_Freq[3] = {0};  //MSB,MID,LSB
uint8_t Changed_Register_Count = 1;  // the number of changed registers.


uint8_t Count1 = 0;
uint8_t Count2 = 0;
uint8_t Count3 = 0;


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
	uint32_t Chirp_Count = 0;
	uint32_t Chirp_Time_Record[LORA_TOTAL_LENGTH] = {0};
	
	uint32_t Init_Frequency_Point = LORA_BASE_FREQ;
	uint32_t Init_Frequency_End_Point = LORA_MAX_FREQ;
	
	enum Chirp_Status Chirp_Status = Preamble;
	
	// +: RTC < TIM ---> TIM - 
	// -: RTC > TIM ---> TIM + 
	int Timer_Calibration_Index = 0;
	int Timer_Calibration_Flag = 0;
	int Timer_Calibration_Flag_Last = 0;
//	Timer_Calibration_Index = RTC_Timer_Calibration();
	
	SX1276SetOpMode( RF_OPMODE_TRANSMITTER );
	delay_ms(100);

	time_count = 0;
	TIM2->CNT = 0;
	HAL_TIM_Base_Start_IT(&TIM2_Handler);
	
	/*******************/
	LL_GPIO_TogglePin(GPIOB,GPIO_PIN_2);
	while( Chirp_Count < LORA_TOTAL_LENGTH )
	{
		/******  Packet states machine ********/
		//symbol @ preamble
		if( Chirp_Count < LORA_PREAMBLE_LENGTH )
		{
			Chirp_Status = Preamble;
			Init_Frequency_Point = LORA_BASE_FREQ;
			Init_Frequency_End_Point = LORA_MAX_FREQ;
		}
		//symbol @ LoRa ID 0x12 0x34
		else if( Chirp_Count >= LORA_PREAMBLE_LENGTH && \
						 Chirp_Count < LORA_PREAMBLE_LENGTH + LORA_ID_LENGTH )
		{
			Chirp_Status = ID;
			Init_Frequency_Point = LoRa_ID_Start_Freq[ Chirp_Count - LORA_PREAMBLE_LENGTH ];
			Init_Frequency_End_Point = LORA_MAX_FREQ;
		}
		//symbol @ LoRa SFD 0x12 0x34
		else if( Chirp_Count >= LORA_PREAMBLE_LENGTH + LORA_ID_LENGTH && \
						 Chirp_Count < LORA_PREAMBLE_LENGTH + LORA_ID_LENGTH + LORA_SFD_LENGTH )
		{
			Chirp_Status = SFD;
			Init_Frequency_Point = LORA_MAX_FREQ;
			Init_Frequency_End_Point = LORA_BASE_FREQ;
		}
		//symbol @ LoRa 0.25 SFD
		else if( Chirp_Count >= LORA_PREAMBLE_LENGTH + LORA_ID_LENGTH + LORA_SFD_LENGTH && \
						 Chirp_Count < LORA_PREAMBLE_LENGTH + LORA_ID_LENGTH + LORA_SFD_LENGTH + LORA_QUARTER_SFD_LENGTH )
		{
			Chirp_Status = Quarter_SFD;
			Init_Frequency_Point = LORA_MAX_FREQ;
			Init_Frequency_End_Point = LORA_BASE_FREQ;
		}
		//symbol @ LoRa payload
		else if( Chirp_Count >= LORA_PREAMBLE_LENGTH + LORA_ID_LENGTH + LORA_SFD_LENGTH + LORA_QUARTER_SFD_LENGTH && \
						 Chirp_Count < LORA_PREAMBLE_LENGTH + LORA_ID_LENGTH + LORA_SFD_LENGTH + LORA_QUARTER_SFD_LENGTH + LORA_PAYLOAD_LENGTH )
		{
			Chirp_Status = Payload;
			Init_Frequency_Point = LoRa_Payload_Start_Freq[ Chirp_Count - LORA_PREAMBLE_LENGTH - LORA_ID_LENGTH - LORA_SFD_LENGTH ];
			Init_Frequency_End_Point = LORA_MAX_FREQ;
		}
		/******  end of Packet states machine ********/
		while(1)  // generate symbol
		{
			//symbol @ downchirp (SFD, quarter_SFD)
			if( Chirp_Status == SFD || Chirp_Status == Quarter_SFD )
				Input_Freq = Init_Frequency_Point - Chip_Count[ Chirp_Count ] * LORA_FREQ_STEP;
			
			//symbol @ upchirp (preamble,ID, payload)
			else
			{
				Input_Freq = Init_Frequency_Point + Chip_Count[ Chirp_Count ] * LORA_FREQ_STEP;
				if( Input_Freq > LORA_MAX_FREQ )
					Input_Freq = Input_Freq - LORA_BW;
			}
			
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
			
			while( TIM2->CNT & ( 8 - 1 ));// chip time = 8us

//			while( time_count & ( 8 - 1 ));  // chip time = 8us
//			while( time_count % 8 != 0);  // chip time = 8us
			
			Fast_SetChannel( Channel_Freq, Changed_Register_Count );
			Time_temp[ Chip_Count[ Chirp_Count ] ] = TIM2->CNT;
			Input_Freq_temp[ Chip_Count[ Chirp_Count ] ] = Input_Freq;
			
//			n_temp[ Chip_Count[ Chirp_Count ] ] = Changed_Register_Count;
			Changed_Register_Count = 1;
			Chip_Count[ Chirp_Count ]++;
			
			LL_GPIO_TogglePin(GPIOB,GPIO_PIN_12);
			
			//symbol @ preamble, ID, SFD, payload
			if(( Chirp_Status == Preamble || Chirp_Status == ID || Chirp_Status == SFD ) && (Chip_Count[ Chirp_Count ] >= (1 << LORA_SF) - 2))
			{
				if( TIM2->CNT - LORA_SYMBOL_TIME * Chirp_Count >= LORA_SYMBOL_TIME - 8 )
				{
					Chirp_Time_Record[ Chirp_Count ] = TIM2->CNT;
					
					break;
				}
			}
			//symbol @ quarter_SFD (0.25*SFD)
			else if( Chirp_Status == Quarter_SFD ) 
			{
				if( TIM2->CNT - LORA_SYMBOL_TIME * Chirp_Count >= ( LORA_SYMBOL_TIME >> 2 ) - 8 )
				{
					Chirp_Time_Record[ Chirp_Count ] = TIM2->CNT;
					
					break;
				}
			}
			else if( Chirp_Status == Payload )
			{
				if( TIM2->CNT - ( LORA_SYMBOL_TIME * ( Chirp_Count - 1 ) + ( LORA_SYMBOL_TIME >> 2 )) >= ( LORA_SYMBOL_TIME - 8 ))
				{
					Chirp_Time_Record[ Chirp_Count ] = TIM2->CNT;
					
					break;
				}
			}
		
//			Timer_Calibration_Flag = TIM2->CNT / Timer_Calibration_Index;
//			// +: RTC < TIM ---> TIM - 
//			// -: RTC > TIM ---> TIM + 
//			if( Timer_Calibration_Flag != Timer_Calibration_Flag_Last )
//			{
//				if( Timer_Calibration_Index > 0 )
//					TIM2->CNT = TIM2->CNT - 1;
//				else if( Timer_Calibration_Index < 0 )
//					TIM2->CNT = TIM2->CNT + 1;
//				
//				Timer_Calibration_Flag_Last = Timer_Calibration_Flag;
//			}
				
		}	// end loop of symbol
		LL_GPIO_TogglePin(GPIOB,GPIO_PIN_2);
		Chirp_Count++;
	}
	
	
	/*******************/
	
	SX1276SetOpMode( RF_OPMODE_STANDBY );
	HAL_TIM_Base_Stop_IT(&TIM2_Handler);
	delay_ms(2000);
	
}










//void LoRa_UpChirp()
//{
//	uint32_t Count = 0;

//	Input_Freq = LORA_BASE_FREQ;
//	TIM1->CNT = 0;
//	while(1)
//	{
//		Time = TIM1->CNT;
//		if( Time > LORA_SYMBOL_TIME || ( Input_Freq > LORA_BASE_FREQ + LORA_BW ))
//		{
//			break;
//		}
//		else if((( GPIOB->IDR & GPIO_PIN_6) != 0x00u) && (( GPIOA->IDR & GPIO_PIN_9) != 0x00u ))
//		{
//			Input_Freq = LORA_BASE_FREQ + Count * LORA_FREQ_STEP;
//			
//			SX_FREQ_TO_CHANNEL( Channel, Input_Freq );

//			Channel_Freq[0] = ( uint8_t )( ( Channel >> 16 ) & 0xFF );
//			Channel_Freq[1] = ( uint8_t )( ( Channel >> 8 ) & 0xFF );
//			Channel_Freq[2] = ( uint8_t )( ( Channel ) & 0xFF );
//			
//			if(Channel_Freq_MSB_temp != Channel_Freq[0])
//			{
//				Changed_Register_Count = 3;
//				Channel_Freq_MSB_temp = Channel_Freq[0];
//				Channel_Freq_MID_temp = Channel_Freq[1];
//				Channel_Freq_LSB_temp = Channel_Freq[2];
//			}
//			else if(Channel_Freq_MID_temp != Channel_Freq[1])
//			{
//				Changed_Register_Count = 2;
//				Channel_Freq_MID_temp = Channel_Freq[1];
//				Channel_Freq_LSB_temp = Channel_Freq[2];
//			}
//			else if(Channel_Freq_MID_temp != Channel_Freq[2])
//			{
//				Changed_Register_Count = 1;
//				Channel_Freq_LSB_temp = Channel_Freq[2];
//			}
//			
//			do
//			{
//				Time = TIM1->CNT;
//			}while( Time & (8-1));  // 10Mhz spi

//			Fast_SetChannel( Channel_Freq, Changed_Register_Count);
//			Time_temp[Count] = Time;
//			Input_Freq_temp[Count] = Input_Freq;
//			
//			n_temp[Count] = Changed_Register_Count;
//			Changed_Register_Count = 1;
//			Count++;
//			
//			HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_12);
//		}
//	}
//}

//void LoRa_DownChirp()
//{
//	uint32_t Count = 0;

//	TIM1->CNT = 0;
//	while(1)
//	{
//		Time = TIM1->CNT;
//		if(Time > LORA_SYMBOL_TIME || ( Input_Freq < LORA_BASE_FREQ ))
//		{
//			break;
//		}
//		else if((( GPIOB->IDR & GPIO_PIN_6) != 0x00u) && ( ( GPIOA->IDR & GPIO_PIN_9 ) != 0x00u ))
//		{
//			Input_Freq = LORA_MAX_FREQ - Count * LORA_FREQ_STEP;
//			
//			SX_FREQ_TO_CHANNEL( Channel, Input_Freq );

//			Channel_Freq[0] = ( uint8_t )( ( Channel >> 16 ) & 0xFF );
//			Channel_Freq[1] = ( uint8_t )( ( Channel >> 8 ) & 0xFF );
//			Channel_Freq[2] = ( uint8_t )( ( Channel ) & 0xFF );
//			
//			if(Channel_Freq_MSB_temp != Channel_Freq[0])
//			{
//				Changed_Register_Count = 3;
//				Channel_Freq_MSB_temp = Channel_Freq[0];
//				Channel_Freq_MID_temp = Channel_Freq[1];
//				Channel_Freq_LSB_temp = Channel_Freq[2];
//			}
//			else if(Channel_Freq_MID_temp != Channel_Freq[1])
//			{
//				Changed_Register_Count = 2;
//				Channel_Freq_MID_temp = Channel_Freq[1];
//				Channel_Freq_LSB_temp = Channel_Freq[2];
//			}
//			else if(Channel_Freq_MID_temp != Channel_Freq[2])
//			{
//				Changed_Register_Count = 1;
//				Channel_Freq_LSB_temp = Channel_Freq[2];
//			}
//			
//			do
//			{
//				Time = TIM1->CNT;
//			}while( Time & (8-1));  // 10Mhz spi

//			Fast_SetChannel( Channel_Freq );
//			Time_temp[Count] = Time;
//			Input_Freq_temp[Count] = Input_Freq;
//			
//			n_temp[Count] = Changed_Register_Count;
//			Changed_Register_Count = 1;
//			Count++;
//			
//			HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_12);
//		}
//	}
//}

//void Generate_Quarter_DownChirp()
//{
//	uint32_t Count = 0;
//	
//	Input_Freq = LORA_MAX_FREQ;
//	TIM1->CNT = 0;
//	while(1)
//	{
//		Time = TIM1->CNT;
//		if(Time > LORA_SYMBOL_TIME / 4 )
//		{
//			break;
//		}
//		else if((( GPIOB->IDR & GPIO_PIN_6) != 0x00u) && ( ( GPIOA->IDR & GPIO_PIN_9 ) != 0x00u ))
//		{
//			Input_Freq = LORA_MAX_FREQ - Count * LORA_FREQ_STEP;
//			
//			SX_FREQ_TO_CHANNEL( Channel, Input_Freq );

//			Channel_Freq[0] = ( uint8_t )( ( Channel >> 16 ) & 0xFF );
//			Channel_Freq[1] = ( uint8_t )( ( Channel >> 8 ) & 0xFF );
//			Channel_Freq[2] = ( uint8_t )( ( Channel ) & 0xFF );
//			
//			if(Channel_Freq_MSB_temp != Channel_Freq[0])
//			{
//				Changed_Register_Count = 3;
//				Channel_Freq_MSB_temp = Channel_Freq[0];
//				Channel_Freq_MID_temp = Channel_Freq[1];
//				Channel_Freq_LSB_temp = Channel_Freq[2];
//			}
//			else if(Channel_Freq_MID_temp != Channel_Freq[1])
//			{
//				Changed_Register_Count = 2;
//				Channel_Freq_MID_temp = Channel_Freq[1];
//				Channel_Freq_LSB_temp = Channel_Freq[2];
//			}
//			else if(Channel_Freq_MID_temp != Channel_Freq[2])
//			{
//				Changed_Register_Count = 1;
//				Channel_Freq_LSB_temp = Channel_Freq[2];
//			}
//			
//			do
//			{
//				Time = TIM1->CNT;
//			}while( Time & (8-1));  // 10Mhz spi

//			Fast_SetChannel( Channel_Freq );
//			Time_temp[Count] = Time;
//			Input_Freq_temp[Count] = Input_Freq;
//			
//			n_temp[Count] = Changed_Register_Count;
//			Changed_Register_Count = 1;
//			Count++;
//			
//			HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_12);
//		}
//	}
//}

//void LoRa_Payload( int Start_freq)
//{
//	uint32_t Count = 0;
//	
//	TIM1->CNT = 0;
//	while(1)
//	{
//		Time = TIM1->CNT;
//		if( Time > LORA_SYMBOL_TIME )
//		{
//			break;
//		}
//		else if((( GPIOB->IDR & GPIO_PIN_6 ) != 0x00u ) && (( GPIOA->IDR & GPIO_PIN_9 ) != 0x00u ) )
//		{
//			Input_Freq = LORA_BASE_FREQ + Start_freq + Count * LORA_FREQ_STEP;
//			
//			if( Input_Freq > LORA_MAX_FREQ )
//			{
//				Input_Freq = Input_Freq - LORA_BW;
//			}
//			
//			SX_FREQ_TO_CHANNEL( Channel, Input_Freq );
//				
//			Channel_Freq[0] = ( uint8_t )( ( Channel >> 16 ) & 0xFF );
//			Channel_Freq[1] = ( uint8_t )( ( Channel >> 8 ) & 0xFF );
//			Channel_Freq[2] = ( uint8_t )( ( Channel ) & 0xFF );
//			
//			if( Channel_Freq_MSB_temp != Channel_Freq[0] )
//			{
//				Changed_Register_Count = 3;
//				Channel_Freq_MSB_temp = Channel_Freq[0];
//				Channel_Freq_MID_temp = Channel_Freq[1];
//				Channel_Freq_LSB_temp = Channel_Freq[2];
//			}
//			else if( Channel_Freq_MID_temp != Channel_Freq[1] )
//			{
//				Changed_Register_Count = 2;
//				Channel_Freq_MID_temp = Channel_Freq[1];
//				Channel_Freq_LSB_temp = Channel_Freq[2];
//			}
//			else if( Channel_Freq_MID_temp != Channel_Freq[2] )
//			{
//				Changed_Register_Count = 1;
//				Channel_Freq_LSB_temp = Channel_Freq[2];
//			}
//			
//			do
//			{
//				Time = TIM1->CNT;
//			}while( Time & (8-1));  // 10Mhz spi

//			Fast_SetChannel( Channel_Freq );
//			Time_temp[Count] = Time;
//			Input_Freq_temp[Count] = Channel;
//			
//			n_temp[Count] = Changed_Register_Count;
//			Changed_Register_Count = 1;
//			Count++;
//			
//			HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_12);
//		}
//	}
//}
