#include "timer_calibration.h"



u8 RTC_NOT_STABLE = 0;

int RTC_Timer_Calibration( void )
{
	u8 i;
	int sum = 0,average = 0; 
	int max = 0, min = 0;
	
	
	Restart_Cali:
	RTC_Init();
	delay_ms( 100 );
	TIM15_Init();
	while( Timer_Calibration_Done_Flag == 0 )
	{
		delay_ms( 800 );
	}
	
	Timer_Calibration_Done_Flag = 0;
	
	for( i = 0; i < Calibration_Times ; i++ )
	{
		Input_Captured_Record[ i ][ 3 ] = 65536 * Input_Captured_Record[ i ][ 2 ] + \
																	Input_Captured_Record[ i ][ 1 ] - Input_Captured_Record[ i ][ 0 ];
		Input_Captured_Record[ i ][ 3 ] = 1000000 - Input_Captured_Record[ i ][ 3 ];
	}
	
	max = Input_Captured_Record[ 0 ][ 3 ];
	min = Input_Captured_Record[ 0 ][ 3 ];
	
	for( i = 0; i < Calibration_Times ; i++ )
	{	
		max = Max( max, Input_Captured_Record[ i ][ 3 ] );
		min = Min( min, Input_Captured_Record[ i ][ 3 ] );
		
		sum += Input_Captured_Record[ i ][ 3 ];
	}
	average = sum / Calibration_Times;
	return 1000000 / average ;
}

int Max(int x,int y)
{
	int z;
	if(x>y)z=x;
	else z=y;
	return z;
}

int Min(int x,int y)
{
	int z;
	if(x<y)z=x;
	else z=y;
	return z;
}

