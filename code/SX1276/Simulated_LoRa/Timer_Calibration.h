#ifndef _TIMER_CALIBRATION_H__
#define _TIMER_CALIBRATION_H__

#include "timer.h"
#include "rtc.h"
#include "delay.h"



extern uint32_t Time;

int RTC_Timer_Calibration( void );
int Max(int x,int y);
int Min(int x,int y);

#endif
