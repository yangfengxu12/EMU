#include "Simulated_LoRa.h"
#include "Timer_Calibration.h"
#include "sx1276Regs-Fsk.h"

int LoRa_ID_Start_Freq[LORA_ID_LENGTH] = {-62011,-61523};
int LoRa_Payload_Start_Freq[LORA_PAYLOAD_LENGTH] = {-7083,
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

uint32_t Input_Freq_temp[1<<LORA_SF]={0};
uint32_t Time_temp_temp[1<<LORA_SF]={0};
uint32_t n_temp[1<<LORA_SF]={0};

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


void LoRa_UpChirp()
{
	uint32_t Count = 0;

	Input_Freq = LORA_BASE_FREQ;
	TIM1->CNT = 0;
	while(1)
	{
		Time_temp = TIM1->CNT;
		if( Time_temp > LORA_SYMBOL_TIME || ( Input_Freq > LORA_BASE_FREQ + LORA_BW ))
		{
			break;
		}
		else if((( GPIOB->IDR & GPIO_PIN_6) != 0x00u) && (( GPIOA->IDR & GPIO_PIN_9) != 0x00u ))
		{
			Input_Freq = LORA_BASE_FREQ + Count * LORA_FREQ_STEP;
			
			SX_FREQ_TO_CHANNEL( Channel, Input_Freq );

			Channel_Freq[0] = ( uint8_t )( ( Channel >> 16 ) & 0xFF );
			Channel_Freq[1] = ( uint8_t )( ( Channel >> 8 ) & 0xFF );
			Channel_Freq[2] = ( uint8_t )( ( Channel ) & 0xFF );
			
			if(Channel_Freq_MSB_temp != Channel_Freq[0])
			{
				Changed_Register_Count = 3;
				Channel_Freq_MSB_temp = Channel_Freq[0];
				Channel_Freq_MID_temp = Channel_Freq[1];
				Channel_Freq_LSB_temp = Channel_Freq[2];
			}
			else if(Channel_Freq_MID_temp != Channel_Freq[1])
			{
				Changed_Register_Count = 2;
				Channel_Freq_MID_temp = Channel_Freq[1];
				Channel_Freq_LSB_temp = Channel_Freq[2];
			}
			else if(Channel_Freq_MID_temp != Channel_Freq[2])
			{
				Changed_Register_Count = 1;
				Channel_Freq_LSB_temp = Channel_Freq[2];
			}
			
			do
			{
				Time_temp = TIM1->CNT;
			}while( Time_temp & (8-1));  // 10Mhz spi

			Fast_SetChannel( Channel_Freq, Changed_Register_Count);
			Time_temp_temp[Count] = Time_temp;
			Input_Freq_temp[Count] = Input_Freq;
			
			n_temp[Count] = Changed_Register_Count;
			Changed_Register_Count = 1;
			Count++;
			
			HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_12);
		}
	}
}

//void LoRa_DownChirp()
//{
//	uint32_t Count = 0;

//	TIM1->CNT = 0;
//	while(1)
//	{
//		Time_temp = TIM1->CNT;
//		if(Time_temp > LORA_SYMBOL_TIME || ( Input_Freq < LORA_BASE_FREQ ))
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
//				Time_temp = TIM1->CNT;
//			}while( Time_temp & (8-1));  // 10Mhz spi

//			Fast_SetChannel( Channel_Freq );
//			Time_temp_temp[Count] = Time_temp;
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
//		Time_temp = TIM1->CNT;
//		if(Time_temp > LORA_SYMBOL_TIME / 4 )
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
//				Time_temp = TIM1->CNT;
//			}while( Time_temp & (8-1));  // 10Mhz spi

//			Fast_SetChannel( Channel_Freq );
//			Time_temp_temp[Count] = Time_temp;
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
//		Time_temp = TIM1->CNT;
//		if( Time_temp > LORA_SYMBOL_TIME )
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
//				Time_temp = TIM1->CNT;
//			}while( Time_temp & (8-1));  // 10Mhz spi

//			Fast_SetChannel( Channel_Freq );
//			Time_temp_temp[Count] = Time_temp;
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
