#include "timer_calibration.h"

int RTC_Timer_Calibration()
{
	RTC_Init();
	delay_ms(100);
	TIM15_Init();
	
}
