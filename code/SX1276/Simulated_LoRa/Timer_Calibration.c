#include "timer_calibration.h"




int RTC_Timer_Calibration()
{
	int Time_Componsation_Index = 0;
	float RTC_Time_temp = 0;
	float Time_Componsation_Value[20];
	uint8_t Comp_Count = 0;
	float Length = sizeof(Time_Componsation_Value) / sizeof(Time_Componsation_Value[0]);
	float Sum,Average;
	uint32_t RTC_Subsecond_Value[3] = {0};
	
	uint32_t Target_Time = 50000 - 1 ;  //us, must less than 65536
	__HAL_TIM_DISABLE(&TIM2_Handler);
	
	for(Comp_Count = 0 ; Comp_Count < Length; Comp_Count++)
	{
		TIM2->CNT = 0;
		RTC_Subsecond_Value[0] = ( uint32_t ) RTC_Handler.Instance->SSR;
		while( RTC_Handler.Instance->SSR - RTC_Subsecond_Value[0] == 0);
		
		__HAL_TIM_ENABLE(&TIM2_Handler);
		RTC_Subsecond_Value[0] = ( uint32_t ) RTC_Handler.Instance->SSR;
		
		while( TIM2->CNT <= Target_Time );
		RTC_Subsecond_Value[1] = ( uint32_t ) RTC_Handler.Instance->SSR;
		while( RTC_Handler.Instance->SSR - RTC_Subsecond_Value[1] == 0);
		
		__HAL_TIM_DISABLE(&TIM2_Handler);
		RTC_Subsecond_Value[1] = ( uint32_t ) RTC_Handler.Instance->SSR;
		Time = TIM2->CNT;
		
		if( RTC_Subsecond_Value[0] < RTC_Subsecond_Value[1] )
		{
			RTC_Subsecond_Value[0] += 32768;
		}
		RTC_Time_temp = ( RTC_Subsecond_Value[0] - RTC_Subsecond_Value[1] ) / 32768.0 * 1000000.0;
		Time_Componsation_Value[Comp_Count] = ( float ) RTC_Time_temp / ( float )( Time + 1 );
	}
	for(Comp_Count = 0 ; Comp_Count < Length; Comp_Count++)
	{
		Sum += Time_Componsation_Value[Comp_Count];//
	}
	Average = Sum / Length;
	
	// +: RTC < TIM ---> TIM - 
	// -: RTC > TIM ---> TIM + 
	Time_Componsation_Index = 1000000.0f / ( 1000000.0f - ( int )( 1000000.0f * Average + 0.5f )); 
	
	return Time_Componsation_Index;
}
